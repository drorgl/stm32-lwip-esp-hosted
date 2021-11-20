/* Iperf Example - iperf implementation

 This example code is in the Public Domain (or CC0 licensed, at your option.)

 Unless required by applicable law or agreed to in writing, this
 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 CONDITIONS OF ANY KIND, either express or implied.

 Source: https://github.com/espressif/esp-idf/tree/master/examples/common_components/iperf
 */

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include "FreeRTOS.h"
#include "task.h"
#include "iperf.h"
#include <stdbool.h>

typedef struct {
	iperf_cfg_t cfg;
	bool finish;
	uint32_t actual_len;
	uint32_t buffer_len;
	uint8_t *buffer;
	uint32_t sockfd;
} iperf_ctrl_t;

static bool s_iperf_is_running = false;
static iperf_ctrl_t s_iperf_ctrl;

inline static bool iperf_is_udp_client(void) {
	return ((s_iperf_ctrl.cfg.flag & IPERF_FLAG_CLIENT)
			&& (s_iperf_ctrl.cfg.flag & IPERF_FLAG_UDP));
}

inline static bool iperf_is_udp_server(void) {
	return ((s_iperf_ctrl.cfg.flag & IPERF_FLAG_SERVER)
			&& (s_iperf_ctrl.cfg.flag & IPERF_FLAG_UDP));
}

inline static bool iperf_is_tcp_client(void) {
	return ((s_iperf_ctrl.cfg.flag & IPERF_FLAG_CLIENT)
			&& (s_iperf_ctrl.cfg.flag & IPERF_FLAG_TCP));
}

inline static bool iperf_is_tcp_server(void) {
	return ((s_iperf_ctrl.cfg.flag & IPERF_FLAG_SERVER)
			&& (s_iperf_ctrl.cfg.flag & IPERF_FLAG_TCP));
}

static int iperf_get_socket_error_code(int sockfd) {
	return errno;
}

static int iperf_show_socket_error_reason(const char *str, int sockfd) {
	int err = errno;
	if (err != 0) {
		printf("%s error, error code: %d, reason: %s\r\n", str, err,
				strerror(err));
	}

	return err;
}

static void iperf_report_task(void *arg) {
	uint32_t interval = s_iperf_ctrl.cfg.interval;
	uint32_t time = s_iperf_ctrl.cfg.time;
	TickType_t delay_interval = (interval * 1000) / portTICK_PERIOD_MS;
	uint32_t cur = 0;
	double average = 0;
	double actual_bandwidth = 0;
	int k = 1;

	printf("\n%16s %s\n", "Interval", "Bandwidth");
	while (!s_iperf_ctrl.finish) {
		vTaskDelay(delay_interval);
		actual_bandwidth = (s_iperf_ctrl.actual_len / 1e6 * 8) / interval;
		printf("%4d-%4d sec       %.2f Mbits/sec\n", cur, cur + interval,
				actual_bandwidth);
		cur += interval;
		average = ((average * (k - 1) / k) + (actual_bandwidth / k));
		k++;
		s_iperf_ctrl.actual_len = 0;
		if (cur >= time) {
			printf("%4d-%4d sec       %.2f Mbits/sec\n", 0, time, average);
			break;
		}
	}

	s_iperf_ctrl.finish = true;
	vTaskDelete(NULL);
}

static int iperf_start_report(void) {
	int ret;

	ret = xTaskCreate(iperf_report_task, IPERF_REPORT_TASK_NAME,
			IPERF_REPORT_TASK_STACK, NULL, IPERF_REPORT_TASK_PRIORITY, NULL);

	if (ret != pdPASS) {
		printf("create task %s failed\r\n", IPERF_REPORT_TASK_NAME);
		return 1;
	}

	return 0;
}

