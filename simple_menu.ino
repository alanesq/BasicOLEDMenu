/**************************************************************************

 This is a basic example of a menu using a rotary encoder and a OLEDs based 
 on i2c version SSD1306 for ESP8266, ESP32 or Arduino               

 Github = https://github.com/alanesq/BasicOLEDMenu

 Note: The very limit memory on an Arduino Uno proved to be a problem and you have to disable DEBUG to have enough free memory for this to work
       I suspect the GFX library is too much for a Uno really?
       
 oled pins: esp8266: sda=d2, scl=d1    
            esp32: sda=21, scl=22
            Arduino: sda=A4, scl=A5
 oled address = 3C 
 rotary encoder pins: 
            esp8266: d5, d6, d7 (button)
            esp32: 13, 14, 15
            Arduino: 2, 3, 4 (button)

 
 The sketch displays a menu on the oled and when an item is selected it sets a 
 flag and waits until the event is acted upon.  Max menu items on a 128x64 oled 
 is four.
 
 To display a menu:
        menuTitle = "Demo Menu";   
        setMenu(0,"");                   // clear any current menu items
        setMenu(0,"item0");
        setMenu(1,"item1");
 This will set the menu displaying and active.
 When an item is selected and clicked on the variable 'menuItemClicked' is set 
 to the menu item number (between 0 and 3), your sketch can now act upon this event     
        e.g.    if (menuTitle == "Demo Menu" && menuItemClicked==0) {
 Notes: When acting on the event you need to flag this has happened with: menuItemClicked=100;
        To stop a menu displaying     menuTitle = "";
 
 
 for more oled info see: https://randomnerdtutorials.com/guide-for-oled-display-with-arduino/
 

  **************************************************************************/

//#include <MemoryFree.h>                     // used to display free memory on Arduino (useful as it can be very limited)
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

bool debug=1;                               // show debug info on serial (does not work on Arduino as there is not enough memory)
const String SketchTitle = "SimpleMenu";    // Sketch Title
const String SketchVersion = "18Nov20";     // Sketch Title


// oled SSD1306 display connected to I2C (SDA, SCL pins)
  #define OLED_ADDR 0x3C                    // OLED i2c address
  #define SCREEN_WIDTH 128                  // OLED display width, in pixels
  #define SCREEN_HEIGHT 64                  // OLED display height, in pixels
  #define OLED_RESET -1                     // Reset pin # (or -1 if sharing Arduino reset pin)
  Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// rotary encoder, gpio pins vary depending on board being used
  volatile int encoder0Pos = 0;             // current value selected with rotary encoder (updated in interrupt routine)
  #if defined(ESP8266)
    // esp8266
    const String boardType="ESP8266";
    #define encoder0PinA  D5
    #define encoder0PinB  D6
    #define encoder0Press D7                // button 
  #elif defined(ESP32)
    // esp32
    const String boardType="ESP32";
    #define encoder0PinA  13
    #define encoder0PinB  14
    #define encoder0Press 15                // button 
  #elif defined (__AVR_ATmega328P__)
    // Arduino Uno
    const String boardType="Arduino";
    #define encoder0PinA  2                 // this must be 2 or 3 on an Arduino Uno as interrupt used
    #define encoder0PinB  3
    #define encoder0Press 4                 // button 
  #else
    #error Unsupported board type
  #endif
  
// oled menu
  const byte menuMax = 4;                   // max number of menu items
  String menuOption[menuMax];               // options displayed in menu
  byte menuCount = 0;                       // which menu item is curently highlighted 
  int itemTrigger = 1;                      // menu item selection change trigger level
  String menuTitle = "";                    // current menu ID number (blank = none)
  byte menuItemClicked = 100;               // menu item has been clicked flag (100=none)

//// demo bitmap displaying
////    display with: display.drawBitmap((display.width() - LOGO_WIDTH ) / 2, (display.height() - LOGO_HEIGHT) / 2, logo_bmp, LOGO_WIDTH, LOGO_HEIGHT, 1);
////    utility for creating the data: http://javl.github.io/image2cpp/ or http://en.radzio.dxp.pl/bitmap_converter/
//  #define LOGO_HEIGHT   16
//  #define LOGO_WIDTH    16
//  static const unsigned char PROGMEM logo_bmp[] =
//  { B00000000, B11000000,
//    B00000001, B11000000,
//    B00000001, B11000000,
//    B00000011, B11100000,
//    B11110011, B11100000,
//    B11111110, B11111000,
//    B01111110, B11111111,
//    B00110011, B10011111,
//    B00011111, B11111100,
//    B00001101, B01110000,
//    B00011011, B10100000,
//    B00111111, B11100000,
//    B00111111, B11110000,
//    B01111100, B11110000,
//    B01110000, B01110000,
//    B00000000, B00110000 };


