
#include "TcpSocket.h"
#include "tcp_common.h"

#include <cstring>

#ifdef _WINDOWS
	#pragma comment(lib, "Ws2_32.lib")
#else
#endif /* _WINDOWS */

#define TAG						"[TcpSocket]"

int TcpSocket::s_initCount = 0;

void TcpSocket::initWinsock() {
#ifdef _WINDOWS
	if (s_initCount == 0) {
		WSADATA wsaData;
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
			LOGE(TAG, "WSAStartup error\n");
		}
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
	m_isServer = false;
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
	close();
	cleanupWinsock();
}

bool TcpSocket::connect(const char *host, uint16_t hostPort) {
	IPv4Addr serverIP(host);

	// if it's not valid IP string, try resolving it as a hostname
	if (serverIP.addr == INADDR_NONE) {
		hostent *hostEntry = gethostbyname(host);
		if (hostEntry == NULL || hostEntry->h_addr_list[0] == NULL) {
			LOGE(TAG, "invalid host name: %s\n", host);
			return false;
		}

		serverIP.addr = ntohl(*(uint32_t *)hostEntry->h_addr_list[0]);
	}

	return connect(serverIP.addr, hostPort);
}

bool TcpSocket::connect(uint32_t serverIPAddress, uint16_t serverPort) {
	if (m_sock != INVALID_SOCKET) {
		close();
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

	if (::connect(m_sock, (sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR) {
		close();
		return false;
	}

	m_isConnected = true;
	return true;
}

bool TcpSocket::listen(uint16_t port, int max_clients) {
	if (m_sock != INVALID_SOCKET) {
		close();
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
		close();
		return false;
	}

	if (::listen(m_sock, max_clients) == SOCKET_ERROR) {
		close();
		return false;
	}

	m_isConnected = true;
	m_isServer = true;
	return true;
}

TcpSocket *TcpSocket::accept() {
	if (m_sock == INVALID_SOCKET) {
		return NULL;
	}

	sockaddr_in client_addr;
	socklen_t client_addr_len = sizeof(client_addr);

	tcp_socket_t clientSocket = ::accept(m_sock, (sockaddr *)&client_addr, &client_addr_len);
	if (clientSocket == INVALID_SOCKET) {
		return NULL;
	}

	TcpSocket *_tcpSocket = new TcpSocket(clientSocket);
	_tcpSocket->ip.addr = ntohl(client_addr.sin_addr.s_addr);

	return _tcpSocket;
}

int TcpSocket::send(const char *data, int len) {
	if (!m_isConnected || m_sock == INVALID_SOCKET) {
		return -1;
	}

	return ::send(m_sock, data, len, 0);
}

int TcpSocket::recv(char *data, int max_len) {
	if (!m_isConnected || m_sock == INVALID_SOCKET) {
		return -1;
	}

	return ::recv(m_sock, data, max_len, 0);
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
		int bytes_received = recv(buf, sizeof(buf));
		if (bytes_received > 0) {
			if (OnDataAvailable != NULL) {
				OnDataAvailable(this, buf, bytes_received);
			}
		} else {
			break;
		}
	}

	close();
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

void TcpSocket::close() {
	if (m_sock != INVALID_SOCKET) {
#ifdef _WINDOWS
		closesocket(m_sock);
#else
		::close(m_sock);
#endif

		m_sock = INVALID_SOCKET;
		m_isConnected = false;
	}
}

bool TcpSocket::isConnected() {
	return m_isConnected;
}

bool TcpSocket::isServer() {
	return m_isServer;
}

std::string IPv4Addr::toStr() {
	char ip_cstr[16] = {0};
	uint8_t b1 = (addr >> 24) & 0xFF;
	uint8_t b2 = (addr >> 16) & 0xFF;
	uint8_t b3 = (addr >> 8) & 0xFF;
	uint8_t b4 = (addr >> 0) & 0xFF;

	snprintf(ip_cstr, sizeof(ip_cstr) - 1, "%hhu.%hhu.%hhu.%hhu", b1, b2, b3, b4);

	return ip_cstr;
}

bool IPv4Addr::fromStr(std::string ip_str) {
	char ip_cstr[16];
	uint8_t b1, b2, b3, b4;

	strncpy(ip_cstr, ip_str.c_str(), sizeof(ip_cstr) - 1);
	ip_cstr[sizeof(ip_cstr) - 1] = '\0';

	if (sscanf(ip_cstr, "%hhu.%hhu.%hhu.%hhu", &b1, &b2, &b3, &b4) != 4) {
		addr = INADDR_NONE;
		return false;
	}

	addr = (b1 << 24) | (b2 << 16) | (b3 << 8) | (b4 << 0);
	return true;
}
