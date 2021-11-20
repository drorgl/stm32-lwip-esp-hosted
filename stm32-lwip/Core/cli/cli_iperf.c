#include "cli_iperf.h"

#include <stdio.h>
#include "iperf.h"
#include "argtable3.h"
#include "lwip_startup.h"
#include <string.h>

//https://github.com/espressif/esp-idf/blob/master/examples/wifi/iperf/README.md
typedef struct {
    struct arg_str *ip;
    struct arg_lit *server;
    struct arg_lit *udp;
    struct arg_lit *version;
    struct arg_int *port;
    struct arg_int *length;
    struct arg_int *interval;
    struct arg_int *time;
    struct arg_lit *abort;
    struct arg_end *end;
} wifi_iperf_t;

static wifi_iperf_t iperf_args;

static void iperf_callback(int argc, const char * const * argv,
		void (*print)(const char * message)) {
//	if (argc == 1) {
//		printf("iperf <arguments> - this is iperf2 implementation from Espressif\r\n");
//		printf("-c <ip> run in client mode, connecting to <host>\r\n");
//		printf("-s run in server mode\r\n");
//		printf("-u use UDP rather than TCP\r\n");
//		printf("-V use IPV6 address rather than IPV4\r\n");
//
//		printf("-p <port> server port to listen on/connect to\r\n");
//		printf("-l <length> Set read/write buffer size\r\n");
//		printf("-i <interval> seconds between periodic bandwidth reports\r\n");
//		printf("-t <time> time in seconds to transmit for (default 10 secs)\r\n");
//		printf("-a abort running iperf\r\n");

//		return;
//	}


	//	iperf_args.ip = arg_str0("c", "client", "<ip>", "run in client mode, connecting to <host>");
	//	    iperf_args.server = arg_lit0("s", "server", "run in server mode");
	//	    iperf_args.udp = arg_lit0("u", "udp", "use UDP rather than TCP");
	//	    iperf_args.version = arg_lit0("V", "ipv6_domain", "use IPV6 address rather than IPV4");
	//	    iperf_args.port = arg_int0("p", "port", "<port>", "server port to listen on/connect to");
	//	    iperf_args.length = arg_int0("l", "len", "<length>", "Set read/write buffer size");
	//	    iperf_args.interval = arg_int0("i", "interval", "<interval>", "seconds between periodic bandwidth reports");
	//	    iperf_args.time = arg_int0("t", "time", "<time>", "time in seconds to transmit for (default 10 secs)");
	//	    iperf_args.abort = arg_lit0("a", "abort", "abort running iperf");
	//	    iperf_args.end = arg_end(1);

		 int nerrors = arg_parse(argc, argv, (void **) &iperf_args);
		    iperf_cfg_t cfg;

		    if (nerrors != 0) {
		        arg_print_errors(stderr, iperf_args.end, argv[0]);
		        return;
		    }

		    memset(&cfg, 0, sizeof(cfg));

		    // now wifi iperf only support IPV4 address
		    cfg.type = IPERF_IP_TYPE_IPV4;

		    if ( iperf_args.abort->count != 0) {
		        printf("stopping\r\n");
		        iperf_stop();
		        return;
		    }

		    if ( ((iperf_args.ip->count == 0) && (iperf_args.server->count == 0)) ||
		            ((iperf_args.ip->count != 0) && (iperf_args.server->count != 0)) ) {
		        printf( "should specific client/server mode\r\n");
		        return;
		    }

		    if (iperf_args.ip->count == 0) {
		        cfg.flag |= IPERF_FLAG_SERVER;
		    } else {

		        ip4addr_aton(iperf_args.ip->sval[0],&cfg.destination_ip4 );
		        cfg.flag |= IPERF_FLAG_CLIENT;
		    }

		    cfg.source_ip4 = ip4_addr_get_u32(get_ip_address());
//		    cfg.source_ip4 = wifi_get_local_ip();
		    if (cfg.source_ip4 == 0) {
		    	printf("No Local IP\r\n");
		        return;
		    }

		    if (iperf_args.udp->count == 0) {
		        cfg.flag |= IPERF_FLAG_TCP;
		    } else {
		        cfg.flag |= IPERF_FLAG_UDP;
		    }

		    if (iperf_args.length->count == 0) {
		        cfg.len_send_buf = 0;
		    } else {
		        cfg.len_send_buf = iperf_args.length->ival[0];
		    }

		    if (iperf_args.port->count == 0) {
		        cfg.sport = IPERF_DEFAULT_PORT;
		        cfg.dport = IPERF_DEFAULT_PORT;
		    } else {
		        if (cfg.flag & IPERF_FLAG_SERVER) {
		            cfg.sport = iperf_args.port->ival[0];
		            cfg.dport = IPERF_DEFAULT_PORT;
		        } else {
		            cfg.sport = IPERF_DEFAULT_PORT;
		            cfg.dport = iperf_args.port->ival[0];
		        }
		    }

		    if (iperf_args.interval->count == 0) {
		        cfg.interval = IPERF_DEFAULT_INTERVAL;
		    } else {
		        cfg.interval = iperf_args.interval->ival[0];
		        if (cfg.interval <= 0) {
		            cfg.interval = IPERF_DEFAULT_INTERVAL;
		        }
		    }

		    if (iperf_args.time->count == 0) {
		        cfg.time = IPERF_DEFAULT_TIME;
		    } else {
		        cfg.time = iperf_args.time->ival[0];
		        if (cfg.time <= cfg.interval) {
		            cfg.time = cfg.interval;
		        }
		    }

		    printf( "mode=%s-%s sip=%d.%d.%d.%d:%d, dip=%d.%d.%d.%d:%d, interval=%d, time=%d\r\n",
		             cfg.flag & IPERF_FLAG_TCP ? "tcp" : "udp",
		             cfg.flag & IPERF_FLAG_SERVER ? "server" : "client",
		             cfg.source_ip4 & 0xFF, (cfg.source_ip4 >> 8) & 0xFF, (cfg.source_ip4 >> 16) & 0xFF,
		             (cfg.source_ip4 >> 24) & 0xFF, cfg.sport,
		             cfg.destination_ip4 & 0xFF, (cfg.destination_ip4 >> 8) & 0xFF,
		             (cfg.destination_ip4 >> 16) & 0xFF, (cfg.destination_ip4 >> 24) & 0xFF, cfg.dport,
		             cfg.interval, cfg.time);

		    iperf_start(&cfg);

		    return;
}

void cli_iperf_register(void){
	iperf_args.ip = arg_str0("c", "client", "<ip>",
			"run in client mode, connecting to <host>");
	iperf_args.server = arg_lit0("s", "server", "run in server mode");
	iperf_args.udp = arg_lit0("u", "udp", "use UDP rather than TCP");
	iperf_args.version = arg_lit0("V", "ipv6_domain",
			"use IPV6 address rather than IPV4");
	iperf_args.port = arg_int0("p", "port", "<port>",
			"server port to listen on/connect to");
	iperf_args.length = arg_int0("l", "len", "<length>",
			"Set read/write buffer size");
	iperf_args.interval = arg_int0("i", "interval", "<interval>",
			"seconds between periodic bandwidth reports");
	iperf_args.time = arg_int0("t", "time", "<time>",
			"time in seconds to transmit for (default 10 secs)");
	iperf_args.abort = arg_lit0("a", "abort", "abort running iperf");
	iperf_args.end = arg_end(1);

	cli_register("iperf", "iperf <arguments> - run iperf", iperf_callback);
}
