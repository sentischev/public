#ifndef _TCP_SOCKET_H
#define _TCP_SOCKET_H

#include <stdint.h>
#include <string>

#ifdef _WINDOWS
	#include <winsock2.h>
	#include <windows.h>

	#define tcp_thread_ret_t DWORD WINAPI

	typedef SOCKET tcp_socket_t;
	typedef HANDLE tcp_thread_t;
	typedef int socklen_t;
#else
	#include <pthread.h>
	#include <unistd.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <netdb.h>

	#define tcp_thread_ret_t void *
	#define INVALID_SOCKET -1
	#define SOCKET_ERROR -1

	typedef int tcp_socket_t;
	typedef pthread_t tcp_thread_t;
#endif

class TcpSocket;

typedef void (*TcpSocketOnDataAvailableCallback)(TcpSocket *socket, const char *data, int len);

class IPv4Addr {
public:
	uint32_t addr;

	IPv4Addr() {
		addr = INADDR_NONE;
	}

	IPv4Addr(uint32_t addr) {
		this->addr = addr;
	}

	IPv4Addr(std::string ip_str) {
		fromStr(ip_str);
	}

	std::string toStr();
	bool fromStr(std::string ip_str);
};

class TcpSocket {
public:
	IPv4Addr ip;

	TcpSocket();
	TcpSocket(tcp_socket_t tcp_socket);
	virtual ~TcpSocket();

	// client mode: connect to server
	bool connect(const char *host, uint16_t hostPort);
	bool connect(uint32_t serverIPAddress, uint16_t serverPort);

	// server mode: listen and accept clients
	bool listen(uint16_t port, int max_clients = 3);
	TcpSocket *accept();

	// data transfer
	int send(const char *data, int len);
	int recv(char *data, int max_len);

	// callback setup and background thread handling
	TcpSocketOnDataAvailableCallback OnDataAvailable;
	void startReceiveThread();

	void close();
	bool isConnected();
	bool isServer();

private:
	tcp_socket_t m_sock;
	bool m_isConnected;
	bool m_isServer;
	tcp_thread_t m_receiveThread;
	bool m_receiveThreadRunning;

	// background thread wrapper function
	static tcp_thread_ret_t tcp_receive_loop(void *arg);
	void run_tcp_receive_loop();

	// prevent copying
	TcpSocket(const TcpSocket&);
	TcpSocket& operator=(const TcpSocket&) { return *this; };

	static int s_initCount;
	static void initWinsock();
	static void cleanupWinsock();
};

#endif /* _TCP_SOCKET_H */
