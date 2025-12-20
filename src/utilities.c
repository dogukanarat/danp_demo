/* utilities.c - one line definition */

/* All Rights Reserved */

/* Includes */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/logging/log.h>

#include "utilities.h"

/* Imports */


/* Definitions */

LOG_MODULE_REGISTER(danp, LOG_LEVEL_DBG);

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
    char log_buf[256];
    vsnprintf(log_buf, sizeof(log_buf), message, args);

    switch (level) {
    case DANP_LOG_ERROR:
        LOG_ERR("%s", log_buf);
        break;
    case DANP_LOG_WARN:
        LOG_WRN("%s", log_buf);
        break;
    case DANP_LOG_INFO:
        LOG_INF("%s", log_buf);
        break;
    case DANP_LOG_DEBUG:
    case DANP_LOG_VERBOSE:
    default:
        LOG_DBG("%s", log_buf);
        break;
    }

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
