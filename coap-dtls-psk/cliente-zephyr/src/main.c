/*
 * Copyright (c) 2026 Rafael Dias
 *
 * SPDX-License-Identifier: MIT
 */

/*
 * Cliente CoAP sobre DTLS com PSK: um handshake, um POST, a resposta.
 *
 * O mbedTLS é usado direto, com callbacks de BIO sobre um socket UDP com connect(), em vez
 * da camada de sockets TLS do Zephyr (IPPROTO_DTLS_1_2). Essa camada pede o endereço de
 * origem em todo recvfrom(), e drivers com sockets offloaded como o eswifi recusam isso.
 * Com o socket conectado, send() e recv() simples bastam.
 */

#include <errno.h>
#include <string.h>

#include <mbedtls/net_sockets.h>
#include <mbedtls/ssl.h>
#include <psa/crypto.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/coap.h>
#include <zephyr/net/socket.h>
#include <zephyr/random/random.h>

LOG_MODULE_REGISTER(cliente, LOG_LEVEL_INF);

#define CONTENT_FORMAT_JSON 50

static const int ciphersuites[] = {
	MBEDTLS_TLS_PSK_WITH_AES_128_CCM_8,
	MBEDTLS_TLS_PSK_WITH_AES_128_GCM_SHA256,
	0,
};

static int sock = -1;
static mbedtls_ssl_context ssl;
static mbedtls_ssl_config conf;

/* Temporizador que o DTLS usa para retransmitir o handshake. */
static struct {
	int64_t inicio;
	uint32_t intermediario_ms;
	uint32_t final_ms;
} timer;

static int bio_send(void *arg, const unsigned char *buf, size_t len)
{
	ARG_UNUSED(arg);

	ssize_t enviado = zsock_send(sock, buf, len, 0);

	if (enviado < 0) {
		return errno == EAGAIN ? MBEDTLS_ERR_SSL_WANT_WRITE : MBEDTLS_ERR_NET_SEND_FAILED;
	}
	return (int)enviado;
}

static int bio_recv_timeout(void *arg, unsigned char *buf, size_t len, uint32_t timeout_ms)
{
	ARG_UNUSED(arg);

	struct zsock_pollfd pfd = {.fd = sock, .events = ZSOCK_POLLIN};
	int ret = zsock_poll(&pfd, 1, timeout_ms == 0 ? SYS_FOREVER_MS : (int)timeout_ms);

	if (ret == 0) {
		return MBEDTLS_ERR_SSL_TIMEOUT;
	}
	if (ret < 0) {
		return MBEDTLS_ERR_NET_RECV_FAILED;
	}

	ssize_t recebido = zsock_recv(sock, buf, len, ZSOCK_MSG_DONTWAIT);

	if (recebido < 0) {
		return errno == EAGAIN ? MBEDTLS_ERR_SSL_WANT_READ : MBEDTLS_ERR_NET_RECV_FAILED;
	}
	return (int)recebido;
}

static void timer_set(void *arg, uint32_t intermediario_ms, uint32_t final_ms)
{
	ARG_UNUSED(arg);

	timer.inicio = k_uptime_get();
	timer.intermediario_ms = intermediario_ms;
	timer.final_ms = final_ms;
}

static int timer_get(void *arg)
{
	ARG_UNUSED(arg);

	if (timer.final_ms == 0) {
		return -1; /* parado */
	}

	int64_t decorrido = k_uptime_get() - timer.inicio;

	if (decorrido >= timer.final_ms) {
		return 2; /* expirou */
	}
	return decorrido >= timer.intermediario_ms ? 1 : 0;
}

