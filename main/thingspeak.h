/* * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * INTERNET OF THINGS FIRST BOOT APPLICATION SOFTWARE
 *
 * ThingSpeak
 * Functions to handle logging of data to ThingSpeak
 *
 * File based on https://github.com/krzychb/esp32-everest-run
 *
 * Author:        James Huggard
 * Last Modified: 20/03/2020
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef THINGSPEAK_H
#define THINGSPEAK_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_err.h"
#include "http.h"

#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ESP_ERR_THINGSPEAK_BASE 0x60000
#define ESP_ERR_THINGSPEAK_POST_FAILED          (ESP_ERR_THINGSPEAK_BASE + 1)

esp_err_t thingspeak_post_data(float percent, int8_t rssi);
void thingspeak_initialise();

#ifdef __cplusplus
}
#endif

#endif  // THINGSPEAK_H
