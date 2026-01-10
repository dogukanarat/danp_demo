/* tmtc.c - one line definition */

/* All Rights Reserved */

/* Includes */

#include <string.h>
#include <zephyr/tmtc.h>
#include <zephyr/logging/log.h>
#include <zephyr/logging/log_instance.h>

/* Imports */


/* Definitions */

LOG_MODULE_DECLARE(main);

LOG_INSTANCE_REGISTER(main, tmtc, LOG_LEVEL_INF);

#undef LOG_ERR
#undef LOG_WRN
#undef LOG_INF
#undef LOG_DBG
#undef LOG_VER

#define LOG_ERR(...) LOG_INSTANCE_ERR(&LOG_INSTANCE_GET(main, tmtc), __VA_ARGS__)
#define LOG_WRN(...) LOG_INSTANCE_WRN(&LOG_INSTANCE_GET(main, tmtc), __VA_ARGS__)
#define LOG_INF(...) LOG_INSTANCE_INF(&LOG_INSTANCE_GET(main, tmtc), __VA_ARGS__)
#define LOG_DBG(...) LOG_INSTANCE_DBG(&LOG_INSTANCE_GET(main, tmtc), __VA_ARGS__)
#define LOG_VER(...) LOG_INSTANCE_VER(&LOG_INSTANCE_GET(main, tmtc), __VA_ARGS__)

/* Types */


/* Forward Declarations */

static int32_t main_tmtc_echo (struct tmtc_args *rqst, struct tmtc_args *rply);

/* Variables */

TMTC_DEFINE(tmtc_example_cmd_handler) =
{
    {
        .id = 1,
        .max_data_len = 256,
        .min_data_len = 0,
        .handler = main_tmtc_echo,
    }
};

/* Functions */

static int32_t main_tmtc_echo (struct tmtc_args *rqst, struct tmtc_args *rply)
{
    int32_t ret = 0;
    uint8_t *rqst_data = NULL;
    size_t rqst_data_len = 0;

    for (;;)
    {
        rqst_data = &rqst->data[rqst->hdr_len];
        rqst_data_len = rqst->len - rqst->hdr_len;

        if (0 == rqst_data_len)
        {
            ret = -ENODATA; // No data to echo
            break;
        }

        if (256 < rqst_data_len)
        {
            ret = -EMSGSIZE; // Data too large
            break;
        }

        rply->len = rply->hdr_len + rqst_data_len;
        rply->data = tmtc_malloc(rply, rply->len);
        if (!rply->data) {
            ret = -1;
            break;
        }
        memset(rply->data, 0, rply->len);

        LOG_HEXDUMP_INF(rqst_data, rqst_data_len, "Echo Data");

        memcpy(&rply->data[rply->hdr_len], rqst_data, rqst_data_len);

        break;
    }

    return ret;
}
