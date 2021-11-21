# lwip demo for esp-hosted

This is a basic demo used for a quick test of esp-hosted.

While this POC is not stable and a bit messy, its main purpose was to find out if esp-hosted is a viable option as lwip interface. I'm releasing this project in hope that someone will pick up where I've left off since I didn't see any attempt to use the [esp-hosted](https://github.com/espressif/esp-hosted) project with lwip.

I've added a few log messages that show up as a warning which seems like a bug in the protocol that might cause wasted blocks to be transferred between the esp32 and the mcu.

Test Results with 20mbps SPI with ESP32-S2 TCP:
```
        Interval Bandwidth
   0-   3 sec       0.31 Mbits/sec
   3-   6 sec       0.35 Mbits/sec
   6-   9 sec       0.35 Mbits/sec
   9-  12 sec       0.35 Mbits/sec
  12-  15 sec       0.35 Mbits/sec
  15-  18 sec       0.35 Mbits/sec
  18-  21 sec       0.35 Mbits/sec
  21-  24 sec       0.31 Mbits/sec
  24-  27 sec       0.35 Mbits/sec
  27-  30 sec       0.35 Mbits/sec
   0-  30 sec       0.34 Mbits/sec

```

# Commands
```
help
start_lwip - start lwip stack
join <ssid> <password> - join wifi network
show_connected - show connected stations
scan - scan ap list
stop_softap - stop soft ap
disconnect_ap - disconnect ap
get_softap_config - get soft ap wifi config
get_config - get wifi config
set_mode <mode> - set wifi mode
get_mode - retrieve wifi mode
get_mac <mode> - retrieve mac address for mode
ping <ip> - ping ip
freemem - show memory info
iperf <arguments> - run iperf
```

iperf:
```
	-c <ip> 				client mode
	-s						server mode
	-u 						use udp
	-l <length> 			length
	-i <seconds interval> 	interval between reports
	-t <seconds time> 		time to transmit
```




# iperf
The code from [esp-idf iperf](https://github.com/espressif/esp-idf/tree/master/examples/common_components/iperf) was adjusted for this example.


# TODO: 
- Check if the portCLEAN_UP_TCB is needed to cleanup newlib reent
- check if PA4 can be used as hardware NSS using __HAL_SPI_ENABLE / __HAL_SPI_DISABLE - does not work, the NSS pin behaves eratically with SPI DISABLE/ENABLE
- check if the HAL_SPI_TransmitReceive at MX_SPI1_Init is still needed once the hardware NSS is usable - irrelevant
 


# Connection
```
STM32 Pin	Nucleo64		ESP32 Pin	Function
PA4             A2                      IO15            CS0
PA5             D13                     IO14            SCLK
PA6             D12                     IO12            MISO
PA7             D11                     IO13            MOSI
GND             GND                     GND             Ground
PB4             D5                      IO2             Handshake
PB5             D4                      IO4             Data Ready
PA9             D8                      EN              ESP32 Reset

STM32 Pin	Nucleo64		ESP32S2 Pin	Function
PA4             A2                      IO10            CS0
PA5             D13                     IO12            SCLK
PA6             D12                     IO13            MISO
PA7             D11                     IO11            MOSI
GND             GND                     GND             Ground
PB4             D5                      IO2             Handshake
PB5             D4                      IO4             Data Ready
PA9             D8                      EN              ESP32 Reset
```

# Memory management
For some reason the ST's ThreadSafe implementation was not enough to prevent a hard fault. wrapping malloc/free with vTaskSuspendAll/xTaskResumeAll seemed to have solved the issue. need to investigate why.

# ESP32-S2 Specifics
The esp32-s2 requires that the CS will be asserted before transfer and deasserted afterwards, since the STM32 SPI Hardware NSS does not work this way it was required to use software NSS.
In addition, one of the issues with STM32 SPI is that it should be initialized before resetting the ESP32 since the rogue SPI outputs misleads the ESP32, so a dummy transfer is started as soon as the SPI initializes.

# Setting up a different STM32 MCU
This project requires the following to setup:
- Handshake and Data Ready pins should be setup as input and interrupt, the interrupt should call osSemaphoreRelease(osSemaphore) - see HAL_GPIO_EXTI_Callback as example
- NSS as GPIO Output
- SPI configured speed below maximum supported by ESP32/ESP32S2/ESP32C3
- CMSIS 1 FreeRTOS

# License
This code is completely free, however, the following are used in this project and have their own license:
- esp-hosted - https://github.com/espressif/esp-hosted
- lwip - https://github.com/lwip-tcpip/lwip
- Dave Nadler's heap - https://github.com/DRNadler/FreeRTOS_helpers/blob/master/heap_useNewlib_ST.c
- argtable3 - https://github.com/argtable/argtable3
- microrl - https://github.com/Helius/microrl
- Tilen MAJERLE TM_BUFFER - https://github.com/MaJerle/stm32fxxx-hal-libraries/blob/master/00-STM32_LIBRARIES/tm_stm32_buffer.h
- ping_cmd - http://118.31.62.158/git/webdata/OPL1000A2-SDK/SDK/APS/middleware/third_party/lwip-2.0.3/ping_cmd.h
