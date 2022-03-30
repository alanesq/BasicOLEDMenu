<h1>Basic OLED with rotary encoder Menu for ESP8266/ESP32/Probably others</h1>
For either Arduino IDE or PlatformIO.

Note: This is a completely different sketch as of Nov21 as I decided to start again and create a version which is none blocking.  
The old version oldVersion.ino is easier to use but will stop everything else whilst waiting for user input etc. 

<table><tr>
  <td><img height='400px' src="/images/menu.jpg" /></td>
  <td><img height='400px' src="/images/project.png" /></td>
</tr></table>  

<br>A simple to use, cheap to build and simple to wire menu system using a small oled display and a rotary encoder.
I created it so I can quickly and easily add an oLed menu to any new projects, as you can see it only requires 5 wires (plus power).
<br>Uses libraries:   Adafruit_SSD1306 and Adafruit_GFX_Library
    
<br>It gives basic menus, the ability to enter a numerical value or choose from a list.
            
<br>The sketch uses the cheap 128x64 oled displays but the settings can be changed to work with different size displays.
<br>I am very impressed with these displays, they are cheap, easy to use and despite being small they are still easy to read.
<br>Note: I have seen reports of them being easily damaged by static and I have had one fail which could have been for this reason,  I think it best to be careful of this when using one on a breadboard etc, they seem ok once installed in a project.
<br>If you have any poor connections (especially to the rotary encoder) this can result in garbage being displayed which can make you think there is a fault with the display.

<br>If you are new to the esp8266 using one of these OLED displays would be a very good project for you to try.
<br>earch ebay for "SSD1306 i2c" and you should be able to pick up an oled display like the one in the picture above for around £3.  Search for "ky-040" for a rotary encoder for around £1.

<br>You can try it out on this online emulation: https://wokwi.com/arduino/projects/323967017900048980

<br>BTW - I have incorporated all this in to my basicWebserver starting point sketch which may be of interest: https://github.com/alanesq/BasicWebserver
<br>Even if you do not use BasicWebserver, this version may be easier to include in your own projects as you just have to include oled.h and add a single line to your setup and loop.

<br>for more oled info see: https://randomnerdtutorials.com/guide-for-oled-display-with-arduino/

<br>Note: I had a lot of trouble finding some code which would work reliably with a rotary encoder (i.e. the interupt section) but find this to be very good although this may not be the case with all encoders? 
<br>See these videos for more info: https://www.youtube.com/watch?v=b2uUYiGrS5Y

<br>If you have a cnc router there is a Fritzing file in the MISC folder I use for making PCBs which may be of interest.  It is in a format which is very basic but it allows me to modify it and create a PCB as quickly as possible.
<br><img height='200px' src="/misc/pcb.jpg" />


                                                           https://github.com/alanesq/BasicOLEDMen
