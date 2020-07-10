/* * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * INTERNET OF THINGS FIRST BOOT APPLICATION SOFTWARE
 *
 * HTTP Server
 * Functions that make up the HTTP Local server run as
 * part of the first-boot process
 *
 * Author:        James Huggard
 * Last Modified: 15/03/2020
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "server.h"

esp_err_t _http_event_handle(esp_http_client_event_t *evt)
{
	switch(evt->event_id) {
	case HTTP_EVENT_ERROR:
		ESP_LOGI("HTTP Handler", "HTTP_EVENT_ERROR");
		break;
	case HTTP_EVENT_ON_CONNECTED:
		ESP_LOGI("HTTP Handler", "HTTP_EVENT_ON_CONNECTED");
		break;
	case HTTP_EVENT_HEADER_SENT:
		ESP_LOGI("HTTP Handler", "HTTP_EVENT_HEADER_SENT");
		break;
	case HTTP_EVENT_ON_HEADER:
		ESP_LOGI("HTTP Handler", "HTTP_EVENT_ON_HEADER");
		printf("%.*s", evt->data_len, (char*)evt->data);
		break;
	case HTTP_EVENT_ON_DATA:
		ESP_LOGI("HTTP Handler", "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
		if (!esp_http_client_is_chunked_response(evt->client)) {
			printf("%.*s", evt->data_len, (char*)evt->data);
		}

		break;
	case HTTP_EVENT_ON_FINISH:
		ESP_LOGI("HTTP Handler", "HTTP_EVENT_ON_FINISH");
		break;
	case HTTP_EVENT_DISCONNECTED:
		ESP_LOGI("HTTP Handler", "HTTP_EVENT_DISCONNECTED");
		break;
	}
	return ESP_OK;
}

/* Our URI handler function to be called during GET /uri request */
esp_err_t network_selection_get_handler(httpd_req_t *req) {
	ESP_LOGI("Get Handler", "Entered get handler");
	/* Get handle to embedded file upload script */
	extern const unsigned char network_select_start[] asm("_binary_network_select_html_start");
	extern const unsigned char network_select_end[]   asm("_binary_network_select_html_end");
	const size_t network_select_size = (network_select_end - network_select_start);

	/* Add file upload form and script which on execution sends a POST request to /upload */
	httpd_resp_send_chunk(req, (const char *)network_select_start, network_select_size);
	httpd_resp_send_chunk(req, "", 0);
	ESP_LOGI("Get Handler", "Sent Response");
	return ESP_OK;
}

esp_err_t network_selection_post_handler(httpd_req_t *req) {
	ESP_LOGI("Post Handler", "Entered Network Selection Post Handler");

	/* Destination buffer for content of HTTP POST request.
	 * httpd_req_recv() accepts char* only, but content could
	 * as well be any binary data (needs type casting).
	 * In case of string data, null termination will be absent, and
	 * content length would give length of string */
	char content[1000];

	/* Truncate if content length larger than the buffer */
	size_t recv_size = fmin(req->content_len, sizeof(content));
	ESP_LOGI("Debug", "%d", req->content_len);

	int ret = httpd_req_recv(req, content, recv_size);
	if (ret <= 0) { /* 0 return value indicates connection closed */
		/* Check if timeout occurred */
		if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
			/* In case of timeout one can choose to retry calling
			 * httpd_req_recv(), but to keep it simple, here we
			 * respond with an HTTP 408 (Request Timeout) error */
			httpd_resp_send_408(req);
		}
		/* In case of error, returning ESP_FAIL will
		 * ensure that the underlying socket is closed */
		return ESP_FAIL;
	}

	content[req->content_len] = '\0';
	ESP_LOGI("debug", "Received info: %s", content);

	if (strstr(content, "network-selection") == &content[0]) {
		int apCount = get_ap_count();
		if (apCount == 0) {
			//TODO: NO APs
			ESP_LOGI("Post Handler", "No APs");
			return ESP_OK;
		}
		ESP_LOGI("Post Handler", "Discovered %d APs", apCount);

		char key[14];
		char ssid_string[33];

		for (int i = 0; i < apCount; i++) {
			sprintf(key, "AP%d", i);
			sprintf(ssid_string, "%s", (char *)get_ap_details(i).ssid);
			ESP_LOGI("Post Handler", "Index: %d, SSID: %s, Length: %d", i, ssid_string, strlen(ssid_string));

			httpd_resp_sendstr_chunk(req, key);
			httpd_resp_sendstr_chunk(req, ",");
			httpd_resp_sendstr_chunk(req, ssid_string);
			if(i != apCount-1) {
				httpd_resp_sendstr_chunk(req, ",");
			}
		}

		httpd_resp_send_chunk(req, "", 0);
		ESP_LOGI("Post Handler", "Sent Response");
		return ESP_OK;
	}

	/*
	 * Check For Access Point Name
	 */
	if (strstr(content, "AccessPoint") == &content[0]) {
		int chosenAP;
		sscanf(content, "AccessPoint=AP%d", &chosenAP);

		ESP_LOGI("Post Handler", "Chose AP %d", chosenAP);

		char ssid[33];
		sprintf(ssid, "%s", (char *)get_ap_details(chosenAP).ssid);

		write_string("ssid_handle", ssid);
		ESP_LOGI("debug", "Saved SSID: %s", ssid);

		// AUTHMODE
		int authmode = get_ap_details(chosenAP).authmode;
		if (authmode == WIFI_AUTH_OPEN) {
			ESP_LOGI("Post Handler", "No password needed for open network");

			write_string("pword_handle", "");

			httpd_resp_set_status(req, "303 See Other");
			httpd_resp_set_hdr(req, "Location", "/connection-check");
			httpd_resp_send(req, NULL, 0);  // Response body can be empty

			return ESP_OK;
		}

		httpd_resp_set_status(req, "303 See Other");
		httpd_resp_set_hdr(req, "Location", "/network-details");
		httpd_resp_send(req, NULL, 0);  // Response body can be empty

		return ESP_OK;
	}
	return ESP_OK;
}