static int conectar(void)
{
	struct net_sockaddr_in servidor = {
		.sin_family = NET_AF_INET,
		.sin_port = net_htons(CONFIG_CLIENTE_SERVIDOR_PORTA),
	};
	const char *chave = CONFIG_CLIENTE_PSK_CHAVE;
	const char *identidade = CONFIG_CLIENTE_PSK_IDENTIDADE;
	int ret;

	zsock_inet_pton(NET_AF_INET, CONFIG_CLIENTE_SERVIDOR_IP, &servidor.sin_addr);

	sock = zsock_socket(NET_AF_INET, NET_SOCK_DGRAM, NET_IPPROTO_UDP);
	if (sock < 0 ||
	    zsock_connect(sock, (struct net_sockaddr *)&servidor, sizeof(servidor)) < 0) {
		LOG_ERR("socket/connect: %d", errno);
		return -errno;
	}

	mbedtls_ssl_init(&ssl);
	mbedtls_ssl_config_init(&conf);
	mbedtls_ssl_config_defaults(&conf, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_DATAGRAM,
				    MBEDTLS_SSL_PRESET_DEFAULT);
	mbedtls_ssl_conf_ciphersuites(&conf, ciphersuites);
	/* Limita os intervalos de retransmissão do handshake a uma faixa de 1 a 16 s.
	 * Nos testes deste exemplo, o timeout aparece como -0x6800. Com uma chave incorreta,
	 * o servidor de teste descarta o registro inválido sem enviar alerta.
	 */
	mbedtls_ssl_conf_handshake_timeout(&conf, 1000, 16000);
	mbedtls_ssl_conf_psk(&conf, (const unsigned char *)chave, strlen(chave),
			     (const unsigned char *)identidade, strlen(identidade));
	mbedtls_ssl_setup(&ssl, &conf);
	mbedtls_ssl_set_bio(&ssl, NULL, bio_send, NULL, bio_recv_timeout);
	mbedtls_ssl_set_timer_cb(&ssl, NULL, timer_set, timer_get);

	do {
		ret = mbedtls_ssl_handshake(&ssl);
	} while (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE);

	if (ret != 0) {
		LOG_ERR("handshake falhou: -0x%04x", -ret);
		return -ECONNREFUSED;
	}

	LOG_INF("handshake OK como '%s', suíte %s", identidade,
		mbedtls_ssl_get_ciphersuite(&ssl));
	return 0;
}

static int enviar_leitura(void)
{
	static const char payload[] = "{\"temperatura\":21.4,\"unidade\":\"C\"}";
	uint8_t buf[256];
	struct coap_packet pedido;
	struct coap_packet resposta;
	uint8_t token[4];
	int ret;

	sys_rand_get(token, sizeof(token));
	coap_packet_init(&pedido, buf, sizeof(buf), COAP_VERSION_1, COAP_TYPE_CON,
			 sizeof(token), token, COAP_METHOD_POST, coap_next_id());
	coap_packet_append_option(&pedido, COAP_OPTION_URI_PATH, "leituras",
				  strlen("leituras"));
	coap_append_option_int(&pedido, COAP_OPTION_CONTENT_FORMAT, CONTENT_FORMAT_JSON);
	coap_packet_append_payload_marker(&pedido);
	coap_packet_append_payload(&pedido, payload, strlen(payload));

	/* Um firmware real retransmite o CON; aqui um envio basta para mostrar o caminho. */
	do {
		ret = mbedtls_ssl_write(&ssl, pedido.data, pedido.offset);
	} while (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE);
	if (ret < 0) {
		LOG_ERR("envio falhou: -0x%04x", -ret);
		return -EIO;
	}

	mbedtls_ssl_conf_read_timeout(&conf, 5000);
	do {
		ret = mbedtls_ssl_read(&ssl, buf, sizeof(buf));
	} while (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE);
	if (ret < 0) {
		LOG_ERR("sem resposta: -0x%04x", -ret);
		return -EIO;
	}

	if (coap_packet_parse(&resposta, buf, ret, NULL, 0) < 0) {
		LOG_ERR("resposta CoAP inválida");
		return -EINVAL;
	}

	uint8_t codigo = coap_header_get_code(&resposta);

	LOG_INF("resposta %u.%02u", codigo >> 5, codigo & 0x1f);
	return codigo == COAP_RESPONSE_CODE_CHANGED ? 0 : -EIO;
}

static void fechar(void)
{
	/* close_notify avisa o servidor que a sessão acabou. Sem ele, a sessão fica aberta
	 * do lado de lá até expirar.
	 */
	(void)mbedtls_ssl_close_notify(&ssl);
	mbedtls_ssl_free(&ssl);
	mbedtls_ssl_config_free(&conf);
	zsock_close(sock);
}

int main(void)
{
	int ret;

	if (psa_crypto_init() != PSA_SUCCESS) {
		LOG_ERR("psa_crypto_init falhou");
		return 0;
	}

	ret = conectar();
	if (ret == 0) {
		ret = enviar_leitura();
	}
	fechar();

	LOG_INF("%s", ret == 0 ? "RESULTADO: OK" : "RESULTADO: FALHA");
	return 0;
}
