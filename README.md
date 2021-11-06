<h1>Basic OLED with rotary encoder Menu for ESP8266/ESP32/Probably others</h1>

This is a completely new/different sketch as of Nov21 as I decided to start again and create a version which is none blocking.  
The old version oldVersion.ino is easier to use but will stop everything else whilst waiting for user input etc. 

<p align="center"><img src="/images/menu.jpg" width="80%"/></p>

<pre>
A very simple to use, cheap to build and simple to wire menu system using an oled and rotary encoder.
I created it so I can quickly and easily add an oLed menu system to any new projects.

Uses libraries:   Adafruit_SSD1306 and Adafruit_GFX_Library
    
It gives basic menus, the ability to enter a numerical value or choose from a list.
An example is included showing how a value can be changed or displayed using this menu system.
            
The sketch uses the cheap 128x64 oled displays but the settings can be changed to work with different 
size displays.

If you are new to the esp8266 using one of these OLED displays would be a very good project for you 
to try.
Search ebay for "SSD1306 i2c" and you should be able to pick up an oled display like the one in the 
picture above for around £3.  Search for "ky-040" for a rotary encoder for around £1.
My favourite development board is the esp32 lolin lite which can befound on eBay for around £5.
</pre>

BTW - I have incorporated all this in to my basicWebserver starting point sketch which may be of 
interest: https://github.com/alanesq/BasicWebserver
Even if you do not use BasicWebserver, this version may be easier to include in your own projects 
as you just have to include oled.h and add a single line to your setup and loop.

for more oled info see: https://randomnerdtutorials.com/guide-for-oled-display-with-arduino/

                                                           https://github.com/alanesq/BasicOLEDMen