//  -------------------------------------------------------------------------------------------


void setup() {

  // show sketch title on serial if debug is enabled 
  if (debug) {
    Serial.begin(115200);
    Serial.println("\n\n" + SketchTitle);
    Serial.println(SketchVersion);
    //Serial.print("Mem: ");
    //Serial.println(freeMemory());         // display free memory on Arduino
  }
Serial.println(boardType);

    if (debug && boardType == "Arduino") Serial.println(F("Note: Disable debug if problems with oled"));

  // configure gpio pins
    pinMode(encoder0Press, INPUT);
    pinMode(encoder0PinA, INPUT);
    pinMode(encoder0PinB, INPUT);

  // initialise the oled display
    if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
      if (debug) Serial.println(("\nError initialising the oled display"));
    }
    
  // Display splash screen on OLED
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.print(SketchTitle);
    display.setCursor(0, 20);
    display.print(SketchVersion);
    //display.setCursor(0, 40);
    //display.print(freeMemory());
    display.display();
    delay(2000);

  // Interrupt for reading the rotary encoder
    attachInterrupt(digitalPinToInterrupt(encoder0PinA), doEncoder, CHANGE); 

  menu1();    // start the menu displaying - see menuItemActions() to customise the menus

}


//  -------------------------------------------------------------------------------------------


void loop() {

  // handle the oled menus
    if (menuTitle != "") {                                  // if a menu is active
      menuCheck();                                          // check if encoder selection button is pressed
      menuItemSelection();                                  // check for change in menu item highlighted
      staticMenu();                                         // display the menu 
      menuItemActions();                                    // act if a menu item has been clicked
    }


    
  delay(50);
}


//  -------------------------------------------------------------------------------------------

// menu action procedures - your custom menu is setup here
//   check if menu item has been selected with:  if (menuTitle == "<menu name>" && menuItemClicked==<item number 1-4>)

void menuItemActions() {
    
  if (menuItemClicked == 100) return;                                 // if no menu item has been clicked exit function


  //  --------------------- Menu 1 Actions ---------------------
  
    if (menuTitle == "Menu 1" && menuItemClicked==1) {
      menuItemClicked=100;                                            // flag that the button press has been actioned (the menu stops and waits until this)             
      int tres=enterValue("Testval", 15, 0, 30);                      // enter a value (title, start value, low limit, high limit)
      if (debug) Serial.println("Menu: Value set = " + String(tres));
    }
  
    if (menuTitle == "Menu 1" && menuItemClicked==2) {
      menuItemClicked=100;                                            
      if (debug) Serial.println(F("Menu: Menu 2 selected"));
      menu2();                                                        // show a different menu
    }
    
  //  --------------------- Menu 2 Actions ---------------------
  
    if (menuTitle == "Menu 2" && menuItemClicked==1) {
      menuItemClicked=100;                                            
      if (debug) Serial.println(F("Menu: menu off"));
      menuTitle = "";                                                 // turn menu off
      display.clearDisplay();
      display.display(); 
    }
  
    if (menuTitle == "Menu 2" && menuItemClicked==2) {
      menuItemClicked=100;                                            
      if (debug) Serial.println(F("Menu: Menu 1 selected"));
      menu1();                                                        // show first menu
    }

  //  ------------------ Combined Menu Actions ----------------
  
    if (menuItemClicked==0) {                                         // item 0 of any menu
      menuItemClicked=100; 
      if (debug) Serial.println(F("Menu: Item 0 selected on any menu"));
    }

   //  ---------------------------------------------------------

}

// menu 1
void menu1() {
    menuTitle = "Menu 1";                                        // set the menu title
    setMenu(0,"");                                               // clear all menu items
    setMenu(0,"no action");                                      // menu items (max of 4)
    setMenu(1,"enter a value");
    setMenu(2,"MENU 2");    
}

