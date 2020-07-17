/* * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * INTERNET OF THINGS FIRST BOOT APPLICATION SOFTWARE
 *
 * This software is written to provide a first-boot
 * procedure for the ESP32, allowing the device to be
 * connected to a WiFi network without the need for a
 * physical interface.
 *
 * To use, a program may be written for the ESP32,
 * the main function of which should be pointed to by
 * the function pointer pt2secondaryAPP.
 *
 * Author:        James Huggard
 * Last Modified: 30/04/2020
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include "sdkconfig.h"
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
#include "lwip/api.h"

#include "thingspeak.h"
#include "wifi.h"
#include "memory.h"
#include "server.h"
#include "first_boot.h"
#include "captive_portal.h"

#include "example_secondary_app.h"

#define FIRST_BOOT_NAMESPACE "first_boot"

/* Program runs as a state machine with the below functions */
typedef enum {
	INIT = 0,
	CONNECT_AS_STA = 1,
	IDENTIFY_NET = 2,
	WAIT_FOR_DETAILS = 3,
	RESTART_DEVICE = 4,
	LAUNCH_APP = 5
} top_level_state_t;

typedef	void (*app_func_pt_t)(void);

/********* USER CONFIGURES THIS *********/
app_func_pt_t     pt2secondaryAPP = log_to_thingspeak;

/* The function app_main is called by ESP32 on boot */
void app_main(void) {
	top_level_state_t state;

	state = INIT;

	while(1) {

		switch(state) {
		/* Gives option to erase ESP32 memory on boot.
		 * If valid network details are stored from previous boot, attempt to connect.
		 * Otherwise, start first boot application in IDENTIFY_NET state */
		case	INIT:
			ESP_LOGI("STATE", "INIT");
			init();
			init_memory(FIRST_BOOT_NAMESPACE);
			if ( valid_network_details_stored(true) == true) {
				state = CONNECT_AS_STA;
			} else {
				state = IDENTIFY_NET;
			}
			break;
			/* Attempt to connect to the last WiFi network used. If unsuccessful, start
			 * first boot application in IDENTIFY_NET state */
		case	CONNECT_AS_STA:
			ESP_LOGI("STATE", "CONNECT_AS_STA");
			if ( connect_to_saved_ap() == ESP_OK) {
				state = LAUNCH_APP;
			}
			else {
				ESP_LOGE("Connection to STA", "Connection failed. Erasing stored network details.");
				clear_namespace();  // Clear incorrect network details
				state = IDENTIFY_NET;
				break;
			}
			ESP_LOGI("Connection to STA", "Passed through 'if' statement");
			break;
			/* Run first boot application to identify network to connect.
			 * Waits in loop, checking every 100ms if network has been selected. */
		case	IDENTIFY_NET:
			ESP_LOGI("STATE", "IDENTIFY_NET");
			identifty_network();
			state = WAIT_FOR_DETAILS;
			break;
			/* Wait for user to input details of network to be connected to. */
		case    WAIT_FOR_DETAILS:
			ESP_LOGI("STATE", "WAIT_FOR_DETAILS");
			while (valid_network_details_stored(false) == false) {
				vTaskDelay(100/portTICK_PERIOD_MS);
			}
			state = RESTART_DEVICE;
			break;
		case    RESTART_DEVICE:
			ESP_LOGI("STATE", "RESTART_DEVICE");
			esp_restart();
			break;
			/* Launch secondary application. If this returns, it will be assumed that there was an
			 * error and the ESP32 will restart. */
		case	LAUNCH_APP:
			ESP_LOGI("STATE", "LAUNCH_APP");
			deinit_memory();

			(*pt2secondaryAPP)();
			// should never get here!
			esp_restart();
			break;
		}
	}
}
