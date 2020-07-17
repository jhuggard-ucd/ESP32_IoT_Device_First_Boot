/* * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * INTERNET OF THINGS FIRST BOOT APPLICATION SOFTWARE
 *
 * Memory
 * Functions that handle reading/writing memory.
 *
 * Author:        James Huggard
 * Last Modified: 12/02/2020
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "memory.h"

nvs_handle_t MEMORY_HANDLE;

esp_err_t init_memory(char *namespace) {
	//ESP_ERROR_CHECK(nvs_flash_erase());

	ESP_LOGI("INIT MEMORY", "Memory initialised");

	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	ESP_ERROR_CHECK(nvs_open(namespace, NVS_READWRITE, &MEMORY_HANDLE));

	return ESP_OK;
}

esp_err_t deinit_memory() {
	nvs_close(MEMORY_HANDLE);
	ESP_LOGI("INIT MEMORY", "Memory deinitialised");
	return ESP_OK;
}

void handle_err(char *tag, esp_err_t err) {
	if (err == ESP_ERR_NVS_NOT_FOUND) {
		ESP_LOGE(tag, "Requested key does not exist");
	} else if (err == ESP_ERR_NVS_INVALID_HANDLE) {
		ESP_LOGE(tag, "Requested handle may have been closed, or is NULL");
	} else if (err == ESP_ERR_NVS_INVALID_NAME) {
		ESP_LOGE(tag, "Requested key does not satisfy constraints");
	} else if (err == ESP_ERR_NVS_INVALID_LENGTH) {
		ESP_LOGE(tag, "String or blob length is not sufficient to store data");
	}
}

esp_err_t get_required_size(char *key, size_t *required_size) {
	esp_err_t err = nvs_get_str(MEMORY_HANDLE, key, NULL, required_size);

	if (err != ESP_OK) handle_err("get_required_size", err);

	return err;
}

esp_err_t read_string(char *key, char *string, size_t *required_size) {

	esp_err_t err = nvs_get_str(MEMORY_HANDLE, key, string, required_size);

	if (string != NULL) {
		if (err != ESP_OK) handle_err("read_string", err);
		else ESP_LOGI("read_string", "Got value for %s: %s", key, string);
	}

	return err;
}

esp_err_t write_string(char *key, char *string) {

	esp_err_t err = nvs_set_str(MEMORY_HANDLE, key, string);

	if (err != ESP_OK) handle_err("write_string", err);
	else ESP_LOGI("write_string", "Wrote value to %s: %s", key, string);

	nvs_commit(MEMORY_HANDLE);

	return err;
}

esp_err_t read_uint16(char *key, uint16_t *value) {
	esp_err_t err = nvs_get_u16(MEMORY_HANDLE, key, value);

	if (err != ESP_OK) handle_err("read_uint16", err);
	else ESP_LOGI("read_uint16", "Got value for %s: %d", key, value[0]);

	return err;
}

esp_err_t write_uint16(char *key, uint16_t value) {
	esp_err_t err = nvs_set_u16(MEMORY_HANDLE, key, value);

	if (err != ESP_OK) handle_err("write_uint16", err);
	else ESP_LOGI("write_uint16", "Wrote value for %s: %u", key, value);

	return err;
}

esp_err_t read_uint8(char *key, uint8_t *value) {
	esp_err_t err = nvs_get_u8(MEMORY_HANDLE, key, value);

	if (err != ESP_OK) handle_err("read_uint8", err);
	else ESP_LOGI("read_uint8", "Got value for %s: %s", key, value);

	return err;
}

esp_err_t write_uint8(char *key, uint8_t value) {
	esp_err_t err = nvs_set_u8(MEMORY_HANDLE, key, value);

	if (err != ESP_OK) handle_err("write_uint8", err);
	else ESP_LOGI("write_uint8", "Wrote value for %s: %d", key, value);

	return err;
}

void clear_namespace() {
	esp_err_t err = nvs_erase_all(MEMORY_HANDLE);
	ESP_LOGI("clear_namespace", "Partition erased with error code: %s", esp_err_to_name(err));
}
