/* main.c - one line definition */

/* All Rights Reserved */

/* Includes */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include "danp/drivers/danpLo.h"
#include "danp/danp.h"
#include "common_definitions.h"
#include "utilities.h"
#include "server.h"
#include "client.h"


/* Imports */


/* Definitions */

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)

/* Types */


/* Forward Declarations */


/* Variables */

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

/* Functions */

int main(void)
{
    k_msleep(1000); // Sleep for 1 second
    
    int ret;

    if (!gpio_is_ready_dt(&led)) {
        return 0;
    }

    ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
    if (ret < 0) {
        return 0;
    }

    printk("DANP Demo Application\n");

    danpLoInterface_t ifaceLo;
    danpConfig_t config = {
        .localNode = LOCAL_NODE,
        .logFunction = logMessage,
    };

    danpLoInit(&ifaceLo, LOCAL_NODE);
    danpInit(&config);
    danpRegisterInterface(&ifaceLo);

    server_init();
    // client_init();

    while (1) 
    {
        ret = gpio_pin_toggle_dt(&led);
        k_msleep(1000); // Sleep for 1 second
    }
    return 0;
}
