/* * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * INTERNET OF THINGS FIRST BOOT APPLICATION SOFTWARE
 *
 * Example Secondary Application
 * This file is provided as an example of how a developer
 * may write a piece of software that can be used with the
 * IoT First Boot software
 *
 * Author:        James Huggard
 * Last Modified: 28/03/2020
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef MAIN_EXAMPLE_SECONDARY_APP_H_
#define MAIN_EXAMPLE_SECONDARY_APP_H_

#include <math.h>
#include <string.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "esp_system.h"
#include "esp_adc_cal.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_http_server.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "thingspeak.h"
#include "wifi.h"
#include "memory.h"
#include "server.h"
#include "first_boot.h"

/*
 * @brief Take a reading of the current ambient brightness and log result to ThingSpeak
 */
void log_to_thingspeak();

#endif /* MAIN_WIFI_H_ */