// menu 2
void menu2() {
    menuTitle = "Menu 2";  
    setMenu(0,""); 
    setMenu(0,"no action");
    setMenu(1,"menu off");
    setMenu(2,"RETURN");
}


//  -------------------------------------------------------------------------------------------
//  ------------------------------------- menu procedures -------------------------------------
//  -------------------------------------------------------------------------------------------

// set menu item
// pass: new menu items number, name         (blank iname clears all)

void setMenu(byte inum, String iname) {
  if (inum >= menuMax) return;    // invalid number
  if (iname == "") {              // clear all
    for (int i; i < menuMax; i++)  menuOption[i] = "";
    // clear the oled display
      display.clearDisplay();
      display.display(); 
    menuCount = 0;                // move highlight to top menu item
  } else {
    menuOption[inum] = iname;
    menuItemClicked = 100;        // set item selected flag as none
  }
}

//  --------------------------------------

// display menu on oled
void staticMenu() {
  display.clearDisplay(); 
  // title
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.print(menuTitle);

  // menu options
    display.setTextSize(1);
    int i=0;
    while (i < menuMax && menuOption[i] != "") {                           // if menu item is not blank display it
      if (i == menuItemClicked) display.setTextColor(BLACK,WHITE);         // if this item has been clicked
      else display.setTextColor(WHITE,BLACK);
      display.setCursor(10, 20 + (i*10));
      display.print(menuOption[i]);
      i++;
    }

  // highlighted item if none yet clicked
    if (menuItemClicked == 100) {
      display.setCursor(2, (menuCount * 10) + 20);
      display.print(">");
    }
  
  display.display();          // update display
}

//  --------------------------------------

// menu button pressed
void menuCheck() {
  // debounce the button
    if (digitalRead(encoder0Press) == HIGH) return;
    delay(40);
    if (digitalRead(encoder0Press) == HIGH) return; 
  if (menuItemClicked != 100 || menuTitle == "") return;            // item already selected or no live menu
  if (debug) Serial.println("menu '" + menuTitle + "' item " + String(menuCount) + " selected");
  menuItemClicked = menuCount;                                      // set item selected flag
  while (digitalRead(encoder0Press) == LOW);                        // wait for button release
}

//  --------------------------------------

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

//  --------------------------------------

// rotary encoder interrupt routine to update counter when turned
#if defined (__AVR_ATmega328P__)
  void doEncoder() {
#else
 ICACHE_RAM_ATTR void doEncoder() {
#endif
  if (debug) Serial.print("i");              // swow interrupt triggered on serial 
  if (digitalRead(encoder0PinA) == HIGH) {
    if (digitalRead(encoder0PinB) == LOW) encoder0Pos = encoder0Pos - 1;
    else encoder0Pos = encoder0Pos + 1;
  } else {
    if (digitalRead(encoder0PinB) == LOW ) encoder0Pos = encoder0Pos + 1;
    else encoder0Pos = encoder0Pos - 1;
  }
}

//  --------------------------------------

// enter a value using the rotary encoder
//   pass Value title, starting value, low limit , high limit
//   returns the chosen value
int enterValue(String title, int start, int low, int high) {
  // display title
    display.clearDisplay();  
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.print(title);
    display.display();                                  // update display
  int tvalue = start;
  while (digitalRead(encoder0Press) == HIGH) {          // while button is not pressed
    if (encoder0Pos > itemTrigger) {                    // encoder0Pos is updated via the interrupt procedure
      encoder0Pos = 0;
      tvalue++;
    }
    if (encoder0Pos < -itemTrigger) {
      encoder0Pos = 0;
      tvalue--;
    }  
    if (tvalue > high) tvalue=high;
    if (tvalue < low) tvalue=low;
    display.setTextSize(3);
    const int textPos = 27;                             // height of number on display
    display.fillRect(0, textPos, SCREEN_WIDTH, SCREEN_HEIGHT - textPos, BLACK);   // clear bottom half of display (128x64)
    display.setCursor(0, textPos);
    display.print(tvalue);
    // bar graph at bottom of display
      int tmag=map(tvalue, low, high, 0 ,SCREEN_WIDTH);
      display.fillRect(0, SCREEN_HEIGHT - 10, tmag, 10, WHITE);  
    display.display();                                  // update display
    delay(50);
  }
  while (digitalRead(encoder0Press) == LOW);            // wait for button release
  return tvalue;
}

//  -------------------------------------------------------------------------------------------
