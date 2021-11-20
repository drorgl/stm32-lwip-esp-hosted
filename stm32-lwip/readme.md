# lwip demo for esp-hosted

This is a basic example used for a quick test of esp-hosted.

# iperf
The code from [esp-idf iperf](https://github.com/espressif/esp-idf/tree/master/examples/common_components/iperf) was adjusted for this example.




# TODO

spi_drv.c - implement stm32 functionality
app_main.c implement stm32 functionality


# Connection
STM32 Pin	Nucleo64		ESP32 Pin	Function
PA4	        A2        		IO15	    CS0
PA5	        D13        		IO14	    SCLK
PA6	        D12        		IO12	    MISO
PA7	        D11        		IO13	    MOSI
GND	        GND        		GND	        Ground
PB4         D5        		IO2	        Handshake
PB5         D4        		IO4	        Data Ready
PA9	        D8        		EN	        ESP32 Reset

STM32 Pin	Nucleo64		ESP32S2 Pin	Function
PA4	         A2        		IO10	    CS0
PA5	        D13        		IO12	    SCLK
PA6	        D12        		IO13	    MISO
PA7	        D11        		IO11	    MOSI
GND	        GND        		GND	        Ground
PB4         D5        		IO2	        Handshake
PB5         D4        		IO4	        Data Ready
PA9	        D8        		EN	        ESP32 Reset



# TODO
commands.c
- add allocator from freertos instead of malloc which doens't work on this system
