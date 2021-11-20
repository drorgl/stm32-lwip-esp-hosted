#pragma once

#include <stdint.h>
#include <lwip/ip_addr.h>

void lwip_startup(uint8_t mac_address[6]) ;

ip4_addr_t * get_ip_address();
