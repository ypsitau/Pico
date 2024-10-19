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
	err_t Handler_sent(struct tcp_pcb* tpcb, u16_t len);
	err_t Handler_recv(struct tcp_pcb* tpcb, struct pbuf* p, err_t err);
	err_t Handler_poll(struct tcp_pcb* tpcb);
	void Handler_err(err_t err);
private:
	static err_t HandlerStub_accept(void*arg, struct tcp_pcb* client_pcb, err_t err) {
		return reinterpret_cast<TCPServer*>(arg)->Handler_accept(client_pcb, err);
	}
	static err_t HandlerStub_sent(void* arg, struct tcp_pcb* tpcb, u16_t len) {
		return reinterpret_cast<TCPServer*>(arg)->Handler_sent(tpcb, len);
	}
	static err_t HandlerStub_recv(void* arg, struct tcp_pcb* tpcb, struct pbuf* p, err_t err) {
		return reinterpret_cast<TCPServer*>(arg)->Handler_recv(tpcb, p, err);
	}
	static err_t HandlerStub_poll(void* arg, struct tcp_pcb* tpcb) {
		return reinterpret_cast<TCPServer*>(arg)->Handler_poll(tpcb);
	}
	static void HandlerStub_err(void* arg, err_t err) {
		return reinterpret_cast<TCPServer*>(arg)->Handler_err(err);
	}
};

#endif
