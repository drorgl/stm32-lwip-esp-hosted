#include "cli_startup.h"

#include "cli.h"
#include "cli_ping.h"
#include "cli_wifi.h"
#include "cli_system.h"
#include  <errno.h>
#include  <stdio.h>
#include  <sys/stat.h>
#include  <sys/unistd.h>
//#include "usbd_cdc_if.h"
#include <cmsis_os.h>

#include "usart.h"

int _write(int file, char *data, int len) {
	if ((file != STDOUT_FILENO) && (file != STDERR_FILENO)) {
		errno = EBADF;
		return -1;
	}

	put_tx_data_with_wait(data,len);

	return len;
}

void cli_startup() {

	cli_system_register();
	cli_ping_register();
	cli_wifi_register();
	cli_initialize();

}
