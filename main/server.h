/* * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * INTERNET OF THINGS FIRST BOOT APPLICATION SOFTWARE
 *
 * HTTP Server
 * Functions that make up the HTTP Local server run as
 * part of the first-boot process
 *
 * Author:        James Huggard
 * Last Modified: 15/03/2020
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef MAIN_SERVER_H_
#define MAIN_SERVER_H_

#include "esp_err.h"
#include "esp_netif.h"
#include <string.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "esp_http_client.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "first_boot.h"
#include "memory.h"
#include "wifi.h"

#define SSID_SIZE 33
#define PWORD_SIZE 64

/**
 * @brief Start HTTP webserver
 * @return Handle of webserver
 */
httpd_handle_t start_webserver(void);

/**
 * @brief Stop HTTP webserver
 * @param server Webserver handle
 */
void stop_webserver(httpd_handle_t server);

/**
 * @breif Returns whether user has been informed of positive connection
 */
int get_user_informed();

#endif /* MAIN_SERVER_H_ */
