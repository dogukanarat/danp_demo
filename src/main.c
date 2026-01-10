/* main.c - one line definition */

/* All Rights Reserved */

/* Includes */

#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/radio_ctrl.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/printk.h>

#include "cfl/services/cfl_service_danp.h"
#include "danp/drivers/danp_z_radio.h"
#include "danp/danp.h"
#include "danp/danp_log.h"
#include "danp/services/danp_ftp_service.h"

#include "common_definitions.h"

/* Imports */


/* Definitions */

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)

#define FREQ_EU_868_HZ (868300000)
#define SYNC_WORD_PUBLIC (0x34)
#define SYNC_WORD_PRIVATE (0x12)
#define SYNC_WORD_NO_RADIO (0x21)

/* Types */


/* Forward Declarations */


/* Variables */

static const char route_table[] = "10:radio0, 20:radio0";
static const struct device *const radio = DEVICE_DT_GET(DT_ALIAS(radio0));

static ralf_params_lora_t default_lora_rx_param = {
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

static danp_radio_interface_t iface_radio;

/* Functions */

danp_ftp_status_t ftp_fs_open(
    danp_ftp_file_handle_t *file_handle,
    const uint8_t *file_id,
    size_t file_id_len,
    danp_ftp_service_fs_mode_t mode,
    void *user_data)
{
    LOG_INF("FTP FS Open called for file ID: %.*s", file_id_len, file_id);
    return DANP_FTP_STATUS_OK;
}

danp_ftp_status_t ftp_fs_close(
    danp_ftp_file_handle_t file_handle,
    void *user_data)
{
    LOG_INF("FTP FS Close called");
    return DANP_FTP_STATUS_OK;
}

danp_ftp_status_t ftp_fs_read(
    danp_ftp_file_handle_t file_handle,
    size_t offset,
    uint8_t *buffer,
    uint16_t length,
    void *user_data)
{
    LOG_INF("FTP FS Read called: offset=%u, length=%u", offset, length);
    return DANP_FTP_STATUS_OK;
}

danp_ftp_status_t ftp_fs_write(
    danp_ftp_file_handle_t file_handle,
    size_t offset,
    const uint8_t *data,
    uint16_t length,
    void *user_data)
{
    LOG_INF("FTP FS Write called: offset=%u, length=%u", offset, length);
    return DANP_FTP_STATUS_OK;
}

int main(void) {
    int ret = 0;

    if (!device_is_ready(radio)) {
        LOG_ERR("Radio not ready\n");
        return 0;
    }

    default_lora_rx_param.mod_params.ldro = ral_compute_lora_ldro(
        default_lora_rx_param.mod_params.sf, default_lora_rx_param.mod_params.bw);
    default_lora_tx_param.mod_params.ldro = ral_compute_lora_ldro(
        default_lora_tx_param.mod_params.sf, default_lora_tx_param.mod_params.bw);

    ret = danp_radio_init(
        &iface_radio,
        "radio0",
        radio,
        &default_lora_rx_param,
        &default_lora_tx_param,
        &default_lora_cad_param,
        OWN_NODE_ID);
    if (ret < 0)
    {
        LOG_ERR("Failed to initialize DANP radio interface: %d\n", ret);
        return 0;
    }
    danp_register_interface((danp_interface_t *)&iface_radio);

    if (danp_route_table_load(route_table) != 0)
    {
        LOG_ERR("Failed to install route '%s'", route_table);
    }
    else
    {
        LOG_INF("Installed static route: %s", route_table);
    }

    danp_config_t danp_config = {
        .local_node = OWN_NODE_ID,
        .log_function = danp_log_message_impl,
        .log_function_io = danp_log_message_io_impl,
    };
    danp_init(&danp_config);

    danp_ftp_service_config_t danp_ftp_service_config = {
        .fs.open = ftp_fs_open,
        .fs.close = ftp_fs_close,
        .fs.read = ftp_fs_read,
        .fs.write = ftp_fs_write,
        .user_data = NULL,
    };
    ret = danp_ftp_service_init(&danp_ftp_service_config);
    if (ret < 0)
    {
        LOG_ERR("Failed to initialize DANP FTP service: %d\n", ret);
        return 0;
    }

    cfl_service_danp_config_t config = {
        .port_id = CONFIG_CFL_SUPPORT_DANP_SERVICE_PORT,
    };

    ret = cfl_service_danp_init(&config);
    if (ret != 0) {
        LOG_ERR("Failed to initialize CFL DANP service: %d\n", ret);
        return 0;
    }

    while (1)
    {
        k_sleep(K_MSEC(1000));
    }

    return 0;
}
