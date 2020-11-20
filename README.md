<h1>Basic OLED with rotary encoder Menu for ESP8266/ESP32/Arduino</h1>
Note: The very limit memory on an Arduino Uno proved to be a problem and you have to disable DEBUG to have enough free memory for this to work
<p align="center"><img src="/images/menu.jpg" width="80%"/></p>

<pre>
A very simple to use, cheap to build and simple to wire menu system using an oled and rotary encoder
using only 5 io pins, a very cheap 128x64 SSD1306 oled and a rotary encoder board

libraries used: Adafruit_SSD1306 and Adafruit_GFX
See the "customise the menus below" section of the sketch for examples of how to use it.
It gives basic menus, the ability to enter a numerical value or choose from a list of options.

Thanks to:  https://create.arduino.cc/projecthub/yilmazyurdakul/arduino-oled-encoder-simple-menu-system-f9baa1
which I used as a starting point for this project.

 oled pins: esp8266: sda=d2, scl=d1    
            esp32: sda=21, scl=22
            Arduino: sda=A4, scl=A5
 oled address = 3C 
 rotary encoder pins: 
            esp8266: d5, d6, d7 (button)
            esp32: 13, 14, 15
            Arduino: 2, 3, 4 (button)
            
The sketch displays a menu on the oled display and when an item is selected it sets a flag and 
waits until the event is acted upon.  Max. menu items on a 128x64 oled is four.

If you are new to the esp8266 using one of these OLED displays would be a very interesting and cheap introduction to using them.
If you search eBay for "esp8266 nodemcu ch340g v3" you should be able to pick up an esp8266 module for around £3.
Search for "SSD1306 i2c" and you should be able to pick up an oled display like the one in the picture above for around £3.
Search for "ky-040" for a rotary encoder for around £1.
</pre>

BTW - I have incorporated all this in to my basicWebserver starting point sketch which may be of interest: https://github.com/alanesq/BasicWebserver

for more oled info see: https://randomnerdtutorials.com/guide-for-oled-display-with-arduino/

