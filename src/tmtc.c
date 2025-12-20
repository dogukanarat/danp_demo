/* tmtc.c - one line definition */

/* All Rights Reserved */

/* Includes */

#include <string.h>
#include <zephyr/tmtc.h>

/* Imports */


/* Definitions */


/* Types */


/* Forward Declarations */

static int32_t main_tmtc_test (struct tmtc_args *rqst, struct tmtc_args *rply);

/* Variables */

TMTC_DEFINE(tmtc_example_cmd_handler) =
{
    {
        .id = 1,
        .max_data_len = 256,
        .min_data_len = 0,
        .handler = main_tmtc_test,
    }
};

/* Functions */

static int32_t main_tmtc_test (struct tmtc_args *rqst, struct tmtc_args *rply)
{
    int32_t ret = 0;

    for (;;)
    {
        rply->len = rply->hdr_len + rqst->len;
        rply->data = tmtc_malloc(rply->len);
        if (!rply->data) {
            ret = -1;
            break;
        }

        memcpy((uint8_t *)rply->data + rply->hdr_len, rqst->data, rqst->len);

        break;
    }

    return ret;
}
