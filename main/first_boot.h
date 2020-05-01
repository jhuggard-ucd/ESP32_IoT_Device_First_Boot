/* * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * INTERNET OF THINGS FIRST BOOT APPLICATION SOFTWARE
 *
 * First boot
 * This file contains the main functions used as part
 * of the first-boot software
 *
 * Author:        James Huggard
 * Last Modified: 29/04/2020
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef MAIN_FIRST_BOOT_H_
#define MAIN_FIRST_BOOT_H_

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/gpio.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "wifi.h"
#include "memory.h"
#include "server.h"
#include "captive_portal.h"

/**
 * @brief Opportunity for user to reset device by clearing NVS
 */
void init();

/**
 * @brief check if memory contains valid network details to connect
 * @return true if network details exist. false otherwise.
 */
bool valid_network_details_stored(bool verbose);

/**
 * @brief Connect to ap whose details are stored in NVS
 * @return ESP_OK on successful connection. Error code otherwise.
 */
esp_err_t connect_to_saved_ap();

/**
 * @brief Clear NVS partition with saved network details
 */
void clear_stored_network_details();

/**
 * @brief Allow for user to select network to connect to
 */
void identifty_network();



#endif /* MAIN_FIRST_BOOT_H_ */
