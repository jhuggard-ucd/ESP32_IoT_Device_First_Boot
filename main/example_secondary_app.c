/* * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * INTERNET OF THINGS FIRST BOOT APPLICATION SOFTWARE
 *
 * Example Secondary Application
 * This file is provided as an example of how a developer
 * may write a piece of software that can be used with the
 * IoT First Boot software
 *
 * Author:        James Huggard
 * Last Modified: 28/03/2020
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "example_secondary_app.h"

#define THINGSPEAK_TAG "log_to_thingspeak"


/* The secondary application - Records the brightness through a temperature sensor
 * and logs data on thingspeak */
void log_to_thingspeak(void) {

	esp_adc_cal_characteristics_t *adc_chars;
	adc_channel_t channel = ADC_CHANNEL_0;     	// Select ADC Channel 0
	adc_atten_t atten = ADC_ATTEN_DB_11;
	adc_unit_t unit = ADC_UNIT_1;
	esp_adc_cal_value_t val_type;

	uint32_t adc_reading;
	float percent_brightness;
	int8_t rssi;
	wifi_ap_record_t wifi_data;

	int delay = 20; 							// Number of seconds to delay between each logged reading
	int num_samples = 64;						// Number of samples to average over for each logged value
	float max_brightness = 4096.0; // From channel width

	/* Setup connection to thingspeak */
	thingspeak_initialise();
	ESP_LOGI(THINGSPEAK_TAG, "Posting to ThingSpeak Initialised");


	/* Set up temperature sensor on ADC Channel */
	adc1_config_width(ADC_WIDTH_BIT_12);    	// Setting channel width to 12 - 4096 possible values
	adc1_config_channel_atten(channel, atten);

	adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
	val_type = esp_adc_cal_characterize(unit, atten, ADC_WIDTH_BIT_12, 1100, adc_chars);

	/* While ESP32 is powered on, log RSSI and Detected Brightness */
	while(1) {

		/* Get average brightness over 64 consecutive readings */
		adc_reading = 0;

		for (int i = 0; i < num_samples; i++) {
			num_samples += adc1_get_raw((adc1_channel_t)channel);
		}
		adc_reading /= num_samples;

		/* Get percentage brightness */
		percent_brightness = (max_brightness-adc_reading)/max_brightness*100.0;
		ESP_LOGI(THINGSPEAK_TAG, "Raw Reading: %d\tPercentage Brightness: %.2f%%", adc_reading, percent_brightness);

		/* Get Relative Signal Strength of WiFi */
		if (esp_wifi_sta_get_ap_info(&wifi_data) == ESP_OK) {
			rssi = wifi_data.rssi;
		} else rssi = 0;
		ESP_LOGI(THINGSPEAK_TAG, "RSSI: %d", rssi);

		thingspeak_post_data(percent_brightness, rssi);

		vTaskDelay(delay * 1000 / portTICK_PERIOD_MS);		// Delay of 'delay' seconds
	}
}

