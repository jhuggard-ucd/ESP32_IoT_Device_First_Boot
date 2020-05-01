/* * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * INTERNET OF THINGS FIRST BOOT APPLICATION SOFTWARE
 *
 * WiFi
 * Functions to control WiFi interface of ESP32
 *
 * Author:        James Huggard
 * Last Modified: 25/02/2020
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef MAIN_WIFI_H_
#define MAIN_WIFI_H_

#include "esp_netif.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_wifi_types.h"
#include "esp_event.h"
#include "esp_log.h"
#include "mdns.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "memory.h"

/** Struct to hold info about scanned APs */
typedef struct {
	char ssid[33];
	int authmode;
} ap_details_t;

/**
 * @brief Initialise ESP32 wifi.
 * @return ESP_OK on wifi successfully initialised
 */
esp_err_t init_wifi();

/**
 * @brief De-initialise ESP32 wifi.
 * @return ESP_OK on wifi successfully de-initialised
 */
esp_err_t deinit_wifi();

/**
 * @brief Setup ESP as wifi access point.
 * @return ESP_OK on AP successfully set up
 */
esp_err_t ap_sta_setup();

/**
 * @brief Disable wifi. Must be performed when switching between AP and STA wifi modes.
 * @return ESP_OK on wifi disabled
 */
esp_err_t disable_wifi();

/**
 * @brief Scan for nearby APs.
 * @return ESP_OK on scan complete and wifi disabled.
 */
esp_err_t scan_aps();

/**
 * @brief Connect to a specified AP.
 * @param ssid SSID of AP to connect to
 * @param pword Password of AP to connect to
 * @return ESP_OK on successfully connected to AP
 */
esp_err_t connect_to_ap(char *ssid, char *pword);

/**
 * @brief Obtain list of aps scanned by ESP32
 * @return ap_details_t object for specified index.
 */
ap_details_t get_ap_details(int index);

/**
 * @brief Get number of aps scanned by ESP32
 * @return number of APs found in scan.
 */
int get_ap_count();

/**
 * @brief Get connection status of WiFi station
 * @return 1 if connected, 0 otherwise
 */
int is_sta_connected();

/**
 * @brief Get network interface of esp_ap
 * @return Handle to network interface
 */
esp_netif_t *get_ap_netif();

/**
 * @brief Get network interface of esp_sta
 * @return Handle to network interface
 */
esp_netif_t *get_sta_netif();

uint32_t get_ap_ip_address();

#endif /* MAIN_WIFI_H_ */