static void socket_recv(int recv_socket, struct sockaddr_storage listen_addr,
		uint8_t type) {
	bool udp_recv_start = true;
	uint8_t *buffer;
	int want_recv = 0;
	int actual_recv = 0;
	socklen_t addr_len = sizeof(struct sockaddr);

	buffer = s_iperf_ctrl.buffer;
	want_recv = s_iperf_ctrl.buffer_len;

	while (!s_iperf_ctrl.finish) {
#if LWIP_IPV6
        if (s_iperf_ctrl.cfg.type == IPERF_IP_TYPE_IPV6) {
            addr_len = sizeof(struct sockaddr_in6);
            actual_recv = recvfrom(recv_socket, buffer, want_recv, 0, (struct sockaddr *)&listen_addr, &addr_len);
        } else
#endif
		if (s_iperf_ctrl.cfg.type == IPERF_IP_TYPE_IPV4) {
			addr_len = sizeof(struct sockaddr_in);
			actual_recv = recvfrom(recv_socket, buffer, want_recv, 0,
					(struct sockaddr* )&listen_addr, &addr_len);
		}
		if (actual_recv < 0) {
			if (type == IPERF_TRANS_TYPE_TCP) {
				iperf_show_socket_error_reason("tcp server recv", recv_socket);
			}
			if (type == IPERF_TRANS_TYPE_UDP) {
				iperf_show_socket_error_reason("udp server recv", recv_socket);
			}
			s_iperf_ctrl.finish = true;
			break;
		} else {
			if (udp_recv_start) {
				iperf_start_report();
				udp_recv_start = false;
			}
			s_iperf_ctrl.actual_len += actual_recv;
		}
	}
}

static void socket_send(int send_socket, struct sockaddr_storage dest_addr,
		uint8_t type) {
	bool retry = false;
	uint8_t *buffer;
	uint8_t delay = 0;
	int actual_send = 0;
	int want_send = 0;
	int err = 0;

	buffer = s_iperf_ctrl.buffer;
	want_send = s_iperf_ctrl.buffer_len;
	iperf_start_report();

	while (!s_iperf_ctrl.finish) {
		if (type == IPERF_TRANS_TYPE_UDP) {
			if (false == retry) {
				delay = 1;
			}
			retry = false;
		}
#if LWIP_IPV6
        if (s_iperf_ctrl.cfg.type == IPERF_IP_TYPE_IPV6) {
            actual_send = sendto(send_socket, buffer, want_send, 0, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr_in6));
        } else
#endif
		if (s_iperf_ctrl.cfg.type == IPERF_IP_TYPE_IPV4) {
			actual_send = sendto(send_socket, buffer, want_send, 0,
					(struct sockaddr* )&dest_addr, sizeof(struct sockaddr_in));
		}
		if (actual_send != want_send) {
			if (type == IPERF_TRANS_TYPE_UDP) {
				err = iperf_get_socket_error_code(send_socket);
				if (err == ENOMEM) {
					vTaskDelay(delay);
					if (delay < IPERF_MAX_DELAY) {
						delay <<= 1;
					}
					retry = true;
					continue;
				} else {
					if (s_iperf_ctrl.cfg.type == IPERF_IP_TYPE_IPV4) {
						printf("udp client send abort: err=%d\r\n", err);
					}
				}
			}
			if (type == IPERF_TRANS_TYPE_TCP) {
				iperf_show_socket_error_reason("tcp client send", send_socket);
				printf("tcp client send error\r\n");
				break;
			}
		} else {
			s_iperf_ctrl.actual_len += actual_send;
		}
	}
}

