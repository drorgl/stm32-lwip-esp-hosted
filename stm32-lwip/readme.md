# TODO

spi_drv.c - implement stm32 functionality
app_main.c implement stm32 functionality


# Connection
STM32 Pin	ESP32 Pin	Function
PA4	                IO15	    CS0
PA5	                IO14	    SCLK
PA6	                IO12	    MISO
PA7	                IO13	    MOSI
GND	                GND	        Ground
PB4                 IO2	        Handshake
PB5                 IO4	        Data Ready
PA9	                EN	        ESP32 Reset



# TODO
commands.c
- add allocator from freertos instead of malloc which doens't work on this system
