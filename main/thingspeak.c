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

#include "thingspeak.h"

static const char* TAG = "ThingSpeak";


#define WEB_SERVER "api.thingspeak.com"

#define THINGSPEAK_WRITE_API_KEY "DUMMY API KEY"

static const char* get_request_start =
		"GET /update?key="
		THINGSPEAK_WRITE_API_KEY;

static const char* get_request_end =
		" HTTP/1.1\n"
		"Host: "WEB_SERVER"\n"
		"Connection: close\n"
		"User-Agent: esp32 / esp-idf\n"
		"\n";

static http_client_data http_client = {0};

/* Collect chunks of data received from server
   into complete message and save it in proc_buf
 */
static void process_chunk(uint32_t *args)
{
	http_client_data* client = (http_client_data*)args;

	int proc_buf_new_size = client->proc_buf_size + client->recv_buf_size;
	char *copy_from;

	if (client->proc_buf == NULL){
		client->proc_buf = malloc(proc_buf_new_size);
		copy_from = client->proc_buf;
	} else {
		proc_buf_new_size -= 1; // chunks of data are '\0' terminated
		client->proc_buf = realloc(client->proc_buf, proc_buf_new_size);
		copy_from = client->proc_buf + proc_buf_new_size - client->recv_buf_size;
	}
	if (client->proc_buf == NULL) {
		ESP_LOGE(TAG, "Failed to allocate memory");
	}
	client->proc_buf_size = proc_buf_new_size;
	memcpy(copy_from, client->recv_buf, client->recv_buf_size);
}

static void disconnected(uint32_t *args)
{
	http_client_data* client = (http_client_data*)args;

	free(client->proc_buf);
	client->proc_buf = NULL;
	client->proc_buf_size = 0;

	ESP_LOGD(TAG, "Free heap %u", xPortGetFreeHeapSize());
}

esp_err_t thingspeak_post_data(float percent, int8_t rssi)
{
	int n;

	// 1
	n = snprintf(NULL, 0, "%.2f", percent);
	char field1[n+1];
	sprintf(field1, "%.2f", percent);

	// 2
	n = snprintf(NULL, 0, "%d", rssi);
	char field2[n+1];
	sprintf(field2, "%d", rssi);



	// request string size calculation
	int string_size = strlen(get_request_start);
	string_size += strlen("&fieldN=");  // number of fields
	string_size += strlen(field1);
	string_size += strlen("&fieldN=");  // number of fields
	string_size += strlen(field2);
	string_size += strlen(get_request_end);
	string_size += 1;  // '\0' - space for string termination character

	// request string assembly / concatenation
	char * get_request = malloc(string_size);
	strcpy(get_request, get_request_start);
	strcat(get_request, "&field1=");
	strcat(get_request, field1);
	strcat(get_request, "&field2=");
	strcat(get_request, field2);
	strcat(get_request, get_request_end);

	// ToDo: REMOVE
	// print get request
	// for diagnostic purposes
	//
	// printf("%d, %s\n", string_size, get_request);

	ESP_LOGI("Request", "%s", get_request);

	esp_err_t err = http_client_request(&http_client, WEB_SERVER, get_request);

	free(get_request);
	return err;
}

void thingspeak_initialise()
{
	http_client_on_process_chunk(&http_client, process_chunk);
	http_client_on_disconnected(&http_client, disconnected);
}
