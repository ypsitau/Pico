/**
 * Copyright (c) 2022 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include <stdlib.h>
#include <memory>
#include "TCPServer.h"

//------------------------------------------------------------------------------
// TokenHandler
//------------------------------------------------------------------------------
void TokenHandler::FeedChar(char ch)
{
	//::printf("%c %02x\n", ch, ch);
	switch (stat_) {
	case Stat::SkipSpace: {
		if (ch == '\0' || ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
			// nothing to do
		} else {
			nChars_ = 0;
			str_[nChars_++] = ch;
			stat_ = Stat::Token;
		}
		break;
	}
	case Stat::Token: {
		if (ch == '\0' || ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
			str_[nChars_] = '\0';
			DoHandle(Type::Symbol);
			if (ch == '\n' || ch == '\r') DoHandle(Type::EndOfLine);
			stat_ = Stat::SkipSpace;
		} else if (nChars_ + 1 < BuffSize) {
			str_[nChars_++] = ch;
		}
		break;
	}
	default: {
		break;
	}
	}
}

//------------------------------------------------------------------------------
// TCPServer
//------------------------------------------------------------------------------
bool TCPServer::ConnectWifi(const char* ssid, const char* password, uint32_t timeout)
{
	if (::cyw43_arch_init()) return false;
	::cyw43_arch_enable_sta_mode();
	return ::cyw43_arch_wifi_connect_timeout_ms(ssid, password, CYW43_AUTH_WPA2_AES_PSK, timeout) == 0;
}

void TCPServer::DisconnectWifi()
{
	::cyw43_arch_deinit();
}

void TCPServer::PollWifi(uint32_t msec)
{
	::cyw43_arch_poll();
	::cyw43_arch_wait_for_work_until(::make_timeout_time_ms(msec));
}

bool TCPServer::WaitForClient()
{
	tcp_pcb* pcbListen = ::tcp_new_ip_type(IPADDR_TYPE_ANY);
	if (!pcbListen) return false;
	if (::tcp_bind(pcbListen, nullptr, port_) != ERR_OK) return false;
	pcbServer_ = ::tcp_listen_with_backlog(pcbListen, 1);
	if (!pcbServer_) {
		::tcp_close(pcbListen);
		return false;
	}
	::tcp_arg(pcbServer_, this);
	::tcp_accept(pcbServer_, HandlerStub_accept);
	return true;
}

void TCPServer::Close()
{
	if (pcbClient_) {
		::tcp_poll(pcbClient_, nullptr, 0);
		::tcp_sent(pcbClient_, nullptr);
		::tcp_recv(pcbClient_, nullptr);
		::tcp_err(pcbClient_, nullptr);
		::tcp_close(pcbClient_);
		pcbClient_ = nullptr;
	}
	if (pcbServer_) {
		::tcp_accept(pcbServer_, nullptr);
		::tcp_close(pcbServer_);
		pcbServer_ = nullptr;
	}
}

err_t TCPServer::SendData(const void* data, u16_t len)
{
	return ::tcp_write(pcbClient_, data, len, TCP_WRITE_FLAG_COPY);
}


err_t TCPServer::Handler_accept(tcp_pcb* pcbClient, err_t err)
{
	pcbClient_ = pcbClient;
	::tcp_arg(pcbClient_, this);
	::tcp_sent(pcbClient_, HandlerStub_sent);
	::tcp_recv(pcbClient_, HandlerStub_recv);
	::tcp_poll(pcbClient_, HandlerStub_poll, POLL_TIME_S * 2);
	::tcp_err(pcbClient_, HandlerStub_err);
	::printf("connected %s\n", ::ip4addr_ntoa(&pcbClient_->remote_ip));
	SendString("Ready\n");
	return ERR_OK;
}

err_t TCPServer::Handler_sent(tcp_pcb* pcb, u16_t len)
{
	//::printf("TCPServer::Handler_sent: %dbytes\n", len);
	return ERR_OK;
}

err_t TCPServer::Handler_recv(tcp_pcb* pcb, pbuf* payloadBuff, err_t err)
{
	if (!payloadBuff) {
		//::printf("TCPServer::Handler_recv: disconnected err=%d\n", err);
		Close();
		WaitForClient();
		return ERR_OK;
	}
	//::printf("TCPServer::Handler_recv: %dbytes err=%d\n", payloadBuff->tot_len, err);
	//::cyw43_arch_lwip_check();
	u16_t len = payloadBuff->tot_len;
	if (len > 0) {
		::pbuf_copy_partial(payloadBuff, buffRecv_, BuffSize, 0);
		const uint8_t* p = buffRecv_;
		for (u16_t i = 0; i < len; i++, p++) tokenHandler_.FeedChar(static_cast<char>(*p));
		::tcp_recved(pcb, payloadBuff->tot_len);
	}
	::pbuf_free(payloadBuff);
	return ERR_OK;
}

err_t TCPServer::Handler_poll(tcp_pcb* pcb)
{
	//::printf("Handler_poll\n");
	return ERR_OK;
}

void TCPServer::Handler_err(err_t err)
{
	::printf("Handler_err\n");
	if (err != ERR_ABRT) {
		::printf("tcp_client_err_fn %d\n", err);
		Close();
	}
}