static int iperf_run_tcp_server(void) {
	int listen_socket = -1;
	int client_socket = -1;
	int opt = 1;
	int err = 0;
	int ret = 0;
	struct sockaddr_in remote_addr;
	struct timeval timeout = { 0 };
	socklen_t addr_len = sizeof(struct sockaddr);
	struct sockaddr_storage listen_addr = { 0 };
#if LWIP_IPV6
    struct sockaddr_in6 listen_addr6 = { 0 };
#endif
	struct sockaddr_in listen_addr4 = { 0 };

	if (!(s_iperf_ctrl.cfg.type == IPERF_IP_TYPE_IPV6
			|| s_iperf_ctrl.cfg.type == IPERF_IP_TYPE_IPV4)) {
		printf("Invalid AF types\r\n");
		ret = 1;
		goto exit;
	}

#if LWIP_IPV6
    if (s_iperf_ctrl.cfg.type == IPERF_IP_TYPE_IPV6) {
        // The TCP server listen at the address "::", which means all addresses can be listened to.
        inet6_aton("::", &listen_addr6.sin6_addr);
        listen_addr6.sin6_family = AF_INET6;
        listen_addr6.sin6_port = htons(s_iperf_ctrl.cfg.sport);

        listen_socket = socket(AF_INET6, SOCK_STREAM, IPPROTO_IPV6);
        if (!(listen_socket >= 0)){
        	printf("Unable to create socket: errno %d", errno);
        	ret = 1;
        	goto exit;
        }

        setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        setsockopt(listen_socket, IPPROTO_IPV6, IPV6_V6ONLY, &opt, sizeof(opt));

        printf( "Socket created\r\n");

        err = bind(listen_socket, (struct sockaddr *)&listen_addr6, sizeof(listen_addr6));
        if (!(err == 0)){
        	printf("Socket unable to bind: errno %d, IPPROTO: %d\r\n", errno, AF_INET6);
        	ret = 1;
        	goto exit;
        }


        err = listen(listen_socket, 1);
        if (!(err == 0)){
        	printf("Error occurred during listen: errno %d\r\n", errno);
        	ret = 1;
        	goto exit;
        }

        timeout.tv_sec = IPERF_SOCKET_RX_TIMEOUT;
        setsockopt(listen_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

        memcpy(&listen_addr, &listen_addr6, sizeof(listen_addr6));
    } else
#endif
	if (s_iperf_ctrl.cfg.type == IPERF_IP_TYPE_IPV4) {
		listen_addr4.sin_family = AF_INET;
		listen_addr4.sin_port = htons(s_iperf_ctrl.cfg.sport);
		listen_addr4.sin_addr.s_addr = s_iperf_ctrl.cfg.source_ip4;

		listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (!(listen_socket >= 0)) {
			printf("Unable to create socket: errno %d\r\n", errno);
			ret = 1;
			goto exit;
		}

		setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

		printf("Socket created\r\n");

		err = bind(listen_socket, (struct sockaddr* )&listen_addr4,
				sizeof(listen_addr4));

		if (!(err == 0)) {
			printf("Socket unable to bind: errno %d, IPPROTO: %d\r\n", errno,
					AF_INET);
			ret = 1;
			goto exit;
		}

		err = listen(listen_socket, 5);
		if (!(err == 0)) {
			printf("Error occurred during listen: errno %d\r\n", errno);
			ret = 1;
			goto exit;
		}
		memcpy(&listen_addr, &listen_addr4, sizeof(listen_addr4));
	}

	client_socket = accept(listen_socket, (struct sockaddr* )&remote_addr,
			&addr_len);
	if (!(client_socket >= 0)) {
		printf("Unable to accept connection: errno %d\r\n", errno);
		ret = 1;
		goto exit;
	}
	printf("accept: %s,%d\r\n", inet_ntoa(remote_addr.sin_addr),
			htons(remote_addr.sin_port));

	timeout.tv_sec = IPERF_SOCKET_RX_TIMEOUT;
	setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout,
			sizeof(timeout));

	socket_recv(client_socket, listen_addr, IPERF_TRANS_TYPE_TCP);
	exit: if (listen_socket != -1) {
		shutdown(listen_socket, 0);
		close(listen_socket);
		printf("TCP Socket server is closed.\r\n");
	}
	s_iperf_ctrl.finish = true;
	return ret;
}

