#include "cli_wifi.h"

#include "commands.h"
#include <string.h>
#include "cli.h"

#include "lwip_startup.h"
#include "util.h"
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

static void get_mac_callback(int argc, const char * const * argv,
		void (*print)(const char * message)) {
	if (argc == 1) {
		printf("get_mac <mode>\n");
		printf("mode sta - station\n");
		printf("mode ap - access point\n");
		return;
	}

	int mode = 0;
	if (strcmp(argv[1], "sta") == 0) {
		mode = WIFI_MODE_STA;
	} else if (strcmp(argv[1], "ap") == 0) {
		mode = WIFI_MODE_AP;
	} else {
		printf("invalid mode\n");
		return;
	}

	char buffer[32];
	memset(buffer, 0, sizeof(buffer));
	if (wifi_get_mac(mode, buffer) == SUCCESS) {
		if (mode == WIFI_MODE_STA) {
			printf("Station MAC %s\n", buffer);
		} else if (mode == WIFI_MODE_AP) {
			printf("AP MAC %s\n", buffer);
		}
	} else {
		printf("Failed to get MAC\n");
	}

}

/*
 * wifi set mac function sets custom mac address to ESP32's station and softAP Interface, returns SUCCESS(0) or FAILURE(-1)
 * Input parameter:
 *      mode : ESP32 wifi mode
 *          (WIFI_MODE_STA  : for station mac
 *           WIFI_MODE_AP   : for softAP mac)
 *      mac  : custom MAC Address for ESP32 Interface
 * @attention 1. First set wifi mode before setting MAC address for respective station and softAP Interface
 * @attention 2. ESP32 station and softAP have different MAC addresses, do not set them to be the same.
 * @attention 3. The bit 0 of the first byte of ESP32 MAC address can not be 1.
 * For example, the MAC address can set to be "1a:XX:XX:XX:XX:XX", but can not be "15:XX:XX:XX:XX:XX".
 * @attention 4. MAC address will get reset after esp restarts
 *
 */
//int wifi_set_mac(int mode, char *mac);
static void get_mode_callback(int argc, const char * const * argv,
		void (*print)(const char * message)) {

	int mode = 0;

	if (wifi_get_mode(&mode) == SUCCESS) {
		switch (mode) {

		case WIFI_MODE_NONE:
			printf("WiFi Mode None\n");
			break;
		case WIFI_MODE_STA:
			printf("WiFi Mode Station\n");
			break;
		case WIFI_MODE_AP:
			printf("WiFi Mode Access Point\n");
			break;
		case WIFI_MODE_APSTA:
			printf("WiFi Mode Access Point/Station\n");
			break;
		case WIFI_MODE_MAX:
			printf("WiFi Mode Max\n");
			break;
		}

	} else {
		printf("Failed to get mode\n");
	}
}

static void set_mode_callback(int argc, const char * const * argv,
		void (*print)(const char * message)) {

	if (argc == 1) {
		printf("set_mode <mode>\n");
		printf("mode sta - station\n");
		printf("mode ap - access point\n");
		printf("mode apsta - access point\n");
		return;
	}

	int mode = 0;
	if (strcmp(argv[1], "sta") == 0) {
		mode = WIFI_MODE_STA;
	} else if (strcmp(argv[1], "ap") == 0) {
		mode = WIFI_MODE_AP;
	} else if (strcmp(argv[1], "apsta") == 0) {
		mode = WIFI_MODE_APSTA;
	} else {
		printf("invalid mode\n");
		return;
	}

	if (wifi_set_mode(mode) == SUCCESS) {
		printf("Successfully set mode\n");
	} else {
		printf("Failed to get MAC\n");
	}
}

static void get_ap_config_callback(int argc, const char * const * argv,
		void (*print)(const char * message)) {

	esp_hosted_control_config_t config;
	if (wifi_get_ap_config(&config) == SUCCESS) {
		printf("Station:\n");
		printf("SSID %s\n", config.station.ssid);
		printf("Password %s\n", config.station.pwd);
		printf("BSSID %s\n", config.station.bssid);
		printf("WPA3 Supported %d\n", config.station.is_wpa3_supported);
		printf("RSSI %d\n", config.station.rssi);
		printf("Channel %d\n", config.station.channel);
		printf("Encryption Mode %d\n", config.station.encryption_mode);
		printf("Listen Interval %d\n", config.station.listen_interval);
		printf("Status %s\n", config.station.status);

		printf("SoftAP:\n");
		printf("SSID %s\n", config.softap.ssid);
		printf("Password %s\n", config.softap.pwd);
		printf("Channel %d\n", config.softap.channel);
		printf("Encryption Mode %d\n", config.softap.encryption_mode);
		printf("Max Connections %d\n", config.softap.max_connections);
		printf("SSID Hidden %d\n", config.softap.ssid_hidden);
		printf("Bandwidth %d\n", config.softap.bandwidth);

	} else {
		printf("Failed to get ap config\n");
	}

}

