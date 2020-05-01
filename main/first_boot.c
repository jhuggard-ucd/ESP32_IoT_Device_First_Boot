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

#include "memory.h"
#include "first_boot.h"

#define RESET_GPIO                 GPIO_NUM_39

#define FIRST_BOOT_NAMESPACE "first_boot"

#define VALID_NETWORK_DETAILS_STORED_TAG "valid_network_details_stored"

/* Stages of connection to a WiFi network */
typedef enum {
	SCAN = 0,
	SETUP_APSTA = 1,
	IDENTIFY_NETWORK = 2
} identify_wifi_state_t;

void init() {
	// Reset and erase memory
	gpio_pad_select_gpio(RESET_GPIO);
	gpio_set_direction(RESET_GPIO, GPIO_MODE_INPUT);
	if (gpio_get_level(RESET_GPIO)) {
		ESP_ERROR_CHECK(nvs_flash_erase());
	}
}


bool valid_network_details_stored(bool verbose) {
	// Initialise NVS
	init_memory(FIRST_BOOT_NAMESPACE);

	// Check NVS
	size_t ssid_size = SSID_SIZE;
	esp_err_t ssid_err = read_string(SSID_HANDLE, NULL, &ssid_size);

	size_t pword_size = PWORD_SIZE;
	esp_err_t pword_err = read_string(PWORD_HANDLE, NULL, &pword_size);

	// Check if SSID and Password were obtained from NVS
	if (ssid_err == ESP_OK && pword_err == ESP_OK) {
		ESP_LOGI(VALID_NETWORK_DETAILS_STORED_TAG, "Valid SSID and Password exist");
		deinit_memory();
		return true;
	} else {
		if (verbose) {
			ESP_LOGE(VALID_NETWORK_DETAILS_STORED_TAG, "No valid SSID and/or Password exists");
			ESP_LOGE(VALID_NETWORK_DETAILS_STORED_TAG, "Error getting SSID:     %s", esp_err_to_name(ssid_err));
			ESP_LOGE(VALID_NETWORK_DETAILS_STORED_TAG, "Error getting password: %s", esp_err_to_name(pword_err));
		}
		deinit_memory();
		return false;
	}

}

esp_err_t connect_to_saved_ap() {
	// Initialise NVS
	init_memory(FIRST_BOOT_NAMESPACE);

	// Read NVS
	size_t ssid_size = SSID_SIZE;
	char ssid[SSID_SIZE];
	read_string(SSID_HANDLE, ssid, &ssid_size);

	size_t pword_size = PWORD_SIZE;
	char pword[PWORD_SIZE];
	read_string(PWORD_HANDLE, pword, &pword_size);

	deinit_memory();

	if (connect_to_ap(ssid, pword) == ESP_OK) {
		return ESP_OK;
	} else {
		ESP_LOGE("connect_to_saved_ap", "Could not connect to saved AP");
		return ESP_FAIL;
	}
}

void clear_stored_network_details() {
	nvs_flash_erase_partition(FIRST_BOOT_NAMESPACE);
}

void identifty_network() {

	identify_wifi_state_t state;

	state = SCAN;

	while(1) {
		switch(state) {
		case	SCAN:
			// Initialise WiFi
			init_wifi();          //TODO: move this into other wifi functions?

			// Scan for nearby APs
			scan_aps();

			// Disable wifi station
			//			disable_wifi();

			state = SETUP_APSTA;

			break;
		case	SETUP_APSTA:
			// Set ESP32 up as APSTA
			ap_sta_setup();

			state = IDENTIFY_NETWORK;
			break;
		case	IDENTIFY_NETWORK:
			// Start webserver to allow for user interaction
			captive_portal_init();
			start_webserver();
			return;
		}
	}

}