static int iperf_run_tcp_client(void) {
	int client_socket = -1;
	int err = 0;
	int ret = 0;
	struct sockaddr_storage dest_addr = { 0 };
#if LWIP_IPV6
    struct sockaddr_in6 dest_addr6 = { 0 };
#endif
	struct sockaddr_in dest_addr4 = { 0 };

	if (!(s_iperf_ctrl.cfg.type == IPERF_IP_TYPE_IPV6
			|| s_iperf_ctrl.cfg.type == IPERF_IP_TYPE_IPV4)) {
		printf("Invalid AF types\r\n");
		ret = 1;
		goto exit;
	}

#if LWIP_IPV6
    if (s_iperf_ctrl.cfg.type == IPERF_IP_TYPE_IPV6) {
        client_socket = socket(AF_INET6, SOCK_STREAM, IPPROTO_IPV6);
        ESP_GOTO_ON_FALSE((client_socket >= 0), 1, exit, TAG, "Unable to create socket: errno %d", errno);

        inet6_aton(s_iperf_ctrl.cfg.destination_ip6, &dest_addr6.sin6_addr);
        dest_addr6.sin6_family = AF_INET6;
        dest_addr6.sin6_port = htons(s_iperf_ctrl.cfg.dport);

        err = connect(client_socket, (struct sockaddr *)&dest_addr6, sizeof(struct sockaddr_in6));
        if (!(err == 0)){
        	printf("Socket unable to connect: errno %d\r\n", errno);
        	ret = 1;
        	goto exit;
        }
        printf("Successfully connected\r\n");
        memcpy(&dest_addr, &dest_addr6, sizeof(dest_addr6));
    } else
#endif
	if (s_iperf_ctrl.cfg.type == IPERF_IP_TYPE_IPV4) {
		client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (!(client_socket >= 0)) {
			printf("Unable to create socket: errno %d\r\n", errno);
			ret = 1;
			goto exit;
		}

		dest_addr4.sin_family = AF_INET;
		dest_addr4.sin_port = htons(s_iperf_ctrl.cfg.dport);
		dest_addr4.sin_addr.s_addr = s_iperf_ctrl.cfg.destination_ip4;
		err = connect(client_socket, (struct sockaddr* )&dest_addr4,
				sizeof(struct sockaddr_in));
		if (!(err == 0)) {
			printf("Socket unable to connect: errno %d\r\n", errno);
			ret = 1;
			goto exit;
		}
		printf("Successfully connected\r\n");
		memcpy(&dest_addr, &dest_addr4, sizeof(dest_addr4));
	}

	socket_send(client_socket, dest_addr, IPERF_TRANS_TYPE_TCP);
	exit: if (client_socket != -1) {
		shutdown(client_socket, 0);
		close(client_socket);
		printf("TCP Socket client is closed.\r\n");
	}
	s_iperf_ctrl.finish = true;
	return ret;
}

