/**
 * Copyright (c) 2022 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include <stdlib.h>
#include <memory>
#include "TCPServer.h"

bool TCPServer::Open()
{
	DEBUG_printf("Starting server at %s on port %u\n", ip4addr_ntoa(netif_ip4_addr(netif_list)), TCP_PORT);

	struct tcp_pcb* pcb = ::tcp_new_ip_type(IPADDR_TYPE_ANY);
	if (!pcb) {
		DEBUG_printf("failed to create pcb\n");
		return false;
	}

	err_t err = ::tcp_bind(pcb, NULL, TCP_PORT);
	if (err) {
		DEBUG_printf("failed to bind to port %u\n", TCP_PORT);
		return false;
	}

	server_pcb_ = ::tcp_listen_with_backlog(pcb, 1);
	if (!server_pcb_) {
		DEBUG_printf("failed to listen\n");
		if (pcb) {
			::tcp_close(pcb);
		}
		return false;
	}

	::tcp_arg(server_pcb_, this);
	::tcp_accept(server_pcb_, HandlerStub_accept);

	return true;
}

bool TCPServer::Close()
{
	bool rtn = true;
	if (client_pcb_ != NULL) {
		::tcp_arg(client_pcb_, NULL);
		::tcp_poll(client_pcb_, NULL, 0);
		::tcp_sent(client_pcb_, NULL);
		::tcp_recv(client_pcb_, NULL);
		::tcp_err(client_pcb_, NULL);
		err_t err = ::tcp_close(client_pcb_);
		if (err != ERR_OK) {
			DEBUG_printf("close failed %d, calling abort\n", err);
			::tcp_abort(client_pcb_);
			rtn = false;
		}
		client_pcb_ = NULL;
	}
	if (server_pcb_) {
		::tcp_arg(server_pcb_, NULL);
		::tcp_close(server_pcb_);
		server_pcb_ = NULL;
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

err_t TCPServer::tcp_server_send_data(struct tcp_pcb* tpcb)
{
	for(int i=0; i< BUF_SIZE; i++) {
		buffer_sent_[i] = rand();
	}

	sent_len_ = 0;
	DEBUG_printf("Writing %ld bytes to client\n", BUF_SIZE);
	// this method is callback from lwIP, so cyw43_arch_lwip_begin is not required, however you
	// can use this method to cause an assertion in debug mode, if this method is called when
	// cyw43_arch_lwip_begin IS needed
	cyw43_arch_lwip_check();
	err_t err = ::tcp_write(tpcb, buffer_sent_, BUF_SIZE, TCP_WRITE_FLAG_COPY);
	if (err != ERR_OK) {
		DEBUG_printf("Failed to write data %d\n", err);
		Complete(-1);
		return ERR_VAL;
	}
	return ERR_OK;
}


err_t TCPServer::Handler_accept(struct tcp_pcb* client_pcb, err_t err)
{
	if (err != ERR_OK || client_pcb == NULL) {
		DEBUG_printf("Failure in accept\n");
		Complete(err);
		return ERR_VAL;
	}
	DEBUG_printf("Client connected\n");

	client_pcb_ = client_pcb;
	::tcp_arg(client_pcb, this);
	::tcp_sent(client_pcb, HandlerStub_sent);
	::tcp_recv(client_pcb, HandlerStub_recv);
	::tcp_poll(client_pcb, HandlerStub_poll, POLL_TIME_S * 2);
	::tcp_err(client_pcb, HandlerStub_err);

	return tcp_server_send_data(client_pcb_);
}

err_t TCPServer::Handler_sent(struct tcp_pcb *tpcb, u16_t len)
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

err_t TCPServer::Handler_recv(struct tcp_pcb* tpcb, struct pbuf* p, err_t err)
{
	if (!p) {
		Complete(-1);
		return ERR_VAL;
	}
	// this method is callback from lwIP, so cyw43_arch_lwip_begin is not required, however you
	// can use this method to cause an assertion in debug mode, if this method is called when
	// cyw43_arch_lwip_begin IS needed
	::cyw43_arch_lwip_check();
	if (p->tot_len > 0) {
		DEBUG_printf("TCPServer::Handler_recv %d/%d err %d\n", p->tot_len, recv_len_, err);

		// Receive the buffer
		const uint16_t buffer_left = BUF_SIZE - recv_len_;
		recv_len_ += ::pbuf_copy_partial(p, buffer_recv_ + recv_len_,
											p->tot_len > buffer_left ? buffer_left : p->tot_len, 0);
		::tcp_recved(tpcb, p->tot_len);
	}
	::pbuf_free(p);

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
		return tcp_server_send_data(client_pcb_);
	}
	return ERR_OK;
}

err_t TCPServer::Handler_poll(struct tcp_pcb* tpcb)
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

int TCPServer::Test()
{
	if (::cyw43_arch_init()) {
		::printf("failed to initialise\n");
		return 1;
	}

	::cyw43_arch_enable_sta_mode();

	::printf("Connecting to Wi-Fi...\n");
	if (::cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
		::printf("failed to connect.\n");
		return 1;
	} else {
		::printf("Connected.\n");
	}
	std::unique_ptr<TCPServer> pTCPServer(new TCPServer());
	if (!pTCPServer) {
		return 1;
	}
	if (!pTCPServer->Open()) {
		pTCPServer->Complete(-1);
		return 1;
	}
	while(!pTCPServer->complete_) {
		// the following #ifdef is only here so this same example can be used in multiple modes;
		// you do not need it in your code
#if PICO_CYW43_ARCH_POLL
		// if you are using pico_cyw43_arch_poll, then you must poll periodically from your
		// main loop (not from a timer) to check for Wi-Fi driver or lwIP work that needs to be done.
		cyw43_arch_poll();
		// you can poll as often as you like, however if you have nothing else to do you can
		// choose to sleep until either a specified time, or cyw43_arch_poll() has work to do:
		cyw43_arch_wait_for_work_until(make_timeout_time_ms(1000));
#else
		// if you are not using pico_cyw43_arch_poll, then WiFI driver and lwIP work
		// is done via interrupt in the background. This sleep is just an example of some (blocking)
		// work you might be doing.
		::sleep_ms(1000);
#endif
	}
	::cyw43_arch_deinit();
	return 0;
}
