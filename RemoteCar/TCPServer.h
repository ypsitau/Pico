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
	struct tcp_pcb *server_pcb_;
	struct tcp_pcb *client_pcb_;
	bool complete_;
	uint8_t buffer_sent_[BUF_SIZE];
	uint8_t buffer_recv_[BUF_SIZE];
	int sent_len_;
	int recv_len_;
	int run_count_;
public:
	static TCPServer* Init();
	bool Open();
	bool Close();
	static int Test();
private:
	err_t Handler_accept(struct tcp_pcb* client_pcb, err_t err);
private:
	static err_t HandlerStub_accept(void*arg, struct tcp_pcb* client_pcb, err_t err) {
		return reinterpret_cast<TCPServer*>(arg)->Handler_accept(client_pcb, err);
	}
};

#endif
