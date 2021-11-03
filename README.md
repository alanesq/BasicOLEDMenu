<h1>Basic OLED with rotary encoder Menu for ESP8266/ESP32/Arduino</h1>
Note: The limited memory on an Arduino Uno proved to be a problem and you have to disable DEBUG to have enough free memory for this to work

This is a completely different version as of Nov21 as I decided to start again and create a version which is none blocking.  The files you require are oled.h and oled.ino.

<p align="center"><img src="/images/menu.jpg" width="80%"/></p>

<pre>
A very simple to use, cheap to build and simple to wire menu system using an oled and rotary encoder
using only 5 gpio pins, a very cheap 128x64 SSD1306 oled and a rotary encoder board

Uses libraries:   Adafruit_SSD1306 and Adafruit_GFX_Library
    
It gives basic menus, the ability to enter a numerical value or choose from a list of options.
            
The sketch uses the cheap 128x64 oled displays but the settings can be changed to work with different size displays.

If you are new to the esp8266 using one of these OLED displays would be a very interesting and cheap introduction to using them.
If you search eBay for "esp8266 nodemcu ch340g v3" you should be able to pick up an esp8266 module for around £3.
Search for "SSD1306 i2c" and you should be able to pick up an oled display like the one in the picture above for around £3.
Search for "ky-040" for a rotary encoder for around £1.
</pre>

BTW - I have incorporated all this in to my basicWebserver starting point sketch which may be of interest: https://github.com/alanesq/BasicWebserver

for more oled info see: https://randomnerdtutorials.com/guide-for-oled-display-with-arduino/

I have a large gap at the top for use on the two colour oled displays (which tend to be cheaper ;-)


                                                                                  https://github.com/alanesq/BasicOLEDMenu
