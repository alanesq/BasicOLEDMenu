/**************************************************************************

 This is a basic example of a menu using a rotary encoder and a OLEDs based on SSD1306 
 
 oled pins: esp8266 = sda-d2, scl-d1    
 oled address = 3C 
 rotary encoder pins: 5, 6, 7 (button)

 
 The sketch displays a menu on the oled and when an item is selected it sets a flag and waits until
 the event is acted upon.  Max menu items on a 128x64 oled is four.
 
 To display a menu:
        menuTitle = "Demo Menu";   
        setMenu(0,"");                   // clear any current menu items
        setMenu(0,"item0");
        setMenu(1,"item1");
 This will set the menu displaying and active.
 When an item is selected and clicked on the variable 'menuItemClicked' is set to the menu item number (between 0 and 3)
 Your sketch can now act upon this event     
        e.g.    if (menuTitle == "Demo Menu" && menuItemClicked==0) {
 Notes: When acting on the event you need to flag this has happened with        menuItemClicked=100;
        To stop a menu displaying     menuTitle = "";
 
 
 for more oled info see: https://randomnerdtutorials.com/guide-for-oled-display-with-arduino/
 

  **************************************************************************/

// #include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

// oled SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_ADDR   0x3C
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// rotary encoder
#define encoder0PinA  D5
#define encoder0PinB  D6
#define encoder0Press  D7                 // button on rotary encoder
volatile int encoder0Pos = 0;             // current value selected with rotary encoder

// menu
const byte menuMax = 4;                   // max number of menu items
String menuOption[menuMax];               // options displayed in menu
byte menuCount = 0;                       // which menu item is curently highlighted 
int itemTrigger = 1;                      // menu item selection change trigger level
String menuTitle = "";                    // current menu ID number (blank = none)
byte menuItemClicked = 100;               // menu item has been clicked flag (100=none)


//  -------------------------------------------------------------------------------------------


void setup() {
  Serial.begin(115200);
  Serial.println("\n\nDemo menu sketch");

  // gpio pins
  pinMode(encoder0Press, INPUT);
  pinMode(encoder0PinA, INPUT);
  pinMode(encoder0PinB, INPUT);
  
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.display();
  display.clearDisplay();
  attachInterrupt(digitalPinToInterrupt(encoder0PinA), doEncoder, CHANGE);      // esp8266 


  // set up a demo menu
    menuTitle = "Demo Menu";         // set the menu title
    setMenu(0,"");                   // clear all menu items
    setMenu(0,"top");
    setMenu(1,"menu off");
    setMenu(2,"menu 2");
    
}


//  -------------------------------------------------------------------------------------------


void loop() {
  
  // display menu on oled
  if (menuTitle != "") {                                                // if there is a menu active
    menuCheck();                                                        // check if encoder button pressed
    menuItemSelection();                                                // check for change in menu item highlighted
    staticMenu();                                                       // display the menu on the oled
  } else {
      // clear oled display
      display.clearDisplay();
      display.display();          // update display
  }
  
  // demo use of menu
    if (menuTitle == "Demo Menu" && menuItemClicked==0) {             // if menu "Demo Menu", item 0 has been clicked
      menuItemClicked=100;                                            // clear menu item selected flag
      Serial.println("Acting on Demo Menu item 0 selection");
    }
    if (menuTitle == "Demo Menu" && menuItemClicked==1) {
      menuItemClicked=100;                                            
      Serial.println("Acting on Demo Menu item 1 selection");
      menuTitle = "";                                                 // turn menu off
    }
      if (menuTitle == "Demo Menu" && menuItemClicked==2) {
      menuItemClicked=100;                                            
      Serial.println("Acting on Demo Menu item 2 selection");
          // show a different menu
          menuTitle = "Menu 2";  
          setMenu(0,"");                   // clear all menu items
          setMenu(0,"item0");
          setMenu(1,"item1");
    }
    
  delay(100);
}


//  -------------------------------------------------------------------------------------------
//  ------------------------------------- menu procedures -------------------------------------
//  -------------------------------------------------------------------------------------------

// set menu item
// pass: new menu items number, name         (blank iname clears all)
void setMenu(byte inum, String iname) {
  if (inum >= menuMax) return;    // invalid number
  if (iname == "") {              // clear all
    for (int i; i < menuMax; i++) {
      menuOption[i] = "";
    }
    menuCount = 0;                // move highlight to top menu item
  } else {
    menuOption[inum] = iname;
    menuItemClicked = 100;        // set item selected flag as none
  }
}

// display menu on oled
void staticMenu() {
  display.clearDisplay();

  // title
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(10, 0);
  display.println(menuTitle);

  // menu options
  for (int x; x < menuMax; x++) {
    if (menuOption[x] != "") {            // if menu utem is not blank display it
      display.setTextSize(1);
      if (x == menuItemClicked) display.setTextColor(BLACK,WHITE);         // if this item has been clicked
      else display.setTextColor(WHITE,BLACK);
      display.setCursor(10, 20 + (x*10));
      display.println(menuOption[x]);
    }
  }

  // highlighted item
  if (menuItemClicked == 100) {
    display.setCursor(2, (menuCount * 10) + 20);
    display.println(">");
  }
  
  display.display();          // update display
}


// rotary encoder menu button pressed
void menuCheck() {
  // debounce the button
    if (digitalRead(encoder0Press) == HIGH) return;
    delay(40);
    if (digitalRead(encoder0Press) == HIGH) return; 
  if (menuItemClicked != 100 || menuTitle == "") return;            // item already selected or no live menu
  Serial.println("menu '" + menuTitle + "' item " + String(menuCount) + " selected");
  menuItemClicked = menuCount;                                      // set item selected flag
  while (digitalRead(encoder0Press) == LOW);                        // wait for button release
}


// handle menu item selection
void menuItemSelection() {
    if (encoder0Pos > itemTrigger) {
      encoder0Pos = 0;
      if (menuCount+1 < menuMax) menuCount++;               // if not past max menu items move
      if (menuOption[menuCount] == "") menuCount--;         // if menu item is blank move back
    }
    if (encoder0Pos < -itemTrigger) {
      encoder0Pos = 0;
      if (menuCount > 0) menuCount--;
    }
}   


// rotary encoder interrupt routine to update counter when turned
ICACHE_RAM_ATTR void doEncoder() {
  if (digitalRead(encoder0PinA) == HIGH) {
    if (digitalRead(encoder0PinB) == LOW) encoder0Pos = encoder0Pos - 1;
    else encoder0Pos = encoder0Pos + 1;
  } else {
    if (digitalRead(encoder0PinB) == LOW ) encoder0Pos = encoder0Pos + 1;
    else encoder0Pos = encoder0Pos - 1;
  }
}

//  -------------------------------------------------------------------------------------------