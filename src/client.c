/* client.c - one line definition */

/* All Rights Reserved */

/* Includes */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

#include "danp/danp.h"
#include "common_definitions.h"
#include "client.h"

/* Imports */


/* Definitions */

LOG_MODULE_REGISTER(client, LOG_LEVEL_DBG);

/* Types */


/* Forward Declarations */


/* Variables */

K_THREAD_STACK_DEFINE(client_stack, STACK_SIZE);
struct k_thread client_data;

/* Functions */

void client_thread(void *p1, void *p2, void *p3)
{
    danp_socket_t *sock;
    int32_t ret;
    char *msg[32];
    uint32_t msgCount = 0;
    uint8_t buffer[DANP_MAX_PACKET_SIZE];
    int32_t received;

    // Give server time to start
    k_sleep(K_SECONDS(1));

    LOG_INF("Starting...");

    while (1) {
        sock = danp_socket(DANP_TYPE_DGRAM);
        if (!sock) {
            LOG_ERR("Failed to create socket");
            k_sleep(K_SECONDS(1));
            continue;
        }

        LOG_INF("Connecting to server...");
        ret = danp_connect(sock, REMOTE_NODE_ID, SERVER_DGRAM_PORT);
        if (ret == 0) {
            LOG_INF("Connected");

            sprintf(msg, "Message %d from client", msgCount++);
            LOG_INF("Sending '%s'", msg);
            danp_send(sock, msg, strlen(msg));

            received = danp_recv(sock, buffer, sizeof(buffer), 1000);
            if (received > 0) {
                buffer[received] = '\0';
                LOG_INF("Received echo '%s'", buffer);
            } else {
                LOG_ERR("Receive timeout or error");
            }

            danp_close(sock);
        } else {
            LOG_ERR("Failed to connect");
            danp_close(sock);
        }

        k_sleep(K_SECONDS(5));
    }
}

int32_t client_init(void)
{
    int32_t status = 0;

    for (;;)
    {
        k_thread_create(&client_data, client_stack, K_THREAD_STACK_SIZEOF(client_stack),
                client_thread, NULL, NULL, NULL,
                PRIORITY, 0, K_NO_WAIT);

        break;
    }

    return status;
}