/* Our URI handler function to be called during GET /uri request */
esp_err_t network_details_get_handler(httpd_req_t *req) {
	ESP_LOGI("Get Handler", "Entered get handler");
	/* Get handle to embedded file upload script */
	extern const unsigned char network_details_start[] asm("_binary_network_details_html_start");
	extern const unsigned char network_details_end[]   asm("_binary_network_details_html_end");
	const size_t network_details_size = (network_details_end - network_details_start);

	/* Add file upload form and script which on execution sends a POST request to /upload */
	httpd_resp_send_chunk(req, (const char *)network_details_start, network_details_size);
	httpd_resp_send_chunk(req, "", 0);
	ESP_LOGI("Get Handler", "Sent Response");
	return ESP_OK;
}

esp_err_t network_details_post_handler(httpd_req_t *req) {
	ESP_LOGI("Post Handler", "Entered Network Details Post Handler");

	/* Destination buffer for content of HTTP POST request.
	 * httpd_req_recv() accepts char* only, but content could
	 * as well be any binary data (needs type casting).
	 * In case of string data, null termination will be absent, and
	 * content length would give length of string */
	char content[1000];

	/* Truncate if content length larger than the buffer */
	size_t recv_size = fmin(req->content_len, sizeof(content));
	ESP_LOGI("Debug", "%d", req->content_len);

	int ret = httpd_req_recv(req, content, recv_size);
	if (ret <= 0) { /* 0 return value indicates connection closed */
		/* Check if timeout occurred */
		if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
			/* In case of timeout one can choose to retry calling
			 * httpd_req_recv(), but to keep it simple, here we
			 * respond with an HTTP 408 (Request Timeout) error */
			httpd_resp_send_408(req);
		}
		/* In case of error, returning ESP_FAIL will
		 * ensure that the underlying socket is closed */
		return ESP_FAIL;
	}

	content[req->content_len] = '\0';
	ESP_LOGI("debug", "Received info: %s", content);

	if (strstr(content, "chosen-ap") == &content[0]) {
		// Read NVS
		size_t ssid_size = SSID_SIZE;
		char ssid[SSID_SIZE];
		read_string(SSID_HANDLE, ssid, &ssid_size);

		httpd_resp_sendstr_chunk(req, ssid);

		httpd_resp_send_chunk(req, "", 0);
		ESP_LOGI("Post Handler", "Sent Response");
		return ESP_OK;
	}

	/*
	 * Check For Password
	 */
	if (strstr(content, "password") == &content[0]) {
		char pword[req->content_len];
		sscanf(content, "password=%s", pword);
		pword[req->content_len] = '\0';
		ESP_LOGI("Post Handler", "Password Entered: %s", pword);

		write_string("pword_handle", pword);

		httpd_resp_set_status(req, "303 See Other");
		httpd_resp_set_hdr(req, "Location", "/connection-check");
		httpd_resp_send(req, NULL, 0);  // Response body can be empty

		return ESP_OK;
	}
	return ESP_OK;
}

