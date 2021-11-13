#include "lwip_startup.h"

#include "lwip/init.h"

#include <lwip/tcpip.h>
#include <lwip/netif.h>
#include <lwip/ip.h>
#include <lwip/ip4_addr.h>
#include <lwip/prot/ethernet.h>
#include <lwip/prot/etharp.h>
#include <lwip/dhcp.h>
#include <lwip/snmp.h>
#include <string.h>
#include <assert.h>

#include "netdev_api.h"
#include "spi_drv.h"

struct netif gnetif;
ip4_addr_t ipaddr;
ip4_addr_t netmask;
ip4_addr_t gw;

err_t ethernetif_init_low(struct netif *netif) {
	//lwip init wifi phy
	return 0;
}

struct network_handle *sta_handle, *ap_handle;

static void sta_rx_callback(struct network_handle *net_handle) {
	struct esp_pbuf *rx_buffer = NULL;

	//uint8_t *arp_resp = NULL;
	//uint16_t arp_resp_len = 0;
	//uint32_t sta_ip = 0;

	rx_buffer = network_read(net_handle, 0);

	if (rx_buffer) {

		//send to lwip interface
		struct pbuf* p = pbuf_alloc(PBUF_RAW, rx_buffer->len, PBUF_POOL);

		if (p != NULL) {
			memcpy(p->payload, rx_buffer->payload, rx_buffer->len);

			err_t err = gnetif.input(p, &gnetif);
			if (err != ERR_OK) {
				printf("error ingesting frame\r\n");
				pbuf_free(p);
			}
		}
//		if (p) {
//			pbuf_free(p);
//		}

		free(rx_buffer->payload);
		rx_buffer->payload = NULL;
		free(rx_buffer);
		rx_buffer = NULL;
	}
}

err_t myif_link_output(struct netif *netif, struct pbuf *p) {

	struct pbuf *q;

#if ETH_PAD_SIZE
	pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif
	for (q = p; q != NULL; q = q->next) {
		/* Send the data from the pbuf to the interface, one pbuf at a
		 time. The size of the data in each pbuf is kept in the ->len
		 variable. */
//	    send data from(q->payload, q->len);
//	ret = network_write(sta_handle, snd_buffer);
//	if (ret) {
//		printf("%s: Failed to send arp response\n\r", __func__);
//	}
		struct esp_pbuf *snd_buffer = NULL;
		snd_buffer = malloc(sizeof(struct esp_pbuf));
		assert(snd_buffer);

		snd_buffer->payload = malloc(q->len);
		assert(snd_buffer->payload);

		memcpy(snd_buffer->payload, q->payload, q->len);

		snd_buffer->len = q->len;

		int ret = network_write(sta_handle, snd_buffer);

		if (ret) {
			printf("%s: Failed to send packet\n\r", __func__);
		}
	}

	MIB2_STATS_NETIF_ADD(netif, ifoutoctets, p->tot_len);
	if (((u8_t*) p->payload)[0] & 1) {
		/* broadcast or multicast packet*/
		MIB2_STATS_NETIF_INC(netif, ifoutnucastpkts);
	} else {
		/* unicast packet */
		MIB2_STATS_NETIF_INC(netif, ifoutucastpkts);
	}
	/* increase ifoutdiscards or ifouterrors on error */

#if ETH_PAD_SIZE
	pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif

	LINK_STATS_INC(link.xmit);

	return ERR_OK;

}

//err_t myif_output(struct netif *netif, struct pbuf *p,
//       const ip4_addr_t *ipaddr) {
//
//}

void lwip_startup(uint8_t mac_address[6]) {
	/* Initialize the LwIP stack with RTOS */
	tcpip_init( NULL, NULL);

	sta_handle = network_open(STA_INTERFACE, sta_rx_callback);
	assert(sta_handle);

	/* set MAC hardware address length */
	gnetif.hwaddr_len = ETHARP_HWADDR_LEN;

	/* set MAC hardware address */
	memcpy(gnetif.hwaddr, mac_address, NETIF_MAX_HWADDR_LEN);

	/* IP addresses initialization with DHCP (IPv4) */
	ip4_addr_set_zero(&ipaddr);
	ip4_addr_set_zero(&netmask);
	ip4_addr_set_zero(&gw);

	/* add the network interface (IPv4/IPv6) with RTOS */
	netif_add(&gnetif, &ipaddr, &netmask, &gw, NULL, ethernetif_init_low,
			tcpip_input);
	gnetif.linkoutput = myif_link_output;
//	gnetif.output = myif_output;
	gnetif.output = etharp_output;
	gnetif.hwaddr_len = 6;
	gnetif.mtu = 1500;
	gnetif.name[0] = 's';
	gnetif.name[1] = 't';
	gnetif.flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP
			| NETIF_FLAG_LINK_UP;
#if LWIP_NETIF_HOSTNAME
	/* Initialize interface hostname */
	gnetif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

	/* Registers the default network interface */
	netif_set_default(&gnetif);

	if (netif_is_link_up(&gnetif)) {
		/* When the netif is fully configured this function must be called */
		netif_set_up(&gnetif);
	} else {
		/* When the netif link is down this function must be called */
		netif_set_down(&gnetif);
	}

	/* Start DHCP negotiation for a network interface (IPv4) */
	dhcp_start(&gnetif);
}

void lwip_example_app_platform_assert(const char *msg, int line,
		const char *file) {
	printf("Assertion \"%s\" failed at line %d in %s\n", msg, line, file);
	fflush(NULL);
	abort();
}

unsigned int lwip_port_rand(void) {
	return (u32_t) rand();
}
