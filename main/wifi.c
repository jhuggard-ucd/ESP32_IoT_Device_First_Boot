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

#include "wifi.h"

// Debug Tags
#define AP_STA_SETUP_TAG "ap_sta_setup"
#define SCAN_APS_TAG "scan_aps"
#define CONNECT_TO_AP_TAG "connect_to_ap"

#define MAC_LEN 6

/* FreeRTOS event group to signal when device is connected */
static EventGroupHandle_t s_wifi_event_group;

/* Bits to handle individual events */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1
#define STA_MODE_BIT       BIT2
#define AP_MODE_BIT        BIT3

/* Number of unsuccessful reconnect attempts before stop */
#define WIFI_MAXIMUM_RETRY 1

/* Temporary AP Details */
#define ESP_WIFI_SSID      "ESP_WIFI"
#define ESP_WIFI_PASS      "password"
#define MAX_STA_CONN       4

static int s_retry_num = 0;
int connected = 0;

ap_details_t *ap_details;
uint16_t apCount;

esp_netif_t *ap_netif;
esp_netif_t *sta_netif;

/*
 * Check if a particular ssid exists in list up to the current index
 * This is to prevent duplicate SSIDs occuring in the list
 */
int ssid_exists(char *ssid, ap_details_t *list, int index) {
	if (index == 0) {
		ESP_LOGI("Check SSID Exists", "List is empty - SSID does not exist");
		return 0;
	}
	for (int i = 0; i < index; i++) {
		if (strcmp(ssid, list[i].ssid) == 0) {
			ESP_LOGI("Check SSID Exists", "List contains SSID %s", ssid);
			return 1;
		}
	}
	return 0;
}

/*
 * Handler for WiFi events in STA and AP modes
 */
static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
		xEventGroupSetBits(s_wifi_event_group, STA_MODE_BIT);
	else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_STOP)
		xEventGroupClearBits(s_wifi_event_group, STA_MODE_BIT);
	else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_START)
		xEventGroupSetBits(s_wifi_event_group, AP_MODE_BIT);
	else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STOP)
		xEventGroupClearBits(s_wifi_event_group, AP_MODE_BIT);
	else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_SCAN_DONE) {
		ESP_LOGI("WiFi Scan Complete", "Found %d APs", ((wifi_event_sta_scan_done_t *) event_data)->number);

		apCount = ((wifi_event_sta_scan_done_t *) event_data)->number;
		if (apCount == 0) {
			return;
		}

		wifi_ap_record_t *list = (wifi_ap_record_t *) malloc(
				sizeof(wifi_ap_record_t) * apCount);
		ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&apCount, list));

		ap_details = (ap_details_t *) malloc (sizeof(ap_details_t) * apCount);

		int index = 0;

		for (int i = 0; i < apCount; i++) {
			if (strlen((char *)list[i].ssid) == 0) {
				ESP_LOGI("Scan Complete", "Not including SSID as length is 0");
				continue;
			} else if (ssid_exists((char *)list[i].ssid, ap_details, index)) {
				ESP_LOGI("Scan Complete", "Not including SSID %s as it is already included in list", list[i].ssid);
				continue;
			}

			memcpy(ap_details[index].ssid, list[i].ssid, sizeof(list[i].ssid)/sizeof(char));
			ESP_LOGI("Scan Complete", "%s added to list", (char *)ap_details[index].ssid);
			ap_details[index].authmode = list[i].authmode;
			index++;
		}
		apCount = index;
		free(list);
	}
	else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
		if (s_retry_num < WIFI_MAXIMUM_RETRY) {
			esp_wifi_connect();
			ESP_LOGI("Event Handler", "Retry to connect to the AP: attempt %d", s_retry_num);
			s_retry_num++;
			ESP_LOGI("Event Handler", "retry to connect to the AP");
		} else {
			ESP_LOGI("Event Handler", "Maximum retry reached. Fail bit set.");
			xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
			connected = 0;
		}
		ESP_LOGI("Event Handler","connect to the AP fail");
	} else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
		ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
		ESP_LOGI("Event Handler", "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
		s_retry_num = 0;
		connected = 1;
		xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
	} else if (event_id == WIFI_EVENT_AP_STACONNECTED) {
		wifi_event_ap_staconnected_t* event =
				(wifi_event_ap_staconnected_t*) event_data;
		ESP_LOGI("WiFi Event Handler", "station "MACSTR" join, AID=%d", MAC2STR(event->mac),
				event->aid);
	} else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
		wifi_event_ap_stadisconnected_t* event =
				(wifi_event_ap_stadisconnected_t*) event_data;
		ESP_LOGI("WiFi Event Handler", "station "MACSTR" leave, AID=%d", MAC2STR(event->mac),
				event->aid);
	}
}

esp_err_t init_wifi() {
	s_wifi_event_group = xEventGroupCreate();

	ESP_ERROR_CHECK(esp_netif_init());

	ESP_ERROR_CHECK(esp_event_loop_create_default());

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	ESP_LOGI("init_wifi", "ESP wifi initialised");

	return ESP_OK;
}

esp_err_t deinit_wifi() {
	return esp_netif_deinit();
}

