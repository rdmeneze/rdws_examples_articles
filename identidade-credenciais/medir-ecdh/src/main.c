/*
 * Copyright (c) 2026 Rafael Dias
 *
 * SPDX-License-Identifier: MIT
 */

/*
 * Mede o que o ECDHE-PSK acrescenta ao PSK puro no lado do cliente: gerar um par de chaves
 * P-256 efêmero e fazer o acordo de chaves com a chave pública do servidor.
 *
 * Mede tempo, pico do heap do mbedTLS e pico de pilha. Não mede o handshake inteiro.
 */

#include <psa/crypto.h>

#include <mbedtls/memory_buffer_alloc.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#define RODADAS 10
#define PUB_P256_TAM 65
#define SEGREDO_P256_TAM 32

static psa_status_t gerar_chave(mbedtls_svc_key_id_t *chave)
{
	psa_key_attributes_t atributos = PSA_KEY_ATTRIBUTES_INIT;

	psa_set_key_type(&atributos, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
	psa_set_key_bits(&atributos, 256);
	psa_set_key_algorithm(&atributos, PSA_ALG_ECDH);
	psa_set_key_usage_flags(&atributos, PSA_KEY_USAGE_DERIVE);

	return psa_generate_key(&atributos, chave);
}

static uint32_t us_desde(uint32_t inicio)
{
	return (uint32_t)k_cyc_to_us_floor64(k_cycle_get_32() - inicio);
}

int main(void)
{
	mbedtls_svc_key_id_t servidor;
	uint8_t pub_servidor[PUB_P256_TAM];
	uint8_t segredo[SEGREDO_P256_TAM];
	size_t tamanho;
	uint64_t soma_gerar = 0;
	uint64_t soma_acordo = 0;
	uint32_t max_total = 0;
	uint32_t min_total = UINT32_MAX;
	size_t heap_usado;
	size_t heap_blocos;
	size_t pilha_livre;

	printk("CPU: %u Hz\n", (unsigned int)sys_clock_hw_cycles_per_sec());

	psa_status_t st = psa_crypto_init();

	if (st != PSA_SUCCESS) {
		printk("psa_crypto_init: %d\n", (int)st);
		return 0;
	}

	st = gerar_chave(&servidor);
	if (st == PSA_SUCCESS) {
		st = psa_export_public_key(servidor, pub_servidor, sizeof(pub_servidor), &tamanho);
	}
	if (st != PSA_SUCCESS) {
		printk("chave do servidor: %d\n", (int)st);
		return 0;
	}

	mbedtls_memory_buffer_alloc_max_reset();

	for (int i = 0; i < RODADAS; i++) {
		mbedtls_svc_key_id_t cliente;
		uint8_t pub_cliente[PUB_P256_TAM];
		uint32_t inicio = k_cycle_get_32();

		/* A exportação entra na conta: o cliente manda a chave pública no handshake, e na
		 * implementação do mbedTLS é aqui que ela é calculada, não em psa_generate_key()
		 */
		st = gerar_chave(&cliente);
		if (st == PSA_SUCCESS) {
			st = psa_export_public_key(cliente, pub_cliente, sizeof(pub_cliente),
						   &(size_t){0});
		}
		if (st != PSA_SUCCESS) {
			printk("gerar: %d\n", (int)st);
			return 0;
		}

		uint32_t t_gerar = us_desde(inicio);

		inicio = k_cycle_get_32();
		st = psa_raw_key_agreement(PSA_ALG_ECDH, cliente, pub_servidor, tamanho, segredo,
					   sizeof(segredo), &(size_t){0});
		if (st != PSA_SUCCESS) {
			printk("acordo: %d\n", (int)st);
			return 0;
		}

		uint32_t t_acordo = us_desde(inicio);
		uint32_t t_total = t_gerar + t_acordo;

		psa_destroy_key(cliente);

		printk("rodada %d: gerar=%u us acordo=%u us total=%u us\n", i, t_gerar, t_acordo,
		       t_total);
		soma_gerar += t_gerar;
		soma_acordo += t_acordo;
		max_total = MAX(max_total, t_total);
		min_total = MIN(min_total, t_total);
	}

	mbedtls_memory_buffer_alloc_max_get(&heap_usado, &heap_blocos);
	k_thread_stack_space_get(k_current_get(), &pilha_livre);

	printk("RESULTADO: gerar=%u us acordo=%u us total min=%u max=%u us (%d rodadas)\n",
	       (unsigned int)(soma_gerar / RODADAS), (unsigned int)(soma_acordo / RODADAS),
	       min_total, max_total, RODADAS);
	printk("RESULTADO: heap pico=%zu B em %zu blocos; pilha usada=%zu B de %d B\n", heap_usado,
	       heap_blocos, CONFIG_MAIN_STACK_SIZE - pilha_livre, CONFIG_MAIN_STACK_SIZE);
	return 0;
}
