<h1>Basic OLED with rotary encoder Menu for ESP8266/ESP32/Probably others</h1>

This is a completely new/different sketch as of Nov21 as I decided to start again and create a version which is none blocking.  
The old version oldVersion.ino is easier to use but will stop everything else whilst waiting for user input etc. 
This will compile in either Arduino IDE or PlatformIO.

<p align="center"><img src="/images/menu.jpg" width="80%"/></p>

<br>A very simple to use, cheap to build and simple to wire menu system using an oled and rotary encoder.
I created it so I can quickly and easily add an oLed menu system to any new projects.
<br>Uses libraries:   Adafruit_SSD1306 and Adafruit_GFX_Library
    
<br><br>It gives basic menus, the ability to enter a numerical value or choose from a list.
<br>An example is included showing how a value can be changed or displayed using this menu system.
            
<br><br>The sketch uses the cheap 128x64 oled displays but the settings can be changed to work with different 
<br>size displays.
<br>I am very impressed with these displays, they are cheap, easy to use and despite being small very easy to read.
<br>I have seen reports of them being easily damaged by static but not experienced this myself.  If you have any 
<br>poor connections (especially to the rotary encoder) this can result in rubbish being displayed which can 
<br>make you think there is a fault with the display.

<br><br>If you are new to the esp8266 using one of these OLED displays would be a very good project for you to try.
<br>Search ebay for "SSD1306 i2c" and you should be able to pick up an oled display like the one in the 
<br>picture above for around £3.  Search for "ky-040" for a rotary encoder for around £1.
<br>My favourite development board is the esp32 lolin lite which can befound on eBay for around £5.

<br><br>Although not working yet as apparently there is an issue with the oled library and the latest esp32 core
<br>I have a copy of this sketch on the online simulator here:   https://wokwi.com/arduino/projects/319868062877614675

<br><br>BTW - I have incorporated all this in to my basicWebserver starting point sketch which may be of 
interest: https://github.com/alanesq/BasicWebserver
<br>Even if you do not use BasicWebserver, this version may be easier to include in your own projects 
<br>as you just have to include oled.h and add a single line to your setup and loop.

<br>for more oled info see: https://randomnerdtutorials.com/guide-for-oled-display-with-arduino/

<br><br>Note: I had a lot of trouble finding some code which would work reliably with a rotary encoder 
<br>(i.e. the interupt section) but find this to be very good although this may not be the case with 
<br>all encoders?
<br>see these videos for more info: https://www.youtube.com/watch?v=b2uUYiGrS5Y
<br>If you start getting all sorts of garbled data on the oled it may be surprisingly an issue with 
<br>bad connections on the rotary encoder.

                                                           https://github.com/alanesq/BasicOLEDMen