esp_err_t ap_sta_setup() {
	unsigned char mac[MAC_LEN];
	char ssid[2*MAC_LEN + strlen(ESP_WIFI_SSID) + 7];
	char *esp_ssid = ESP_WIFI_SSID;

	ESP_ERROR_CHECK(esp_efuse_mac_get_default(mac));

	printf("Using \"0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\" as MAC address\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

	ap_netif = esp_netif_create_default_wifi_ap();

	sprintf(ssid, "%s-%02x:%02x:%02x:%02x:%02x:%02x", esp_ssid, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

	// Set AP configuration
	wifi_config_t wifi_ap_config = {
			.ap = {
					.ssid = "",
					.ssid_len = strlen(ssid),
					.password = ESP_WIFI_PASS,
					.max_connection = MAX_STA_CONN,
					.authmode = WIFI_AUTH_WPA_WPA2_PSK
			}
	};
	if (strlen(ESP_WIFI_PASS) == 0) {
		wifi_ap_config.ap.authmode = WIFI_AUTH_OPEN;
	}

	strcpy((char*)wifi_ap_config.ap.ssid, ssid);

	wifi_config_t wifi_sta_config = {
			.sta = {
					.ssid = "",
					.password = ""
			}
	};

	ESP_LOGI(AP_STA_SETUP_TAG, "Set WiFi AP Config");

	ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_ap_config));
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_sta_config));
	ESP_ERROR_CHECK(esp_wifi_start());

	ESP_LOGI(AP_STA_SETUP_TAG, "Started WiFi as AP. SSID:%s password:%s",
			ssid, ESP_WIFI_PASS);                 //TODO: Remove this?

//	// Initialise mDNS
//	esp_err_t err = mdns_init();
//	if (err) {
//		ESP_LOGI(AP_STA_SETUP_TAG, "Failed to init mdns: %d", err);
//	}
//
//	mdns_hostname_set("esp32");
//	mdns_instance_name_set("first boot of esp32");
//
//	mdns_service_add(NULL, "_http", "_tcp", 80, NULL, 0);

	return ESP_OK;
}

esp_err_t disable_wifi() {
	ESP_ERROR_CHECK(esp_wifi_stop());
	ESP_LOGI("disable_wifi", "Complete");

	return ESP_OK;
}

esp_err_t scan_aps() {
	sta_netif = esp_netif_create_default_wifi_sta();

	ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
	ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

	ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
	ESP_ERROR_CHECK(esp_wifi_start());

	ESP_LOGI(SCAN_APS_TAG, "ESP configured as wifi station/AP");

	wifi_scan_config_t scanConf = {
			.ssid = NULL,
			.bssid = NULL,
			.channel = 0,
			.show_hidden = true
	};
	ESP_ERROR_CHECK(esp_wifi_scan_start(&scanConf, true));

	ESP_LOGI(SCAN_APS_TAG, "Finished Scan");

	return ESP_OK;
}

int is_sta_connected() {
	ESP_LOGI("is_sta_connected", "%d", connected);
	return connected;
}

esp_err_t connect_to_ap(char *ssid, char *pword) {
	wifi_mode_t wifi_mode;
	esp_err_t wifi_err = esp_wifi_get_mode(&wifi_mode);

	esp_err_t connect_err = ESP_FAIL;

	if (wifi_err == ESP_ERR_WIFI_NOT_INIT) {
		// Initialise WiFi
		init_wifi();          //TODO: move this into other wifi functions?
		wifi_config_t wifi_config = {
				.sta = {
						.ssid = "",		//Must initialise ssid and password as empty strings
						.password = ""  //in order to strcpy later
				}
		};

		sta_netif = esp_netif_create_default_wifi_sta();

		ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
		ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

		strcpy((char*)wifi_config.sta.ssid, ssid);
		strcpy((char*)wifi_config.sta.password, pword);

		ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
		ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
		ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

		ESP_ERROR_CHECK(esp_wifi_start());

		connect_err = esp_wifi_connect();

	} else if(wifi_mode == WIFI_MODE_APSTA) {
		wifi_config_t wifi_config;

		ESP_ERROR_CHECK(esp_wifi_get_config(WIFI_IF_STA, &wifi_config));

		strcpy((char*)wifi_config.sta.ssid, ssid);
		strcpy((char*)wifi_config.sta.password, pword);

		ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

		connect_err = esp_wifi_connect();
	}
	ESP_LOGI(CONNECT_TO_AP_TAG, "Set up connection to wifi: ssid: %s, pword: %s", ssid, pword);


	ESP_LOGI(CONNECT_TO_AP_TAG, "Set up connection to wifi with error code %s", esp_err_to_name(connect_err));

	ESP_LOGI(CONNECT_TO_AP_TAG, "Waiting for connection to be made");
	xEventGroupWaitBits(
			s_wifi_event_group,
			WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
			pdFALSE,
			pdFALSE,
			portMAX_DELAY);
	ESP_LOGI(CONNECT_TO_AP_TAG, "Connection or fail made");

	if (is_sta_connected()) {
		return ESP_OK;
	} else {
		return ESP_FAIL;
	}
}

ap_details_t get_ap_details(int index) { //TODO: Error check for scan incomplete
	return ap_details[index];
}

int get_ap_count() {   //TODO: Error check for scan incomplete
	return apCount;
}

esp_netif_t *get_sta_netif() {
	return sta_netif;
}

esp_netif_t *get_ap_netif() {
	return ap_netif;
}

uint32_t get_ap_ip_address() {

	esp_netif_ip_info_t ip_info;

	esp_netif_get_ip_info(ap_netif, &ip_info);

	return ip_info.ip.addr;
}

int get_connection_status() {
	return connected;
}
