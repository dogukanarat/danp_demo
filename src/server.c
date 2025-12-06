/* server.c - one line definition */

/* All Rights Reserved */

/* Includes */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <zephyr/logging/log.h>

#include "danp/danp.h"
#include "common_definitions.h"
#include "server.h"

LOG_MODULE_REGISTER(server, LOG_LEVEL_DBG);

/* Imports */


/* Definitions */


/* Types */

typedef struct {
    danpSocket_t *serverSock;
    danpSocket_t *clientSock;
    int32_t ret;
    uint8_t buffer[DANP_MAX_PACKET_SIZE];
    int32_t received;
    char hexMsg[DANP_MAX_PACKET_SIZE * 2 + 1];
} serverRamData_t;
/* Forward Declarations */


/* Variables */

K_THREAD_STACK_DEFINE(server_stream_stack, STACK_SIZE);
static struct k_thread server_stream_data;
K_THREAD_STACK_DEFINE(server_dgram_stack, STACK_SIZE);
static struct k_thread server_dgram_data;

static serverRamData_t serverStreamRamData;
static serverRamData_t serverDgramRamData;

/* Functions */

void server_stream_thread(void *p1, void *p2, void *p3)
{
    serverRamData_t *ramData = (serverRamData_t *)p1;

    ramData->serverSock = danpSocket(DANP_TYPE_STREAM);
    if (!ramData->serverSock)
    {
        LOG_ERR("Failed to create socket");
        return;
    }

    ramData->ret = danpBind(ramData->serverSock, SERVER_STREAM_PORT);
    if (ramData->ret < 0)
    {
        LOG_ERR("Failed to bind socket");
        return;
    }

    ramData->ret = danpListen(ramData->serverSock, 1);
    if (ramData->ret < 0)
    {
        LOG_ERR("Failed to listen on socket");
        return;
    }

    LOG_INF("STREAM server listening on port %d for echo", SERVER_STREAM_PORT);

    while (1)
    {
        LOG_DBG("Waiting for client connection...");
        ramData->clientSock = danpAccept(ramData->serverSock, DANP_WAIT_FOREVER);
        if (ramData->clientSock)
        {
            LOG_INF("Client connected");

            while((ramData->received = danpRecv(ramData->clientSock, ramData->buffer, sizeof(ramData->buffer), SOCK_TIMEOUT)) > 0)
            {
                // Convert received data to hex string for logging
                for (int i = 0; i < ramData->received; i++) {
                    sprintf(&ramData->hexMsg[i * 2], "%02X", ramData->buffer[i]);
                }
                ramData->hexMsg[ramData->received * 2] = '\0';

                LOG_INF("Received %d bytes: %s", ramData->received, ramData->hexMsg);
                // Echo back the received data
                danpSend(ramData->clientSock, ramData->buffer, ramData->received);
                LOG_INF("Echoed back %d bytes", ramData->received);
            }
            LOG_INF("Client disconnected or recv error");
            danpClose(ramData->clientSock);
        }
    }
}

void server_dgram_thread(void *p1, void *p2, void *p3)
{
    serverRamData_t *ramData = (serverRamData_t *)p1;
    uint16_t dstNode;
    uint16_t dstPort;

    ramData->serverSock = danpSocket(DANP_TYPE_DGRAM);
    if (!ramData->serverSock)
    {
        LOG_ERR("Failed to create socket");
        return;
    }

    ramData->ret = danpBind(ramData->serverSock, SERVER_DGRAM_PORT);
    if (ramData->ret < 0)
    {
        LOG_ERR("Failed to bind socket");
        return;
    }

    LOG_INF("DGRAM server listening on port %d for echo", SERVER_DGRAM_PORT);

    while (1)
    {
        LOG_DBG("Waiting for datagram...");
        ramData->received = danpRecvFrom(
            ramData->serverSock, 
            ramData->buffer, 
            sizeof(ramData->buffer), 
            &dstNode, 
            &dstPort, 
            DANP_WAIT_FOREVER);
        if (ramData->received >= 0)
        {
            LOG_INF("Received %d bytes from client", ramData->received);
            // Echo back the received data
            danpSendTo(ramData->serverSock, ramData->buffer, ramData->received, dstNode, dstPort);
            LOG_INF("Echoed back %d bytes", ramData->received);
        }
    }
}

int32_t server_init(void)
{
    int32_t status = 0;
    k_tid_t server_tid;

    for (;;)
    {         
        server_tid = k_thread_create(&server_stream_data, server_stream_stack, K_THREAD_STACK_SIZEOF(server_stream_stack),
                                     server_stream_thread, &serverStreamRamData, NULL, NULL,
                                     PRIORITY, 0, K_NO_WAIT);
        if (!server_tid)
        {
            status = -1; // Thread creation failed
            break;
        }

        server_tid = k_thread_create(&server_dgram_data, server_dgram_stack, K_THREAD_STACK_SIZEOF(server_dgram_stack),
                                     server_dgram_thread, &serverDgramRamData, NULL, NULL,
                                     PRIORITY, 0, K_NO_WAIT);
        if (!server_tid)
        {
            status = -1; // Thread creation failed
            break;
        }

        break;
    }

    return status;
}