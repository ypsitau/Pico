/**
 * Copyright (c) 2022 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include <stdlib.h>
#include <memory>
#include "TCPServer.h"

#define DEBUG_printf printf

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

bool TCPServer::Wait(uint16_t port)
{
	DEBUG_printf("Starting server at %s on port %u\n", ip4addr_ntoa(netif_ip4_addr(netif_list)), port);
	tcp_pcb* pcbListen = ::tcp_new_ip_type(IPADDR_TYPE_ANY);
	if (!pcbListen) {
		DEBUG_printf("failed to create pcbListen\n");
		return false;
	}
	err_t err = ::tcp_bind(pcbListen, nullptr, port);
	if (err) {
		DEBUG_printf("failed to bind to port %u\n", port);
		return false;
	}
	pcbServer_ = ::tcp_listen_with_backlog(pcbListen, 1);
	if (!pcbServer_) {
		DEBUG_printf("failed to listen\n");
		if (pcbListen) {
			::tcp_close(pcbListen);
		}
		return false;
	}
	::tcp_arg(pcbServer_, this);
	::tcp_accept(pcbServer_, HandlerStub_accept);
	return true;
}

bool TCPServer::Close()
{
	bool rtn = true;
	if (pcbClient_) {
		::tcp_arg(pcbClient_, nullptr);
		::tcp_poll(pcbClient_, nullptr, 0);
		::tcp_sent(pcbClient_, nullptr);
		::tcp_recv(pcbClient_, nullptr);
		::tcp_err(pcbClient_, nullptr);
		err_t err = ::tcp_close(pcbClient_);
		if (err != ERR_OK) {
			DEBUG_printf("close failed %d, calling abort\n", err);
			::tcp_abort(pcbClient_);
			rtn = false;
		}
		pcbClient_ = nullptr;
	}
	if (pcbServer_) {
		::tcp_arg(pcbServer_, nullptr);
		::tcp_close(pcbServer_);
		pcbServer_ = nullptr;
	}
	return rtn;
}

void TCPServer::Complete(int status)
{
	if (status == 0) {
		DEBUG_printf("test success\n");
	} else {
		DEBUG_printf("test failed %d\n", status);
	}
	complete_ = true;
	Close();
}

err_t TCPServer::SendData(tcp_pcb* pcb)
{
	for(int i=0; i< BUF_SIZE; i++) {
		buffer_sent_[i] = rand();
	}
	sent_len_ = 0;
	DEBUG_printf("Writing %ld bytes to client\n", BUF_SIZE);
	// this method is callback from lwIP, so cyw43_arch_lwip_begin is not required, however you
	// can use this method to cause an assertion in debug mode, if this method is called when
	// cyw43_arch_lwip_begin IS needed
	::cyw43_arch_lwip_check();
	err_t err = ::tcp_write(pcb, buffer_sent_, BUF_SIZE, TCP_WRITE_FLAG_COPY);
	if (err != ERR_OK) {
		DEBUG_printf("Failed to write data %d\n", err);
		Complete(-1);
		return ERR_VAL;
	}
	return ERR_OK;
}


err_t TCPServer::Handler_accept(tcp_pcb* pcbClient, err_t err)
{
	if (err != ERR_OK || !pcbClient) {
		DEBUG_printf("Failure in accept\n");
		Complete(err);
		return ERR_VAL;
	}
	pcbClient_ = pcbClient;
	::tcp_arg(pcbClient, this);
	::tcp_sent(pcbClient, HandlerStub_sent);
	::tcp_recv(pcbClient, HandlerStub_recv);
	::tcp_poll(pcbClient, HandlerStub_poll, POLL_TIME_S * 2);
	::tcp_err(pcbClient, HandlerStub_err);
	return SendData(pcbClient_);
}

err_t TCPServer::Handler_sent(tcp_pcb* pcb, u16_t len)
{
	DEBUG_printf("TCPServer::Handler_sent %u\n", len);
	sent_len_ += len;

	if (sent_len_ >= BUF_SIZE) {

		// We should get the data back from the client
		recv_len_ = 0;
		DEBUG_printf("Waiting for buffer from client\n");
	}

	return ERR_OK;
}

err_t TCPServer::Handler_recv(tcp_pcb* pcb, pbuf* payloadBuf, err_t err)
{
	if (!payloadBuf) {
		Complete(-1);
		return ERR_VAL;
	}
	// this method is callback from lwIP, so cyw43_arch_lwip_begin is not required, however you
	// can use this method to cause an assertion in debug mode, if this method is called when
	// cyw43_arch_lwip_begin IS needed
	::cyw43_arch_lwip_check();
	if (payloadBuf->tot_len > 0) {
		DEBUG_printf("TCPServer::Handler_recv %d/%d err %d\n", payloadBuf->tot_len, recv_len_, err);

		// Receive the buffer
		const uint16_t buffer_left = BUF_SIZE - recv_len_;
		recv_len_ += ::pbuf_copy_partial(payloadBuf, buffer_recv_ + recv_len_,
											payloadBuf->tot_len > buffer_left ? buffer_left : payloadBuf->tot_len, 0);
		::tcp_recved(pcb, payloadBuf->tot_len);
	}
	::pbuf_free(payloadBuf);

	// Have we have received the whole buffer
	if (recv_len_ == BUF_SIZE) {

		// check it matches
		if (::memcmp(buffer_sent_, buffer_recv_, BUF_SIZE) != 0) {
			DEBUG_printf("buffer mismatch\n");
			Complete(-1);
			return ERR_VAL;
		}
		DEBUG_printf("TCPServer::Handler_recv buffer ok\n");

		// Test complete?
		run_count_++;
		if (run_count_ >= TEST_ITERATIONS) {
			Complete(0);
			return ERR_OK;
		}

		// Send another buffer
		return SendData(pcbClient_);
	}
	return ERR_OK;
}

err_t TCPServer::Handler_poll(tcp_pcb* pcb)
{
	DEBUG_printf("tcp_server_poll_fn\n");
	Complete(-1); // no response is an error?
	return ERR_VAL;
}

void TCPServer::Handler_err(err_t err)
{
	if (err != ERR_ABRT) {
		DEBUG_printf("tcp_client_err_fn %d\n", err);
		Complete(err);
	}
}
