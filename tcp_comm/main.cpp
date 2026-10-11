
#include <stdio.h>
#include <string.h>

#include "main.h"

#define TAG						"[MAIN]"

uint16_t tcp_port = 5000;
char server_host[256] = {0};
bool is_server = true;

int main(int argc, char *argv[]) {
	strncpy(server_host, "127.0.0.1", sizeof(server_host) - 1);

	for (int i = 1; i < argc; ++i) {
		if (strcmp("-a", argv[i]) == 0 || strcmp("--server-host", argv[i]) == 0) {
			// tcp server ip-address or host name
			++i;
			if (i < argc) {
				strncpy(server_host, argv[i], sizeof(server_host) - 1);
			}
			is_server = false;
		} else if (strcmp("-p", argv[i]) == 0 || strcmp("--port", argv[i]) == 0) {
			// tcp server port to listen or connect to
			++i;
			if (i < argc) {
				if (sscanf(argv[i], "%hu", &tcp_port) != 1) {
					print_help(argv[0]);
					return 0;
				}
			}
		} else if (strcmp("-h", argv[i]) == 0 || strcmp("--help", argv[i]) == 0) {
			print_help(argv[0]);
			return 0;
		} else {
			print_help(argv[0]);
			return 0;
		}
	}

	if (is_server) {
		run_tcp_server();
	} else {
		run_tcp_client();
	}

	return 0;
}

void run_tcp_server() {
	TcpSocket server;

	if (!server.listen(tcp_port)) {
		LOGE(TAG, "server listen error\n");
		return;
	}

	LOGI(TAG, "server listening on port %u\n", tcp_port);

	TcpSocket *client = server.accept();
	if (client != NULL) {
		LOGI(TAG, "client connected\n");

		client->OnDataAvailable = client_OnDataAvailable;
		client->startReceiveThread();

		while (client->isConnected()) {
			sleep_ms(100);
		}

		delete client;
	}
}

void run_tcp_client() {
	TcpSocket client;

	if (client.connect(server_host, tcp_port)) {
		LOGI(TAG, "connected to %s\n", server_host);

		client.OnDataAvailable = client_OnDataAvailable;
		client.startReceiveThread();

		while (client.isConnected()) {
			sleep_ms(100);
		}

		client.close();
	} else {
		LOGE(TAG, "can't connect to %s\n", server_host);
	}
}

void server_OnDataAvailable(TcpSocket *socket, const char *data, int len) {
	TCP_UNUSED(socket);
	TCP_UNUSED(data);
	TCP_UNUSED(len);
}

void client_OnDataAvailable(TcpSocket *socket, const char *data, int len) {
	TCP_UNUSED(socket);
	TCP_UNUSED(data);
	TCP_UNUSED(len);
}

void print_help(const char *exe_name) {
	printf(
		"Usage:\n"
		"    %s [options]\n\n"
		"Options are:\n"
		"    -a|--server-host <server IP-address or host name>:\n"
		"        act as a TCP client and connect to TCP server. If this parameter\n"
		"        is ommited or missing, start as a server.\n"
		"    -p|--port <0..65535>:\n"
		"        TCP server port to listen or TCP client port used to connect to server host\n"
		"    -h|--help:\n"
		"        print this help info\n", exe_name
	);
}