/* Our URI handler function to be called during GET /uri request */
esp_err_t connection_check_get_handler(httpd_req_t *req) {
	ESP_LOGI("Get Handler", "Entered get handler");
	/* Get handle to embedded file upload script */
	extern const unsigned char connection_check_start[] asm("_binary_connection_check_html_start");
	extern const unsigned char connection_check_end[]   asm("_binary_connection_check_html_end");
	const size_t connection_check_size = (connection_check_end - connection_check_start);

	/* Add file upload form and script which on execution sends a POST request to /upload */
	httpd_resp_send_chunk(req, (const char *)connection_check_start, connection_check_size);
	httpd_resp_send_chunk(req, "", 0);
	ESP_LOGI("Get Handler", "Sent Response");
	return ESP_OK;
}

/* Our URI handler function to be called during GET /uri request */
esp_err_t connection_check_post_handler(httpd_req_t *req) {
	ESP_LOGI("Post Handler", "Entered post handler");


	/* Add file upload form and script which on execution sends a POST request to /upload */
	httpd_resp_sendstr(req, is_sta_connected() ? "1" : "0");
	ESP_LOGI("Post Handler", "Sent Response");
	return ESP_OK;
}

/* Function for starting the webserver */
httpd_handle_t start_webserver(void) {
	/* Generate default configuration */
	httpd_config_t config = HTTPD_DEFAULT_CONFIG();

	/* Use the URI wildcard matching function in order to
	 * allow the same handler to respond to multiple different
	 * target URIs which match the wildcard scheme */
	config.uri_match_fn = httpd_uri_match_wildcard;

	ESP_LOGI("Start Webserver", "Set HTTP Config");

	/* Empty handle to esp_http_server */
	httpd_handle_t server = NULL;

	/* Start the httpd server */
	if (httpd_start(&server, &config) != ESP_OK) {
		/* If server failed to start, handle will be NULL */
		return server;
	}

	/* URI handler for getting first page */
	httpd_uri_t get_network_selection = {
			.uri       = "/",  // Match all URIs
			.method    = HTTP_GET,
			.handler   = network_selection_get_handler,
			.user_ctx  = NULL
	};
	httpd_register_uri_handler(server, &get_network_selection);
	ESP_LOGI("Start Webserver", "Registered get handler");

	/* URI handler for network selection */
	httpd_uri_t post_network_selection = {
			.uri       = "/",   // Match all URIs of type /upload/path/to/file /network-selection
			.method    = HTTP_POST,
			.handler   = network_selection_post_handler,
			.user_ctx  = NULL    // Pass server data as context
	};
	httpd_register_uri_handler(server, &post_network_selection);
	ESP_LOGI("Start Webserver", "Registered network select post handler");

	/* URI handler for getting network details page */
	httpd_uri_t get_network_details = {
			.uri       = "/network-details",  // Match all URIs
			.method    = HTTP_GET,
			.handler   = network_details_get_handler,
			.user_ctx  = NULL
	};
	httpd_register_uri_handler(server, &get_network_details);
	ESP_LOGI("Start Webserver", "Registered get handler");

	/* URI handler for network details */
	httpd_uri_t post_network_details = {
			.uri       = "/network-details",   // Match all URIs of type /upload/path/to/file
			.method    = HTTP_POST,
			.handler   = network_details_post_handler,
			.user_ctx  = NULL    // Pass server data as context
	};
	httpd_register_uri_handler(server, &post_network_details);

	/* URI handler for getting connection check page */
	httpd_uri_t get_connection_check = {
			.uri       = "/connection-check",  // Match all URIs
			.method    = HTTP_GET,
			.handler   = connection_check_get_handler,
			.user_ctx  = NULL
	};
	httpd_register_uri_handler(server, &get_connection_check);
	ESP_LOGI("Start Webserver", "Registered get handler");

	/* URI handler for checking connection */
	httpd_uri_t post_connection_check = {
			.uri       = "/connection-check",   // Match all URIs of type /upload/path/to/file
			.method    = HTTP_POST,
			.handler   = connection_check_post_handler,
			.user_ctx  = NULL    // Pass server data as context
	};
	httpd_register_uri_handler(server, &post_connection_check);

	return server;
}

/* Function for stopping the webserver */
void stop_webserver(httpd_handle_t server) {
	if (server) {
		/* Stop the httpd server */
		httpd_stop(server);
	}
}


