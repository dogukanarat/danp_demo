#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include "danp/drivers/danpLo.h"
#include "danp/danp.h"

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

#define STACK_SIZE      (4 * 1024)
#define PRIORITY        (7)
#define SERVER_PORT     (10)
#define LOCAL_NODE      (1)

K_THREAD_STACK_DEFINE(server_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(client_stack, STACK_SIZE);

struct k_thread server_data;
struct k_thread client_data;

static void logMessage(
    danpLogLevel_t level,
    const char *funcName,
    const char *message,
    va_list args)
{
    if (level < DANP_LOG_WARN) {
        return; // Filter out verbose logs
    }
    const char *color;
    switch (level) {
        case DANP_LOG_ERROR:   color = "\x1B[31m"; break; // Red
        case DANP_LOG_WARN:    color = "\x1B[33m"; break; // Yellow
        case DANP_LOG_INFO:    color = "\x1B[32m"; break; // Green
        case DANP_LOG_VERBOSE: color = "\x1B[36m"; break; // Cyan
        default:               color = "\x1B[0m";  break; // Reset
    }
    printk("%s%s: ", color, funcName);
    vprintk(message, args);
    printk("\x1B[0m\n"); // Reset color
}

void server_thread(void *p1, void *p2, void *p3)
{
    danpSocket_t *serverSock;
    danpSocket_t *clientSock;
    int32_t ret;
    uint8_t buffer[DANP_MAX_PACKET_SIZE];
    int32_t received;

    printk("Server: Starting...\n");

    serverSock = danpSocket(DANP_TYPE_STREAM);
    if (!serverSock) {
        printk("Server: Failed to create socket\n");
        return;
    }

    ret = danpBind(serverSock, SERVER_PORT);
    if (ret < 0) {
        printk("Server: Failed to bind\n");
        return;
    }

    ret = danpListen(serverSock, 1);
    if (ret < 0) {
        printk("Server: Failed to listen\n");
        return;
    }

    printk("Server: Listening on port %d\n", SERVER_PORT);

    while (1) {
        clientSock = danpAccept(serverSock, DANP_WAIT_FOREVER);
        if (clientSock) {
            printk("Server: Client connected\n");

            while ((received = danpRecv(clientSock, buffer, sizeof(buffer), DANP_WAIT_FOREVER)) > 0) {
                buffer[received] = '\0'; // Null-terminate for printing
                printk("Server: Received '%s', echoing...\n", buffer);
                danpSend(clientSock, buffer, received);
            }

            printk("Server: Client disconnected\n");
            danpClose(clientSock);
        }
    }
}

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
        ret = danpConnect(sock, LOCAL_NODE, SERVER_PORT);
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

int main(void)
{
    int ret;

    if (!gpio_is_ready_dt(&led)) {
        return 0;
    }

    danpLoInterface_t ifaceLo;
    danpConfig_t config = {
        .localNode = LOCAL_NODE,
        .logFunction = logMessage,
    };

    danpLoInit(&ifaceLo, LOCAL_NODE);
    danpInit(&config);
    danpRegisterInterface(&ifaceLo);

    k_thread_create(&server_data, server_stack, K_THREAD_STACK_SIZEOF(server_stack),
                    server_thread, NULL, NULL, NULL,
                    PRIORITY, 0, K_NO_WAIT);

    k_thread_create(&client_data, client_stack, K_THREAD_STACK_SIZEOF(client_stack),
                    client_thread, NULL, NULL, NULL,
                    PRIORITY, 0, K_NO_WAIT);

    ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
    if (ret < 0) {
        return 0;
    }

    printk("Hello from Nucleo-F767ZI!\n");

    while (1) {
        ret = gpio_pin_toggle_dt(&led);
        k_msleep(1000); // Sleep for 1 second
    }
    return 0;
}