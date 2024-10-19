#ifndef TCPSERVER_H
#define TCPSERVER_H
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"

#define TCP_PORT 4242
#define DEBUG_printf printf
#define BUF_SIZE 2048
#define TEST_ITERATIONS 10
#define POLL_TIME_S 5

class TCPServer {
public:
	struct tcp_pcb *server_pcb;
	struct tcp_pcb *client_pcb;
	bool complete;
	uint8_t buffer_sent[BUF_SIZE];
	uint8_t buffer_recv[BUF_SIZE];
	int sent_len;
	int recv_len;
	int run_count;
public:
	static int Test();
};

#endif
