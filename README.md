<h1>Basic OLED with rotary encoder Menu for ESP8266/ESP32</h1>
should work on any board with minor modification   i.e. io pins and interrupt useage
<p align="center"><img src="/images/menu.jpg" width="80%"/></p>

A very simple to use, cheap to build and simple to wire menu system using an oled and rotary encoder
using only 5 io pins, a very cheap 128x64 SSD1306 oled and a rotary encoder board


libraries used: Adafruit_SSD1306 and Adafruit_GFX


<pre>
 oled pins: esp8266: sda=d2, scl=d1    
              esp32: sda=21, scl=22
 oled address = 3C 
 rotary encoder pins: 
            esp8266: d5, d6, d7 (button)
            esp32: 13, 14, 15

The sketch displays a menu on the oled and when an item is selected it sets a flag and 
waits until the event is acted upon.  Max menu items on a 128x64 oled is four.

To display a menu:
    menuTitle = "Demo Menu";   
    setMenu(0,"");                   // clear any current menu items
    setMenu(0,"item0");
    setMenu(1,"item1");
This will set the menu displaying and active.

When an item is selected and clicked on the variable 'menuItemClicked' is set to the menu 
item number (between 0 and 3), your sketch can now act upon this event   
    e.g.    if (menuTitle == "Demo Menu" && menuItemClicked==0) {
Notes: When acting on the event you need to flag this has happened with: menuItemClicked=100;
    To stop a menu displaying     menuTitle = "";
</pre>

for more oled info see: https://randomnerdtutorials.com/guide-for-oled-display-with-arduino/

