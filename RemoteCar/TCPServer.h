#ifndef TCPSERVER_H
#define TCPSERVER_H
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"

class TCPServer {
public:
	static const int BUF_SIZE = 2048;
	static const int TEST_ITERATIONS = 10;
	static const int POLL_TIME_S = 5;
public:
	tcp_pcb *pcbServer_;
	tcp_pcb *pcbClient_;
	bool complete_;
	uint8_t buffer_sent_[BUF_SIZE];
	uint8_t buffer_recv_[BUF_SIZE];
	int sent_len_;
	int recv_len_;
	int run_count_;
public:
	static bool ConnectWifi(const char* ssid, const char* password, uint32_t timeout);
	static bool ConnectWifi();
	static void DisconnectWifi();
	static void PollWifi(uint32_t msec);
public:
	bool Wait(uint16_t port);
	bool Close();
	err_t SendData(tcp_pcb* pcb);
	void Complete(int status);
private:
	err_t Handler_accept(tcp_pcb* client_pcb, err_t err);
	err_t Handler_sent(tcp_pcb* pcb, u16_t len);
	err_t Handler_recv(tcp_pcb* pcb, pbuf* payloadBuf, err_t err);
	err_t Handler_poll(tcp_pcb* pcb);
	void Handler_err(err_t err);
private:
	static err_t HandlerStub_accept(void*arg, tcp_pcb* client_pcb, err_t err) {
		return reinterpret_cast<TCPServer*>(arg)->Handler_accept(client_pcb, err);
	}
	static err_t HandlerStub_sent(void* arg, tcp_pcb* pcb, u16_t len) {
		return reinterpret_cast<TCPServer*>(arg)->Handler_sent(pcb, len);
	}
	static err_t HandlerStub_recv(void* arg, tcp_pcb* pcb, pbuf* p, err_t err) {
		return reinterpret_cast<TCPServer*>(arg)->Handler_recv(pcb, p, err);
	}
	static err_t HandlerStub_poll(void* arg, tcp_pcb* pcb) {
		return reinterpret_cast<TCPServer*>(arg)->Handler_poll(pcb);
	}
	static void HandlerStub_err(void* arg, err_t err) {
		return reinterpret_cast<TCPServer*>(arg)->Handler_err(err);
	}
};

#endif
