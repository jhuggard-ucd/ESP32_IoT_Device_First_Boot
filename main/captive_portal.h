/* * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * INTERNET OF THINGS FIRST BOOT APPLICATION SOFTWARE
 *
 * DNS Captive Portal
 * Use of the DNS Captive Portal will redirect a user's
 * device to the network selection page.
 *
 * Author:        James Huggard
 * Last Modified: 09/04/2020
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef MAIN_CAPTIVE_PORTAL_H_
#define MAIN_CAPTIVE_PORTAL_H_

#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "lwip/sockets.h"
#include "lwip/err.h"
#include "esp_netif.h"
#include "string.h"

#include "wifi.h"

/*
 * @brief Function to initialise a captive portal RTOS task
 */
void captive_portal_init();

#endif /* MAIN_CAPTIVE_PORTAL_H_ */