static void disconnect_ap_callback(int argc, const char * const * argv,
		void (*print)(const char * message)) {

	if (wifi_disconnect_ap() == SUCCESS) {
		printf("Disconnected successfully\n");
	} else {
		printf("Failed to disconnect\n");
	}
}

//static int min(int x, int y) {
//	return (x < y) ? x : y;
//}

static void set_softap_config_callback(int argc, const char * const * argv,
		void (*print)(const char * message)) {

	if (argc < 3) {
		printf("set_softap_config <ssid> <password>\n");
		return;
	}

	int ret;
	esp_hosted_control_config_t ap_config = { 0 };

	strncpy((char*) &ap_config.station.ssid, argv[1],
			MIN(SSID_LENGTH, strlen(argv[1]) + 1));
	strncpy((char*) &ap_config.station.pwd, argv[2],
			MIN(PASSWORD_LENGTH, strlen(argv[2]) + 1));
//		if (strlen(INPUT_STATION_BSSID)) {
//			strncpy((char* )&ap_config.station.bssid,   INPUT_STATION_BSSID,
//				min(BSSID_LENGTH,    strlen(INPUT_STATION_BSSID)+1));
//		}
//		ap_config.station.is_wpa3_supported =
//			get_boolean_param(INPUT_STATION_IS_WPA3_SUPPORTED);

	ret = wifi_set_ap_config(ap_config);
	if (ret) {
		printf("Failed to connect with AP \n");
	} else {
		printf("Connected to %s \n", ap_config.station.ssid);
	}
}

static void get_softap_config_callback(int argc, const char * const * argv,
		void (*print)(const char * message)) {

	esp_hosted_control_config_t config;
	if (wifi_get_softap_config(&config) == SUCCESS) {
		printf("Station:\n");
		printf("SSID %s\n", config.station.ssid);
		printf("Password %s\n", config.station.pwd);
		printf("BSSID %s\n", config.station.bssid);
		printf("WPA3 Supported %d\n", config.station.is_wpa3_supported);
		printf("RSSI %d\n", config.station.rssi);
		printf("Channel %d\n", config.station.channel);
		printf("Encryption Mode %d\n", config.station.encryption_mode);
		printf("Listen Interval %d\n", config.station.listen_interval);
		printf("Status %s\n", config.station.status);

		printf("SoftAP:\n");
		printf("SSID %s\n", config.softap.ssid);
		printf("Password %s\n", config.softap.pwd);
		printf("Channel %d\n", config.softap.channel);
		printf("Encryption Mode %d\n", config.softap.encryption_mode);
		printf("Max Connections %d\n", config.softap.max_connections);
		printf("SSID Hidden %d\n", config.softap.ssid_hidden);
		printf("Bandwidth %d\n", config.softap.bandwidth);

	} else {
		printf("Failed to get ap config\n");
	}

}

static void stop_softap_callback(int argc, const char * const * argv,
		void (*print)(const char * message)) {

	if (wifi_stop_softap() == SUCCESS) {
		printf("SoftAP stopped successfully\n");
	} else {
		printf("Failed to stop softap\n");
	}
}

static void ap_scan_list_callback(int argc, const char * const * argv,
		void (*print)(const char * message)) {

	int count = 0;

	esp_hosted_wifi_scanlist_t* list = wifi_ap_scan_list(&count);
	if (count > 0) {
		for (int i = 0; i < count; i++) {
			printf("%s, %s, %d, %d, %d\n", list[i].ssid, list[i].bssid,
					list[i].rssi, list[i].channel, list[i].encryption_mode);
		}
	} else {
		printf("No APs found\n");
	}

	esp_hosted_free(list);

}

static void connected_stations_list_callback(int argc,
		const char * const * argv, void (*print)(const char * message)) {

	int count = 0;

	esp_hosted_wifi_connected_stations_list* list =
			wifi_connected_stations_list(&count);
	if (count > 0) {
		for (int i = 0; i < count; i++) {
			printf("%s, %d\n", list[i].bssid, list[i].rssi);
		}
	} else {
		printf("No Connected APs\n");
	}

}

static void start_lwip_callback(int argc, const char * const * argv,
		void (*print)(const char * message)) {
	char buffer[32] = { 0 };
	if (wifi_get_mac(WIFI_MODE_STA, buffer) == SUCCESS) {
		printf("Station MAC %s\n", buffer);

		uint8_t parsed_mac[6];
		if (STM_OK == convert_mac_to_bytes(parsed_mac, buffer)) {
			printf("converted mac successfully\r\n");
			lwip_startup(parsed_mac);
		}
	} else {
		printf("Failed to get MAC\n");
	}
}

