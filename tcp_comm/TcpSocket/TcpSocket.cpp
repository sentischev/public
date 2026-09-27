
#include "TcpSocket.h"
#include "tcp_common.h"

#include <cstring>

#ifdef _WINDOWS
	#pragma comment(lib, "Ws2_32.lib")
#else
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <netdb.h>
#endif /* _WINDOWS */

#define TAG						"[TCP]"

int TcpSocket::s_initCount = 0;

void TcpSocket::initWinsock() {
#ifdef _WINDOWS
	if (s_initCount == 0) {
		WSADATA wsaData;
		WSAStartup(MAKEWORD(2, 2), &wsaData);
	}
	s_initCount++;
#endif
}

void TcpSocket::cleanupWinsock() {
#ifdef _WINDOWS
	s_initCount--;
	if (s_initCount == 0) {
		WSACleanup();
	}
#endif
}

TcpSocket::TcpSocket() {
	m_sock = INVALID_SOCKET;
	m_isConnected = false;
	OnDataAvailable = NULL;
	m_receiveThread = 0;
	m_receiveThreadRunning = false;

	initWinsock();
}

TcpSocket::TcpSocket(tcp_socket_t tcp_socket) {
	m_sock = tcp_socket;
	m_isConnected = true;
	OnDataAvailable = NULL;
	m_receiveThread = 0;
	m_receiveThreadRunning = false;

	initWinsock();
}

TcpSocket::~TcpSocket() {
	tcp_close();
	cleanupWinsock();
}

bool TcpSocket::tcp_connect(const char *host, uint16_t hostPort) {
	uint32_t serverIPAddress = inet_addr(host);

	// if it's not valid IP string, try resolving it as a hostname
	if (serverIPAddress == INADDR_NONE) {
		hostent *hostEntry = gethostbyname(host);
		if (hostEntry == NULL || hostEntry->h_addr_list[0] == NULL) {
			return false;
		}

		std::memcpy(&serverIPAddress, hostEntry->h_addr_list[0], sizeof(serverIPAddress));
	}

	return tcp_connect(serverIPAddress, hostPort);
}

bool TcpSocket::tcp_connect(uint32_t serverIPAddress, uint16_t serverPort) {
	if (m_sock != INVALID_SOCKET) {
		tcp_close();
	}

	m_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_sock == INVALID_SOCKET) {
		return false;
	}

	sockaddr_in addr;
	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(serverIPAddress);
	addr.sin_port = htons(serverPort);

	if (connect(m_sock, (sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR) {
		tcp_close();
		return false;
	}

	m_isConnected = true;
	return true;
}

bool TcpSocket::tcp_listen(uint16_t port, int max_clients) {
	if (m_sock != INVALID_SOCKET) {
		tcp_close();
	}

	m_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_sock == INVALID_SOCKET) {
		return false;
	}

	int optval = 1;
	setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, (char *)&optval, sizeof(optval));

	sockaddr_in addr;
	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);

	if (bind(m_sock, (sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR) {
		tcp_close();
		return false;
	}

	if (listen(m_sock, max_clients) == SOCKET_ERROR) {
		tcp_close();
		return false;
	}

	m_isConnected = true;
	return true;
}

TcpSocket *TcpSocket::tcp_accept() {
	if (m_sock == INVALID_SOCKET) {
		return NULL;
	}

	sockaddr_in client_addr;
	socklen_t client_addr_len = sizeof(client_addr);

	tcp_socket_t clientSocket = accept(m_sock, (sockaddr *)&client_addr, &client_addr_len);
	if (clientSocket == INVALID_SOCKET) {
		return NULL;
	}

	return new TcpSocket(clientSocket);
}

int TcpSocket::tcp_send(const char *data, int len) {
	if (!m_isConnected || m_sock == INVALID_SOCKET) {
		return -1;
	}

	return send(m_sock, data, len, 0);
}

int TcpSocket::tcp_receive(char *data, int max_len) {
	if (!m_isConnected || m_sock == INVALID_SOCKET) {
		return -1;
	}

	return recv(m_sock, data, max_len, 0);
}

tcp_thread_ret_t TcpSocket::tcp_receive_loop(void *arg) {
	TcpSocket *self = (TcpSocket *)arg;
	self->run_tcp_receive_loop();

	return 0;
}

void TcpSocket::run_tcp_receive_loop() {
	char buf[4096] = {0};
	m_receiveThreadRunning = true;

	while (m_isConnected) {
		int bytes_received = tcp_receive(buf, sizeof(buf));
		if (bytes_received > 0) {
			if (OnDataAvailable != NULL) {
				OnDataAvailable(this, buf, bytes_received);
			}
		} else {
			break;
		}
	}

	tcp_close();
	m_receiveThreadRunning = false;
}

void TcpSocket::startReceiveThread() {
	if (!m_isConnected || m_receiveThreadRunning) {
		return;
	}

#ifdef _WINDOWS
	m_receiveThread = CreateThread(NULL, 0, tcp_receive_loop, this, 0, NULL);
#else
	pthread_create(&m_receiveThread, NULL, tcp_receive_loop, this);
	pthread_detach(m_receiveThread);
#endif
}

void TcpSocket::tcp_close() {
	if (m_sock != INVALID_SOCKET) {
#ifdef _WINDOWS
		closesocket(m_sock);
#else
		close(m_sock);
#endif

		m_sock = INVALID_SOCKET;
		m_isConnected = false;
	}
}

bool TcpSocket::isConnected() {
	return m_isConnected;
}
