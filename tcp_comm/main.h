
#ifndef _MAIN_H
#define _MAIN_H

#include "TcpSocket.h"
#include "tcp_common.h"

#ifdef _WINDOWS
	#define sleep_ms(x) Sleep(x)
#else
	#define sleep_ms(x) usleep(x * 1000)
#endif

extern TcpSocket server;
extern TcpSocket client;

void run_tcp_server();
void run_tcp_client();
void server_OnDataAvailable(TcpSocket *socket, const char *data, int len);
void client_OnDataAvailable(TcpSocket *socket, const char *data, int len);
void print_help(const char *exe_name);

#endif
