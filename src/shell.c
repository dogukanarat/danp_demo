/* cli.c - one line definition */

/* All Rights Reserved */

/* Includes */

#include <zephyr/shell/shell.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "danp/danp.h"

/* Imports */


/* Definitions */

LOG_MODULE_DECLARE(danp);

/* Types */


/* Forward Declarations */

static int danp_transaction(const struct shell *shell, size_t argc, char **argv);
static int danp_test(const struct shell *shell, size_t argc, char **argv);
static int danp_stats(const struct shell *shell, size_t argc, char **argv);

/* Variables */

/* Shell Commands */

SHELL_STATIC_SUBCMD_SET_CREATE(sub_danp_cmds,
    SHELL_CMD(
        transaction,
        NULL,
        "Send/receive message\nUsage: danp transaction <id> <dPort> [<dataHex>] [<timeout>]",
        danp_transaction
    ),
    SHELL_CMD(
        test,
        NULL,
        "Run DANP test (not implemented yet)\nUsage: danp test <dgram|stream> <count> <size> <id> <dPort> <interval>",
        danp_test
    ),
    SHELL_CMD(
        stats,
        NULL,
        "Print DANP statistics",
        danp_stats
    ),
    SHELL_SUBCMD_SET_END
);

SHELL_CMD_REGISTER(danp, &sub_danp_cmds, "Base command for DANP operations", NULL);

/* Functions */

static void danp_print_func(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

static int danp_transaction(const struct shell *shell, size_t argc, char **argv) {
    if (argc < 2) {
        shell_print(shell, "Usage: danp transaction <id> <dPort> [<dataHex>] [<timeout>]");
        return -EINVAL;
    }

    danp_socket_t *sock;
    int32_t ret;
    uint8_t buffer[DANP_MAX_PACKET_SIZE];
    int32_t received = 0;
    uint16_t id = (uint16_t)atoi(argv[1]);
    uint16_t dPort = (uint16_t)atoi(argv[2]);
    uint8_t data[DANP_MAX_PACKET_SIZE];
    size_t dataLen = 0;

    // Parse data if provided
    if (argc >= 4) {
        const char *dataHex = argv[3];
        dataLen = strlen(dataHex) / 2;
        for (size_t i = 0; i < dataLen; i++) {
            sscanf(&dataHex[i * 2], "%2hhx", &data[i]);
        }
    }

    ret = transaction(
        id,
        dPort,
        data,
        dataLen,
        buffer,
        sizeof(buffer),
        (size_t *)&received,
        2000);
    if (ret == 0) {
        LOG_INF("Transaction successful, received %d bytes:", received);
        LOG_HEXDUMP_INF(buffer, received, "Received data");
    } else {
        LOG_ERR("Transaction failed with error code: %d", ret);
    }

    return 0;
}

static int danp_test(const struct shell *shell, size_t argc, char **argv)
{
    int ret = 0;
    if (argc < 7) {
        shell_print(shell, "Usage: danp test <dgram|stream> <count> <size> <id> <dPort> <interval>");
        return -EINVAL;
    }

    const char *type = argv[1];
    int count = atoi(argv[2]);
    int size = atoi(argv[3]);
    uint16_t id = (uint16_t)atoi(argv[4]);
    uint16_t dPort = (uint16_t)atoi(argv[5]);
    int interval = atoi(argv[6]);
    danp_socket_t *sock = NULL;
    uint8_t *tx_buf = NULL;
    uint8_t *rx_buf = NULL;

    for (;;)
    {
        // Safety check for packet size vs buffer size
        if (size > DANP_MAX_PACKET_SIZE) {
            shell_error(shell, "Size %d exceeds max packet size", size);
            ret = -EINVAL;
            break;
        }

        shell_print(shell, "Running DANP test: type=%s, count=%d, size=%d, id=%u, dPort=%u",
                    type,
                    count,
                    size,
                    id,
                    dPort);

        if (strcmp(type, "dgram") == 0) {
            sock = danp_socket(DANP_TYPE_DGRAM);
        } else if (strcmp(type, "stream") == 0) {
            sock = danp_socket(DANP_TYPE_STREAM);
        } else {
            shell_error(shell, "Invalid type");
            ret = -EINVAL;
            break;
        }
        if (!sock) {
            shell_error(shell, "Failed to create socket");
            ret = -ENOMEM;
            break;
        }

        int ret = danp_connect(sock, id, dPort);
        if (ret < 0) {
            shell_error(shell, "Failed to connect socket");
            ret = -ECONNREFUSED;
            break;
        }

        tx_buf = k_malloc(DANP_MAX_PACKET_SIZE);
        rx_buf = k_malloc(DANP_MAX_PACKET_SIZE);

        if (!tx_buf || !rx_buf) {
            ret = -ENOMEM;
            break;
        }

        for (int j = 0; j < size; j++) {
            tx_buf[j] = (uint8_t)(j & 0xFF);
        }

        for (int i = 0; i < count; i++) {
            // Check for abort (allows user to Ctrl+C if shell supports it, or system shutdown)
            if (k_is_in_isr()) break;

            shell_print(shell, "Iteration %d/%d", i + 1, count);

            int ret = danp_send(sock, tx_buf, size);
            if (ret < 0) {
                shell_error(shell, "Send failed iter %d", i);
                // Decide: break or continue?
            } else {
                // Only try to receive if send was okay
                ret = danp_recv(sock, rx_buf, DANP_MAX_PACKET_SIZE, 2000);
                if (ret < 0) {
                    shell_warn(shell, "Recv timeout/fail iter %d", i + 1);
                } else {
                    // Verify data
                    if (ret != size || memcmp(tx_buf, rx_buf, size) != 0) {
                        if (ret != size) {
                            shell_error(shell, "Data size mismatch iter %d: sent %d, recv %d", i + 1, size, ret);
                        } else {
                            shell_error(shell, "Data content mismatch iter %d", i + 1);
                        }
                    } else {
                        shell_print(shell, "Iteration %d successful", i + 1);
                    }
                }
            }

            // 4. Use the interval argument
            if (interval > 0) {
                k_msleep(interval);
            }
        }

        break;
    }

    if (sock)
    {
        danp_close(sock);
    }
    if (tx_buf)
    {
        k_free(tx_buf);
    }
    if (rx_buf)
    {
        k_free(rx_buf);
    }

    shell_print(shell, "Test complete");

    return 0;
}

static int danp_stats(const struct shell *shell, size_t argc, char **argv) {
    danp_print_stats((void (*)(const char *, ...))danp_print_func);
    return 0;
}
