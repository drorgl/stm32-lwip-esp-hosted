#include "cli_ping.h"
#include "cli.h"

#include <stdio.h>
#include <stdbool.h>
#include <lwip/sockets.h>
#include "ping/ping_cmd.h"

bool isValidIpAddress(char *ipAddress, struct sockaddr_in *sa)
{
//    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress, &(sa->sin_addr));
    return result != 0;
}

void ping_cli_callback(ping_result_t *ptResult)
{
//    CtrlWifi_PsStateForce(STA_PS_NONE, 0);
	printf("ping done\r\n");

    return;
}


static void ping_callback(int argc, const char * const * argv,
		void (*print)(const char * message)) {
	if (argc == 1) {
		printf("ping <ip>\r\n");
		return;
	}


	struct sockaddr_in sa;

	if (!isValidIpAddress(argv[1], &sa)){
		//resolve ip
//		dns_gethostbyname)
		printf("hostnames are not implemented\r\n");
	}

	uint32_t count = 3;
	uint32_t pktsz = 64;
	uint32_t recv_timeout = 1000; //ms
	uint32_t ping_period = 1000;  //ms

	 ping_request(count, argv[1], PING_IP_ADDR_V4, pktsz, recv_timeout, ping_period, ping_cli_callback);


//	char buf[127];
//	if (strcmp(argv[1], "encoder") == 0) {
//		buf[0] = 0;
//		uint32_t encoder_value = ADC_read_encoder();
//		sprintf(buf, "encoder: %d\r\n", encoder_value);
//		print(buf);
//	}
//	if (strcmp(argv[1], "temp") == 0) {
//		buf[0] = 0;
//		int32_t temp_value = ADC_read_temperatue();
//		sprintf(buf, "temperature: %d\r\n", temp_value);
//		print(buf);
//	}
//
//	for (uint8_t i = 0; i < argc;i++){
//		buf[0] = 0;
//		sprintf(buf, "param %d, %s\r\n", i, argv[i]);
//		print(buf);
//	}
//

//	print("adc value is bla\r\n");
}

void cli_ping_register(void){
	cli_register("ping", "ping <ip> - ping ip", ping_callback);
}
