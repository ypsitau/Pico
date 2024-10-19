#ifndef TCPSERVER_H
#define TCPSERVER_H
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"

class TokenHandler {
public:
	enum class Stat { SkipSpace, Token, };
	enum class Type { Symbol, EndOfLine };
	static const int BuffSize = 512;
private:
	Stat stat_;
	size_t nChars_;
	char str_[BuffSize];
public:
	TokenHandler() : stat_(Stat::SkipSpace), nChars_(0) {}
	void FeedChar(char ch);
	const char* GetSymbol() const { return str_; }
	virtual void DoHandle(Type type) = 0;
};

class TCPServer {
public:
	static const int BuffSize = 2048;
	static const int POLL_TIME_S = 5;
public:
	uint16_t port_;
	TokenHandler& tokenHandler_;
	tcp_pcb *pcbServer_;
	tcp_pcb *pcbClient_;
	uint8_t buffRecv_[BuffSize];
public:
	static bool ConnectWifi(const char* ssid, const char* password, uint32_t timeout);
	static bool ConnectWifi();
	static void DisconnectWifi();
	static void PollWifi(uint32_t msec);
public:
	TCPServer(uint16_t port, TokenHandler& tokenHandler) : port_(port), tokenHandler_(tokenHandler) {}
	uint16_t GetPort() const { return port_; }
	bool WaitForClient();
	void Close();
	err_t SendData(const void* data, u16_t len);
	err_t SendString(const char* str) { return SendData(str, ::strlen(str)); }
private:
	err_t Handler_accept(tcp_pcb* client_pcb, err_t err);
	err_t Handler_sent(tcp_pcb* pcb, u16_t len);
	err_t Handler_recv(tcp_pcb* pcb, pbuf* payloadBuff, err_t err);
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