static int iperf_run_udp_server(void) {
	int listen_socket = -1;
	int opt = 1;
	int err = 0;
	int ret = 0;
	struct timeval timeout = { 0 };
	struct sockaddr_storage listen_addr = { 0 };
#if LWIP_IPV6
    struct sockaddr_in6 listen_addr6 = { 0 };
#endif
	struct sockaddr_in listen_addr4 = { 0 };

	if (!(s_iperf_ctrl.cfg.type == IPERF_IP_TYPE_IPV6
			|| s_iperf_ctrl.cfg.type == IPERF_IP_TYPE_IPV4)) {
		printf("Ivalid AF types\r\n");
		ret = 1;
		goto exit;
	}

#if LWIP_IPV6
    if (s_iperf_ctrl.cfg.type == IPERF_IP_TYPE_IPV6) {
        // The UDP server listen at the address "::", which means all addresses can be listened to.
        inet6_aton("::", &listen_addr6.sin6_addr);
        listen_addr6.sin6_family = AF_INET6;
        listen_addr6.sin6_port = htons(s_iperf_ctrl.cfg.sport);

        listen_socket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
        if (!(listen_socket >= 0)){
        	printf("Unable to create socket: errno %d\r\n", errno);
        	ret = 1;
        	goto exit;
        }
        printf( "Socket created\r\n");

        setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        err = bind(listen_socket, (struct sockaddr *)&listen_addr6, sizeof(struct sockaddr_in6));
        if (!(err == 0)){
        	printf("Socket unable to bind: errno %d", errno);
        	ret = 1;
        	goto exit;
        }
        printf( "Socket bound, port %d\r\n", listen_addr6.sin6_port);

        memcpy(&listen_addr, &listen_addr6, sizeof(listen_addr6));
    } else
#endif
	if (s_iperf_ctrl.cfg.type == IPERF_IP_TYPE_IPV4) {
		listen_addr4.sin_family = AF_INET;
		listen_addr4.sin_port = htons(s_iperf_ctrl.cfg.sport);
		listen_addr4.sin_addr.s_addr = s_iperf_ctrl.cfg.source_ip4;

		listen_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (!(listen_socket >= 0)) {
			printf("Unable to create socket: errno %d\r\n", errno);
			ret = 1;
			goto exit;
		}
		printf("Socket created\r\n");

		setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

		err = bind(listen_socket, (struct sockaddr* )&listen_addr4,
				sizeof(struct sockaddr_in));
		if (!(err == 0)) {
			printf("Socket unable to bind: errno %d\r\n", errno);
			ret = 1;
			goto exit;
		}
		printf("Socket bound, port %d\r\n", listen_addr4.sin_port);
		memcpy(&listen_addr, &listen_addr4, sizeof(listen_addr4));
	}

	timeout.tv_sec = IPERF_SOCKET_RX_TIMEOUT;
	setsockopt(listen_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout,
			sizeof(timeout));

	socket_recv(listen_socket, listen_addr, IPERF_TRANS_TYPE_UDP);
	exit: if (listen_socket != -1) {
		shutdown(listen_socket, 0);
		close(listen_socket);
	}
	printf("Udp socket server is closed.\r\n");
	s_iperf_ctrl.finish = true;
	return ret;
}

