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

#ifndef MAIN_MEMORY_H_
#define MAIN_MEMORY_H_

#include "esp_err.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#define SSID_HANDLE "ssid_handle"
#define PWORD_HANDLE "pword_handle"

/**
 * @brief Open memory for a specified namespace.
 * @param namespace Namespace to be opened
 * @return ESP_OK on success
 */
esp_err_t init_memory(char *namespace);

/**
 * @brief Close open memory.
 * @return ESP_OK on success
 */
esp_err_t deinit_memory();

/**
 * @brief Get length of a string stored in memory.
 * @param key Key for string required
 * @param size Output size of string
 * @return ESP_OK on success
 */
esp_err_t get_required_size(char *key, size_t *required_size);

/**
 * @brief Read a string from memory.
 * @param key Key for string required
 * @param string Output string requested
 * @param size Size of string
 * @return ESP_OK on success
 */
esp_err_t read_string(char *key, char *string, size_t *required_size);

/**
 * @brief Write a string to memory.
 * @param key Key for string
 * @param string String to be written to memory
 * @return ESP_OK on success
 */
esp_err_t write_string(char *key, char *string);

/**
 * @brief Read a 16-bit unsigned int from memory.
 * @param key Key for uint16_t
 * @param value uint16_t to be read
 * @return ESP_OK on success
 */
esp_err_t read_uint16(char *key, uint16_t *value);

/**
 * @brief Write a 16-bit unsigned int to memory.
 * @param key Key for uint16_t
 * @param value uint16_t to be written
 * @return ESP_OK on success
 */
esp_err_t write_uint16(char *key, uint16_t value);

/**
 * @brief Read an 8-bit unsigned int from memory.
 * @param key Key for uint8_t
 * @param value uint8_t to be read
 * @return ESP_OK on success
 */
esp_err_t read_uint8(char *key, uint8_t *value);

/**
 * @brief Write an 8-bit unsigned int to memory.
 * @param key Key for uint8_t
 * @param value uint8_t to be written
 * @return ESP_OK on success
 */
esp_err_t write_uint8(char *key, uint8_t value);



#endif /* MAIN_MEMORY_H_ */
