/* main.c - one line definition */

/* All Rights Reserved */

/* Includes */

#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/radio_ctrl.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include "danp/drivers/danp_lo.h"
#include "danp/danp.h"
#include "common_definitions.h"
#include "utilities.h"
#include "server.h"
#include "client.h"

/* Imports */

/* Definitions */

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)

#define FREQ_EU_868_HZ (868300000)
#define SYNC_WORD_PUBLIC (0x34)
#define SYNC_WORD_PRIVATE (0x12)
#define SYNC_WORD_NO_RADIO (0x21)

/* Types */

/* Forward Declarations */

/* Variables */

static const struct device *const radio = DEVICE_DT_GET(DT_ALIAS(radio0));

static ralf_params_lora_t defaukt_lora_rx_param = {
    .mod_params =
        {
            .sf = RAL_LORA_SF8,
            .bw = RAL_LORA_BW_125_KHZ,
            .cr = RAL_LORA_CR_4_5,
            .ldro = 0, /* to be computed later */
        },
    .pkt_params =
        {
            .preamble_len_in_symb = 8,
            .header_type = RAL_LORA_PKT_EXPLICIT,
            .pld_len_in_bytes = 255,
            .crc_is_on = true,
            .invert_iq_is_on = false,
        },
    .rf_freq_in_hz = FREQ_EU_868_HZ,
    .output_pwr_in_dbm = 14,
    .sync_word = SYNC_WORD_PUBLIC,
    .symb_nb_timeout = 0,
};

static ralf_params_lora_t default_lora_tx_param = {
    .mod_params =
        {
            .sf = RAL_LORA_SF8,
            .bw = RAL_LORA_BW_125_KHZ,
            .cr = RAL_LORA_CR_4_5,
            .ldro = 0, /* to be computed later */
        },
    .pkt_params =
        {
            .preamble_len_in_symb = 8,
            .header_type = RAL_LORA_PKT_EXPLICIT,
            .pld_len_in_bytes = 255,
            .crc_is_on = true,
            .invert_iq_is_on = false,
        },
    .rf_freq_in_hz = FREQ_EU_868_HZ,
    .output_pwr_in_dbm = 14,
    .sync_word = SYNC_WORD_PUBLIC,
    .symb_nb_timeout = 0,
};

static const ralf_params_lora_cad_t default_lora_cad_param = {
    .ral_lora_cad_params =
        {
            .cad_symb_nb = RAL_LORA_CAD_16_SYMB,
            .cad_det_peak_in_symb = 21,
            .cad_det_min_in_symb = 10,
            .cad_exit_mode = RAL_LORA_CAD_ONLY,
            .cad_timeout_in_ms = 0,
        },
    .sf = RAL_LORA_SF8,
    .bw = RAL_LORA_BW_125_KHZ,
    .rf_freq_in_hz = FREQ_EU_868_HZ,
    .invert_iq_is_on = false,
};

/* Functions */

int rx_thread(const struct device *radio)
{
    int ret = 0;
    ralf_params_lora_t rx_params = {0};
    struct radio_ctrl_stats stats = {0};
    struct radio_ctrl_msg_stats msg_stats = {0};

    radio_ctrl_flush_rx_queue(radio);

    radio_ctrl_listen(radio, UINT32_MAX);
    uint8_t payload[255] = {0};

    ret = radio_ctrl_receive(radio, payload, sizeof(payload), &msg_stats, UINT32_MAX);
    if (ret > 0) {
        // Successful reception
        printk("Received payload: ");
        for (size_t i = 0; i < ret; i++) {
            printk("%02X ", payload[i]);
        }
        printk("\n");
        printk("RSSI: %d dBm, SNR: %d dB, Signal RSSI: %d dBm\n",
                msg_stats.rssi, msg_stats.snr, msg_stats.signal_rssi);
    } else {
        // Reception error
        printk("Receive error: %d\n", ret);
    }

    return 0;
}

int tx_thread(const struct device *radio)
{
    int ret = 0;
    struct radio_ctrl_stats stats = {0};
    uint8_t payload[] = "hello";

    radio_ctrl_transmit(radio, payload, sizeof(payload));
    radio_ctrl_get_stats(radio, &stats);
    printk("TX Success: %d/%d, TX Timeout: %d/%d\n", stats.tx_success, stats.tx_attempts, stats.tx_timeout, stats.tx_attempts);

    return 0;
}

int main(void) {
    int ret;

    if (!device_is_ready(radio)) {
        printk("Radio not ready\n");
        return 0;
    }

    defaukt_lora_rx_param.mod_params.ldro = ral_compute_lora_ldro(
        defaukt_lora_rx_param.mod_params.sf, defaukt_lora_rx_param.mod_params.bw);
    default_lora_tx_param.mod_params.ldro = ral_compute_lora_ldro(
        default_lora_tx_param.mod_params.sf, default_lora_tx_param.mod_params.bw);

    radio_ctrl_set_config_lora(radio, &defaukt_lora_rx_param, &default_lora_tx_param,
                    &default_lora_cad_param);

    printk("DANP Demo Application\n");

    danp_lo_interface_t ifaceLo;
    danp_config_t config = {
        .local_node = LOCAL_NODE,
        .log_function = danp_log_message_impl,
    };

    danp_lo_init(&ifaceLo, LOCAL_NODE);
    danp_init(&config);
    danp_register_interface(&ifaceLo);

    server_init();
    client_init();

    while (1) {
#if RX_MODE == 1
        rx_thread(radio);
#elif TX_MODE == 1
        tx_thread(radio);
        k_sleep(K_MSEC(5000));
#else
        printk("Please define RX_MODE or TX_MODE\n");
#endif
    }

    return 0;
}
