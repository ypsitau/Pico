/**
 * Copyright (c) 2022 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include <stdlib.h>
#include "TCPServer.h"

static TCPServer* tcp_server_init(void)
{
    TCPServer *pTCPServer = (TCPServer*)calloc(1, sizeof(TCPServer));
    if (!pTCPServer) {
        DEBUG_printf("failed to allocate pTCPServer\n");
        return NULL;
    }
    return pTCPServer;
}

static err_t tcp_server_close(void *arg)
{
    TCPServer *pTCPServer = (TCPServer*)arg;
    err_t err = ERR_OK;
    if (pTCPServer->client_pcb != NULL) {
        tcp_arg(pTCPServer->client_pcb, NULL);
        tcp_poll(pTCPServer->client_pcb, NULL, 0);
        tcp_sent(pTCPServer->client_pcb, NULL);
        tcp_recv(pTCPServer->client_pcb, NULL);
        tcp_err(pTCPServer->client_pcb, NULL);
        err = tcp_close(pTCPServer->client_pcb);
        if (err != ERR_OK) {
            DEBUG_printf("close failed %d, calling abort\n", err);
            tcp_abort(pTCPServer->client_pcb);
            err = ERR_ABRT;
        }
        pTCPServer->client_pcb = NULL;
    }
    if (pTCPServer->server_pcb) {
        tcp_arg(pTCPServer->server_pcb, NULL);
        tcp_close(pTCPServer->server_pcb);
        pTCPServer->server_pcb = NULL;
    }
    return err;
}

static err_t tcp_server_result(void *arg, int status)
{
    TCPServer *pTCPServer = (TCPServer*)arg;
    if (status == 0) {
        DEBUG_printf("test success\n");
    } else {
        DEBUG_printf("test failed %d\n", status);
    }
    pTCPServer->complete = true;
    return tcp_server_close(arg);
}

static err_t tcp_server_sent(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
    TCPServer *pTCPServer = (TCPServer*)arg;
    DEBUG_printf("tcp_server_sent %u\n", len);
    pTCPServer->sent_len += len;

    if (pTCPServer->sent_len >= BUF_SIZE) {

        // We should get the data back from the client
        pTCPServer->recv_len = 0;
        DEBUG_printf("Waiting for buffer from client\n");
    }

    return ERR_OK;
}

err_t tcp_server_send_data(void *arg, struct tcp_pcb *tpcb)
{
    TCPServer *pTCPServer = (TCPServer*)arg;
    for(int i=0; i< BUF_SIZE; i++) {
        pTCPServer->buffer_sent[i] = rand();
    }

    pTCPServer->sent_len = 0;
    DEBUG_printf("Writing %ld bytes to client\n", BUF_SIZE);
    // this method is callback from lwIP, so cyw43_arch_lwip_begin is not required, however you
    // can use this method to cause an assertion in debug mode, if this method is called when
    // cyw43_arch_lwip_begin IS needed
    cyw43_arch_lwip_check();
    err_t err = tcp_write(tpcb, pTCPServer->buffer_sent, BUF_SIZE, TCP_WRITE_FLAG_COPY);
    if (err != ERR_OK) {
        DEBUG_printf("Failed to write data %d\n", err);
        return tcp_server_result(arg, -1);
    }
    return ERR_OK;
}

err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    TCPServer *pTCPServer = (TCPServer*)arg;
    if (!p) {
        return tcp_server_result(arg, -1);
    }
    // this method is callback from lwIP, so cyw43_arch_lwip_begin is not required, however you
    // can use this method to cause an assertion in debug mode, if this method is called when
    // cyw43_arch_lwip_begin IS needed
    cyw43_arch_lwip_check();
    if (p->tot_len > 0) {
        DEBUG_printf("tcp_server_recv %d/%d err %d\n", p->tot_len, pTCPServer->recv_len, err);

        // Receive the buffer
        const uint16_t buffer_left = BUF_SIZE - pTCPServer->recv_len;
        pTCPServer->recv_len += pbuf_copy_partial(p, pTCPServer->buffer_recv + pTCPServer->recv_len,
                                             p->tot_len > buffer_left ? buffer_left : p->tot_len, 0);
        tcp_recved(tpcb, p->tot_len);
    }
    pbuf_free(p);

    // Have we have received the whole buffer
    if (pTCPServer->recv_len == BUF_SIZE) {

        // check it matches
        if (memcmp(pTCPServer->buffer_sent, pTCPServer->buffer_recv, BUF_SIZE) != 0) {
            DEBUG_printf("buffer mismatch\n");
            return tcp_server_result(arg, -1);
        }
        DEBUG_printf("tcp_server_recv buffer ok\n");

        // Test complete?
        pTCPServer->run_count++;
        if (pTCPServer->run_count >= TEST_ITERATIONS) {
            tcp_server_result(arg, 0);
            return ERR_OK;
        }

        // Send another buffer
        return tcp_server_send_data(arg, pTCPServer->client_pcb);
    }
    return ERR_OK;
}

static err_t tcp_server_poll(void *arg, struct tcp_pcb *tpcb)
{
    DEBUG_printf("tcp_server_poll_fn\n");
    return tcp_server_result(arg, -1); // no response is an error?
}

static void tcp_server_err(void *arg, err_t err)
{
    if (err != ERR_ABRT) {
        DEBUG_printf("tcp_client_err_fn %d\n", err);
        tcp_server_result(arg, err);
    }
}

static err_t tcp_server_accept(void *arg, struct tcp_pcb *client_pcb, err_t err)
{
    TCPServer *pTCPServer = (TCPServer*)arg;
    if (err != ERR_OK || client_pcb == NULL) {
        DEBUG_printf("Failure in accept\n");
        tcp_server_result(arg, err);
        return ERR_VAL;
    }
    DEBUG_printf("Client connected\n");

    pTCPServer->client_pcb = client_pcb;
    tcp_arg(client_pcb, pTCPServer);
    tcp_sent(client_pcb, tcp_server_sent);
    tcp_recv(client_pcb, tcp_server_recv);
    tcp_poll(client_pcb, tcp_server_poll, POLL_TIME_S * 2);
    tcp_err(client_pcb, tcp_server_err);

    return tcp_server_send_data(arg, pTCPServer->client_pcb);
}

static bool tcp_server_open(void *arg)
{
    TCPServer *pTCPServer = (TCPServer*)arg;
    DEBUG_printf("Starting server at %s on port %u\n", ip4addr_ntoa(netif_ip4_addr(netif_list)), TCP_PORT);

    struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    if (!pcb) {
        DEBUG_printf("failed to create pcb\n");
        return false;
    }

    err_t err = tcp_bind(pcb, NULL, TCP_PORT);
    if (err) {
        DEBUG_printf("failed to bind to port %u\n", TCP_PORT);
        return false;
    }

    pTCPServer->server_pcb = tcp_listen_with_backlog(pcb, 1);
    if (!pTCPServer->server_pcb) {
        DEBUG_printf("failed to listen\n");
        if (pcb) {
            tcp_close(pcb);
        }
        return false;
    }

    tcp_arg(pTCPServer->server_pcb, pTCPServer);
    tcp_accept(pTCPServer->server_pcb, tcp_server_accept);

    return true;
}

void run_tcp_server_test(void)
{
    TCPServer *pTCPServer = tcp_server_init();
    if (!pTCPServer) {
        return;
    }
    if (!tcp_server_open(pTCPServer)) {
        tcp_server_result(pTCPServer, -1);
        return;
    }
    while(!pTCPServer->complete) {
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
        sleep_ms(1000);
#endif
    }
    free(pTCPServer);
}

int TCPServer::Test()
{
    if (cyw43_arch_init()) {
        printf("failed to initialise\n");
        return 1;
    }

    cyw43_arch_enable_sta_mode();

    printf("Connecting to Wi-Fi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("failed to connect.\n");
        return 1;
    } else {
        printf("Connected.\n");
    }
    run_tcp_server_test();
    cyw43_arch_deinit();
    return 0;
}