/*
 * wifi set power save mode function sets power save mode of ESP32, returns SUCCESS(0) or FAILURE(-1)
 *  Input parameter:
 *      power save mode : ESP32's power save mode
 *                  (1  : WIFI_PS_MIN_MODEM,   Minimum modem power saving.
 *                        In this mode, station wakes up to receive beacon every DTIM period
 *                   2  : WIFI_PS_MAX_MODEM,   Maximum modem power saving.
 *                        In this mode, interval to receive beacons is determined
 *                        by the listen_interval parameter in wifi_set_ap_config function
 *      Default :: power save mode is WIFI_PS_MIN_MODEM
 */
//int wifi_set_power_save_mode(int power_save_mode);
/*
 * wifi get power save mode function gives power save mode of ESP32, returns SUCCESS(0) or FAILURE(-1)
 *  Output parameter:
 *      power save mode : ESP32's power save mode
 *                  (1  : WIFI_PS_MIN_MODEM,   Minimum modem power saving.
 *                        In this mode, station wakes up to receive beacon every DTIM period
 *                   2  : WIFI_PS_MAX_MODEM,   Maximum modem power saving.
 *                        In this mode, interval to receive beacons is determined
 *                        by the listen_interval parameter in wifi_set_ap_config function
 *                   3  : WIFI_PS_INVALID,     Invalid power save mode. In case of failure of command
 */
//int wifi_get_power_save_mode(int *power_save_mode);
/*
 * esp ota begin function performs an OTA begin operation for ESP32
 * which sets partition for OTA write and erase it.
 * returns SUCCESS(0) or FAILURE(-1)
 */
//int esp_ota_begin();
/*
 * esp ota write function performs an OTA write operation for ESP32,
 * It writes ota_data buffer to OTA partition in flash
 * returns SUCCESS(0) or FAILURE(-1)
 *  Input parameter:
 *      ota_data        : OTA data buffer
 *      ota_data_len    : length of OTA data buffer
 */
//int esp_ota_write(uint8_t* ota_data, uint32_t ota_data_len);
/*
 * esp ota end function performs an OTA end operation for ESP32,
 * It validates written OTA image, set OTA partition as boot partition for next boot,
 * Creates timer which reset ESP32 after 5 sec,
 * returns SUCCESS(0) or FAILURE(-1)
 */
//int esp_ota_end();
//
//
//static void get_mac_callback(int argc, const char * const * argv,
//		void (*print)(const char * message)) {
//	if (argc == 1) {
//		printf("ping <ip>\n");
//		return;
//	}
//
//
//	struct sockaddr_in sa;
//
//	if (!isValidIpAddress(argv[1], &sa)){
//		//resolve ip
////		dns_gethostbyname)
//		printf("hostnames are not implemented\n");
//	}
//
//	uint32_t count = 3;
//	uint32_t pktsz = 64;
//	uint32_t recv_timeout = 1000; //ms
//	uint32_t ping_period = 1000;  //ms
//
//	ping_request(count, argv[1], PING_IP_ADDR_V4, pktsz, recv_timeout, ping_period, ping_cli_callback);
//
//
////	char buf[127];
////	if (strcmp(argv[1], "encoder") == 0) {
////		buf[0] = 0;
////		uint32_t encoder_value = ADC_read_encoder();
////		sprintf(buf, "encoder: %d\n", encoder_value);
////		print(buf);
////	}
////	if (strcmp(argv[1], "temp") == 0) {
////		buf[0] = 0;
////		int32_t temp_value = ADC_read_temperatue();
////		sprintf(buf, "temperature: %d\n", temp_value);
////		print(buf);
////	}
////
////	for (uint8_t i = 0; i < argc;i++){
////		buf[0] = 0;
////		sprintf(buf, "param %d, %s\n", i, argv[i]);
////		print(buf);
////	}
////
//
////	print("adc value is bla\n");
//}
void cli_wifi_register(void) {
	cli_register("get_mac", "get_mac <mode> - retrieve mac address for mode",
			get_mac_callback);
	cli_register("get_mode", "get_mode - retrieve wifi mode",
			get_mode_callback);
	cli_register("set_mode", "set_mode <mode> - set wifi mode",
			set_mode_callback);
	cli_register("get_config", "get_config - get wifi config",
			get_ap_config_callback);
	cli_register("get_softap_config",
			"get_softap_config - get soft ap wifi config",
			get_softap_config_callback);
	cli_register("disconnect_ap", "disconnect_ap - disconnect ap",
			disconnect_ap_callback);
	cli_register("stop_softap", "stop_softap - stop soft ap",
			stop_softap_callback);
	cli_register("scan", "scan - scan ap list", ap_scan_list_callback);
	cli_register("show_connected", "show_connected - show connected stations",
			connected_stations_list_callback);
	cli_register("join",
			"join <ssid> <password> - join wifi network",
			set_softap_config_callback);
	cli_register("start_lwip", "start_lwip - start lwip stack",
			start_lwip_callback);

}

