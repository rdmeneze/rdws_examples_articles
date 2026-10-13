/*
 * Copyright (c) 2026 Rafael Dias
 *
 * SPDX-License-Identifier: MIT
 */

/*
 * Servidor CoAP sobre DTLS com PSK, uma chave por device.
 *
 * Aceita POST em /leituras, loga a identidade PSK da sessão e o corpo, e responde 2.04.
 * A identidade da sessão diz quem é o device; o corpo da mensagem não tem voz nisso.
 *
 * Uso: servidor [porta] [timeout_sessao_s]
 */

#include <coap3/coap.h>

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Num sistema real esta tabela vem do banco; aqui é fixa. */
struct credencial {
	const char *identidade;
	const char *chave;
};

static const struct credencial credenciais[] = {
	{"estacao-01", "segredo-da-estacao-01"},
	{"estacao-02", "segredo-da-estacao-02"},
};

static volatile sig_atomic_t rodando = 1;

static void parar(int sinal)
{
	(void)sinal;
	rodando = 0;
}

/*
 * Chamado no handshake com a identidade que o cliente mandou. Devolve a chave daquele
 * device, ou NULL para recusar o handshake.
 */
static const coap_bin_const_t *buscar_chave(coap_bin_const_t *identidade,
					    coap_session_t *sessao, void *arg)
{
	static coap_bin_const_t chave;

	(void)sessao;
	(void)arg;

	for (size_t i = 0; i < sizeof(credenciais) / sizeof(credenciais[0]); i++) {
		const char *id = credenciais[i].identidade;

		if (identidade->length == strlen(id) &&
		    memcmp(identidade->s, id, identidade->length) == 0) {
			chave.s = (const uint8_t *)credenciais[i].chave;
			chave.length = strlen(credenciais[i].chave);
			return &chave;
		}
	}

	printf("handshake recusado: identidade desconhecida '%.*s'\n",
	       (int)identidade->length, (const char *)identidade->s);
	return NULL;
}

static void receber_leitura(coap_resource_t *recurso, coap_session_t *sessao,
			    const coap_pdu_t *pedido, const coap_string_t *query,
			    coap_pdu_t *resposta)
{
	const coap_bin_const_t *identidade = coap_session_get_psk_identity(sessao);
	size_t tamanho = 0;
	const uint8_t *dados = NULL;
	size_t offset = 0;
	size_t total = 0;

	(void)recurso;
	(void)query;

	coap_get_data_large(pedido, &tamanho, &dados, &offset, &total);

	printf("POST de '%.*s': %.*s\n",
	       identidade ? (int)identidade->length : 0,
	       identidade ? (const char *)identidade->s : "",
	       (int)tamanho, dados ? (const char *)dados : "");

	coap_pdu_set_code(resposta, COAP_RESPONSE_CODE_CHANGED);
}

int main(int argc, char *argv[])
{
	const uint16_t porta = argc > 1 ? (uint16_t)atoi(argv[1]) : 5684;
	const unsigned int timeout_sessao_s = argc > 2 ? (unsigned int)atoi(argv[2]) : 60;
	coap_context_t *contexto;
	coap_address_t endereco;
	coap_dtls_spsk_t psk = {0};
	coap_resource_t *recurso;

	setvbuf(stdout, NULL, _IOLBF, 0);
	coap_startup();
	coap_set_log_level(COAP_LOG_WARN);

	if (!coap_dtls_is_supported()) {
		fprintf(stderr, "libcoap compilado sem DTLS\n");
		return EXIT_FAILURE;
	}

	contexto = coap_new_context(NULL);
	if (contexto == NULL) {
		return EXIT_FAILURE;
	}

	/* Corpo grande em blocos (Block1) chega ao handler já remontado. */
	coap_context_set_block_mode(contexto, COAP_BLOCK_USE_LIBCOAP | COAP_BLOCK_SINGLE_BODY);

	/*
	 * Sessão ociosa expira em timeout_sessao_s (padrão do libcoap: 300 s). Um device que
	 * reinicia sem fechar a sessão e volta da mesma porta de origem fica preso na sessão
	 * antiga até ela expirar.
	 */
	coap_context_set_session_timeout(contexto, timeout_sessao_s);

	psk.version = COAP_DTLS_SPSK_SETUP_VERSION;
	psk.validate_id_call_back = buscar_chave;
	if (!coap_context_set_psk2(contexto, &psk)) {
		fprintf(stderr, "falha ao configurar PSK\n");
		coap_free_context(contexto);
		return EXIT_FAILURE;
	}

	coap_address_init(&endereco);
	endereco.addr.sin.sin_family = AF_INET;
	endereco.addr.sin.sin_addr.s_addr = htonl(INADDR_ANY);
	endereco.addr.sin.sin_port = htons(porta);
	endereco.size = sizeof(endereco.addr.sin);
	if (coap_new_endpoint(contexto, &endereco, COAP_PROTO_DTLS) == NULL) {
		fprintf(stderr, "falha ao abrir a porta %u\n", porta);
		coap_free_context(contexto);
		return EXIT_FAILURE;
	}

	recurso = coap_resource_init(coap_make_str_const("leituras"), 0);
	coap_register_request_handler(recurso, COAP_REQUEST_POST, receber_leitura);
	coap_add_resource(contexto, recurso);

	signal(SIGINT, parar);
	signal(SIGTERM, parar);
	printf("ouvindo coaps://0.0.0.0:%u/leituras (sessão ociosa expira em %u s)\n", porta,
	       timeout_sessao_s);

	while (rodando) {
		coap_io_process(contexto, 1000);
	}

	coap_free_context(contexto);
	coap_cleanup();
	return EXIT_SUCCESS;
}
