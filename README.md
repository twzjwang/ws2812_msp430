	Function    :  control ws2812b led strip with msp430f5529lp
	IDE         :  IAR
	Connections :  LED <=> msp	
                 +5v     3V3
                 GND     GND
                 Din     P1.2 (default)
	Instructions:  1. set RGB info with "set_RGB(int led, char R, char G, char B)"
	               2. call "send_RGB()" to transmit data
	               3. use "delay_m(ms)" between different mode
	Reference   :  ws2812b document
	               https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf
