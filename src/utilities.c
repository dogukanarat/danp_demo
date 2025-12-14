/* utilities.c - one line definition */

/* All Rights Reserved */

/* Includes */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/logging/log.h>

#include "utilities.h"

/* Imports */


/* Definitions */

#define DANP_LOG_SEVERITY DANP_LOG_VERBOSE

/* Types */


/* Forward Declarations */


/* Variables */


/* Functions */

void danp_log_message_impl(
    danp_log_level_t level,
    const char *funcName,
    const char *message,
    va_list args)
{
    if (level < DANP_LOG_SEVERITY) {
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

int32_t transaction(
    uint16_t id,
    uint16_t dPort,
    uint8_t *data,
    size_t dataLen,
    uint8_t *responseBuffer,
    size_t responseBufferLen,
    size_t *responseLen,
    uint32_t timeout)
{
    int32_t ret = 0;
    danp_socket_t *sock = NULL;
    bool isSockCreated = false;
    int32_t receivedLen;
    int32_t sentLen;

    for (;;)
    {
        sock = danp_socket(DANP_TYPE_STREAM);
        if (!sock) {
            ret = -1; // Socket creation failed
            break;
        }
        isSockCreated = true;

        ret = danp_connect(sock, id, dPort);
        if (ret != 0) {
            ret = -2; // Connection failed
            break;
        }

        sentLen = danp_send(sock, data, dataLen);
        if (sentLen < 0) {
            ret = -3; // Send failed
            break;
        }

        if (responseBuffer == NULL || responseBufferLen == 0 || responseLen == NULL) {
            // No response expected
            ret = 0;
            break;
        }

        receivedLen = danp_recv(sock, responseBuffer, responseBufferLen, timeout);
        if (receivedLen < 0) {
            ret = -4; // Receive failed
            break;
        }

        *responseLen = (size_t)receivedLen;

        break;
    }

    if (isSockCreated && sock) {
        danp_close(sock);
    }

    return ret;
}
