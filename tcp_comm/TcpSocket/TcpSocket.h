#ifndef _TCP_SOCKET_H
#define _TCP_SOCKET_H

#include <stdint.h>

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

	#define tcp_thread_ret_t void *

	typedef int tcp_socket_t;
	typedef pthread_t tcp_thread_t;

	#define INVALID_SOCKET -1
	#define SOCKET_ERROR -1
#endif

class TcpSocket;

typedef void (*TcpSocketOnDataAvailableCallback)(TcpSocket *socket, const char *data, int len);

class TcpSocket {
public:
	TcpSocket();
	TcpSocket(tcp_socket_t tcp_socket);
	virtual ~TcpSocket();

	// client mode: connect to server
	bool tcp_connect(const char *host, uint16_t hostPort);
	bool tcp_connect(uint32_t serverIPAddress, uint16_t serverPort);

	// server mode: listen and accept clients
	bool tcp_listen(uint16_t port, int max_clients = 3);
	TcpSocket *tcp_accept();

	// data transfer
	int tcp_send(const char *data, int len);
	int tcp_receive(char *data, int max_len);

	// callback setup and background thread handling
	TcpSocketOnDataAvailableCallback OnDataAvailable;
	void startReceiveThread();

	void tcp_close();
	bool isConnected();

private:
	tcp_socket_t m_sock;
	bool m_isConnected;
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
