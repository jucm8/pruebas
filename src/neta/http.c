/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/logging/log.h>
#include <zephyr/posix/sys/socket.h>
#include <zephyr/posix/unistd.h>
#include <zephyr/posix/arpa/inet.h>

#include <zephyr/net/net_ip.h>
#include <zephyr/net/tls_credentials.h>
#include <zephyr/net/http/client.h>
#include "http.h"

LOG_MODULE_REGISTER(net_http_client_sample);

#define SERVER_ADDR4 "146.190.62.39"
#define MAX_RECV_BUF_LEN 4096

static uint8_t recv_buf_ipv4[MAX_RECV_BUF_LEN];

int setup_socket(sa_family_t family, const char *server, int port,
			int *sock, struct sockaddr *addr, socklen_t addr_len)
{
	const char *family_str = family == AF_INET ? "IPv4" : "IPv6";
	int ret = 0;

	memset(addr, 0, addr_len);

	net_sin(addr)->sin_family = AF_INET;
	net_sin(addr)->sin_port = htons(port);
	inet_pton(family, server, &net_sin(addr)->sin_addr);
	*sock = socket(family, SOCK_STREAM, IPPROTO_TCP);

	if (*sock < 0) {
		LOG_ERR("Failed to create %s HTTP socket (%d)", family_str,
			-errno);
	}

	return ret;
}

int payload_cb(int sock, struct http_request *req, void *user_data)
{
	const char *content[] = {
		"foobar",
		"chunked",
		"last"
	};
	char tmp[64];
	int i, pos = 0;

	for (i = 0; i < ARRAY_SIZE(content); i++) {
		pos += snprintk(tmp + pos, sizeof(tmp) - pos,
				"%x\r\n%s\r\n",
				(unsigned int)strlen(content[i]),
				content[i]);
	}

	pos += snprintk(tmp + pos, sizeof(tmp) - pos, "0\r\n\r\n");

	(void)send(sock, tmp, pos, 0);

	return pos;
}

int response_cb(struct http_response *rsp,
		       enum http_final_call final_data,
		       void *user_data)
{
	if (final_data == HTTP_DATA_MORE) {
		LOG_INF("Partial data received (%zd bytes)", rsp->data_len);
	} else if (final_data == HTTP_DATA_FINAL) {
		LOG_INF("All the data received (%zd bytes)", rsp->data_len);
	}

	LOG_INF("Response to %s", (const char *)user_data);
	LOG_INF("Response status %s", rsp->http_status);

	return 0;
}

int connect_socket(sa_family_t family, const char *server, int port,
			  int *sock, struct sockaddr *addr, socklen_t addr_len)
{
	int ret;

	ret = setup_socket(family, server, port, sock, addr, addr_len);
	if (ret < 0 || *sock < 0) {
		return -1;
	}

	ret = connect(*sock, addr, addr_len);
	if (ret < 0) {
		LOG_ERR("Cannot connect to %s remote (%d)",
			family == AF_INET ? "IPv4" : "IPv6",
			-errno);
		//close(*sock);
		*sock = -1;
		ret = -errno;
	}

	return ret;
}

int run_queries()
{
	struct sockaddr_in addr4;
	int sock4 = -1, sock6 = -1;
	int32_t timeout = 3 * MSEC_PER_SEC;
	int ret = 0;
	int port = 80;
	struct http_request req;

	if (IS_ENABLED(CONFIG_NET_SOCKETS_SOCKOPT_TLS)) {
		/* ret = tls_credential_add(CA_CERTIFICATE_TAG, */
		/* 			 TLS_CREDENTIAL_CA_CERTIFICATE, */
		/* 			 ca_certificate, */
		/* 			 sizeof(ca_certificate)); */
		/* if (ret < 0) { */
		/* 	LOG_ERR("Failed to register public certificate: %d", */
		/* 		ret); */
		/* 	return ret; */
		/* } */
		/**/
		/* port = HTTPS_PORT; */
	}

	if (IS_ENABLED(CONFIG_NET_IPV4)) {
		(void)connect_socket(AF_INET, SERVER_ADDR4, port,
				     &sock4, (struct sockaddr *)&addr4,
				     sizeof(addr4));
		if (sock4 < 0) {
			LOG_ERR("Cannot create HTTP IPv4 connection.");
			return -ECONNABORTED;
		}

		memset(&req, 0, sizeof(req));

		req.method = HTTP_GET;
		req.url = "/";
		req.host = SERVER_ADDR4;
		req.protocol = "HTTP/1.1";
		req.response = response_cb;
		req.recv_buf = recv_buf_ipv4;
		req.recv_buf_len = sizeof(recv_buf_ipv4);

		ret = http_client_req(sock4, &req, timeout, "IPv4 GET");
		if (ret < 0) {
			LOG_ERR("Client error %d", ret);
		}

		close(sock4);
	}

	sock4 = -1;
	sock6 = -1;

	if (IS_ENABLED(CONFIG_NET_IPV4)) {
		(void)connect_socket(AF_INET, SERVER_ADDR4, port,
				     &sock4, (struct sockaddr *)&addr4,
				     sizeof(addr4));
		if (sock4 < 0) {
			LOG_ERR("Cannot create HTTP IPv4 connection.");
			return -ECONNABORTED;
		}

		memset(&req, 0, sizeof(req));

		req.method = HTTP_POST;
		req.url = "/foobar";
		req.host = SERVER_ADDR4;
		req.protocol = "HTTP/1.1";
		req.payload = "foobar";
		req.payload_len = strlen(req.payload);
		req.response = response_cb;
		req.recv_buf = recv_buf_ipv4;
		req.recv_buf_len = sizeof(recv_buf_ipv4);

		ret = http_client_req(sock4, &req, timeout, "IPv4 POST");
		if (ret < 0) {
			LOG_ERR("Client error %d", ret);
		}

		close(sock4);
	}

	return ret;
}
