/* client.c - one line definition */

/* All Rights Reserved */

/* Includes */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

#include "danp/danp.h"
#include "common_definitions.h"
#include "client.h"

/* Imports */


/* Definitions */


/* Types */


/* Forward Declarations */


/* Variables */

K_THREAD_STACK_DEFINE(client_stack, STACK_SIZE);
struct k_thread client_data;

/* Functions */

void client_thread(void *p1, void *p2, void *p3)
{
    danpSocket_t *sock;
    int32_t ret;
    char *msg[32];
    uint32_t msgCount = 0;
    uint8_t buffer[DANP_MAX_PACKET_SIZE];
    int32_t received;

    // Give server time to start
    k_sleep(K_SECONDS(1));

    printk("Client: Starting...\n");

    while (1) {
        sock = danpSocket(DANP_TYPE_STREAM);
        if (!sock) {
            printk("Client: Failed to create socket\n");
            k_sleep(K_SECONDS(1));
            continue;
        }

        printk("Client: Connecting to server...\n");
        ret = danpConnect(sock, LOCAL_NODE, SERVER_STREAM_PORT);
        if (ret == 0) {
            printk("Client: Connected\n");

            sprintf(msg, "Message %d from client", msgCount++);
            printk("Client: Sending '%s'\n", msg);
            danpSend(sock, msg, strlen(msg));

            received = danpRecv(sock, buffer, sizeof(buffer), 1000);
            if (received > 0) {
                buffer[received] = '\0';
                printk("Client: Received echo '%s'\n", buffer);
            } else {
                printk("Client: Receive timeout or error\n");
            }
            
            danpClose(sock);
        } else {
            printk("Client: Failed to connect\n");
            danpClose(sock);
        }

        k_sleep(K_SECONDS(2));
    }
}

int32_t client_init(void)
{
    int32_t status = 0;

    for (;;)
    {
        // k_thread_create(&client_data, client_stack, K_THREAD_STACK_SIZEOF(client_stack),
        //         client_thread, NULL, NULL, NULL,
        //         PRIORITY, 0, K_NO_WAIT);

        break;
    }

    return status;
}