static int iperf_run_udp_client(void) {
	int client_socket = -1;
	int opt = 1;
	int ret = 0;
	struct sockaddr_storage dest_addr = { 0 };
#if LWIP_IPV6
    struct sockaddr_in6 dest_addr6 = { 0 };
#endif
	struct sockaddr_in dest_addr4 = { 0 };

	if (!(s_iperf_ctrl.cfg.type == IPERF_IP_TYPE_IPV6
			|| s_iperf_ctrl.cfg.type == IPERF_IP_TYPE_IPV4)) {
		printf("Ivalid AF types\r\n");
		ret = 1;
		goto exit;
	}

#if LWIP_IPV6
    if (s_iperf_ctrl.cfg.type == IPERF_IP_TYPE_IPV6) {
        inet6_aton(s_iperf_ctrl.cfg.destination_ip6, &dest_addr6.sin6_addr);
        dest_addr6.sin6_family = AF_INET6;
        dest_addr6.sin6_port = htons(s_iperf_ctrl.cfg.dport);

        client_socket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_IPV6);
        if (!(client_socket >= 0)){
        	printf("Unable to create socket: errno %d\r\n", errno);
        	ret = 1;
        	goto exit;
        }
        printf( "Socket created, sending to %s:%d\r\n", s_iperf_ctrl.cfg.destination_ip6, s_iperf_ctrl.cfg.dport);

        setsockopt(client_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        memcpy(&dest_addr, &dest_addr6, sizeof(dest_addr6));
    } else
#endif
	if (s_iperf_ctrl.cfg.type == IPERF_IP_TYPE_IPV4) {
		dest_addr4.sin_family = AF_INET;
		dest_addr4.sin_port = htons(s_iperf_ctrl.cfg.dport);
		dest_addr4.sin_addr.s_addr = s_iperf_ctrl.cfg.destination_ip4;

		client_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (!(client_socket >= 0)) {
			printf("Unable to create socket: errno %d\r\n", errno);
			ret = 1;
			goto exit;
		}
//        ESP_GOTO_ON_FALSE((client_socket >= 0), 1, exit, TAG, "Unable to create socket: errno %d", errno);
		printf("Socket created, sending to %d:%d\r\n",
				s_iperf_ctrl.cfg.destination_ip4, s_iperf_ctrl.cfg.dport);

		setsockopt(client_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
		memcpy(&dest_addr, &dest_addr4, sizeof(dest_addr4));
	}

	socket_send(client_socket, dest_addr, IPERF_TRANS_TYPE_UDP);
	exit: if (client_socket != -1) {
		shutdown(client_socket, 0);
		close(client_socket);
	}
	s_iperf_ctrl.finish = true;
	printf("UDP Socket client is closed\r\n");
	return ret;
}

static void iperf_task_traffic(void *arg) {
	if (iperf_is_udp_client()) {
		iperf_run_udp_client();
	} else if (iperf_is_udp_server()) {
		iperf_run_udp_server();
	} else if (iperf_is_tcp_client()) {
		iperf_run_tcp_client();
	} else {
		iperf_run_tcp_server();
	}

	if (s_iperf_ctrl.buffer) {
		free(s_iperf_ctrl.buffer);
		s_iperf_ctrl.buffer = NULL;
	}
	printf("iperf exit\r\n");
	s_iperf_is_running = false;
	vTaskDelete(NULL);
}

static uint32_t iperf_get_buffer_len(void) {
	if (iperf_is_udp_client()) {
		return (s_iperf_ctrl.cfg.len_send_buf == 0 ?
				IPERF_UDP_TX_LEN : s_iperf_ctrl.cfg.len_send_buf);
	} else if (iperf_is_udp_server()) {
		return IPERF_UDP_RX_LEN;
	} else if (iperf_is_tcp_client()) {
		return (s_iperf_ctrl.cfg.len_send_buf == 0 ?
				IPERF_TCP_TX_LEN : s_iperf_ctrl.cfg.len_send_buf);
	} else {
		return IPERF_TCP_RX_LEN;
	}
	return 0;
}

int iperf_start(iperf_cfg_t *cfg) {
	BaseType_t ret;

	if (!cfg) {
		return 1;
	}

	if (s_iperf_is_running) {
		printf("iperf is running\r\n");
		return 1;
	}

	memset(&s_iperf_ctrl, 0, sizeof(s_iperf_ctrl));
	memcpy(&s_iperf_ctrl.cfg, cfg, sizeof(*cfg));
	s_iperf_is_running = true;
	s_iperf_ctrl.finish = false;
	s_iperf_ctrl.buffer_len = iperf_get_buffer_len();
	s_iperf_ctrl.buffer = (uint8_t*) malloc(s_iperf_ctrl.buffer_len);
	if (!s_iperf_ctrl.buffer) {
		printf("create buffer: not enough memory\r\n");
		return 1;
	}
	memset(s_iperf_ctrl.buffer, 0, s_iperf_ctrl.buffer_len);
	ret = xTaskCreate(iperf_task_traffic, IPERF_TRAFFIC_TASK_NAME,
			IPERF_TRAFFIC_TASK_STACK, NULL, IPERF_TRAFFIC_TASK_PRIORITY, NULL);
	if (ret != pdPASS) {
		printf("create task %s failed\r\n", IPERF_TRAFFIC_TASK_NAME);
		free(s_iperf_ctrl.buffer);
		s_iperf_ctrl.buffer = NULL;
		return 1;
	}
	return 0;
}

int iperf_stop(void) {
	if (s_iperf_is_running) {
		s_iperf_ctrl.finish = true;
	}

	while (s_iperf_is_running) {
		printf("wait current iperf to stop ...\r\n");
		vTaskDelay(pdMS_TO_TICKS(300));
	}
	return 0;
}
