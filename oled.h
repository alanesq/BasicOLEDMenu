/**************************************************************************************************
 *  
 *      OLED display simple none blocking menu System - i2c version SSD1306 - 03Nov21
 * 
 *      part of the BasicWebserver sketch
 *                                   
 * 
 **************************************************************************************************
      
 The sketch displays a menu on the oled and when an item is selected it sets a 
 flag and waits until the event is acted upon.  Max menu items on a 128x64 oled 
 is four.
 
 Notes:   text size 1 = 21 x 8 characters
          text size 2 = 10 x 4

 for more oled info    see: https://randomnerdtutorials.com/guide-for-oled-display-with-arduino/
                    

 
 **************************************************************************************************/


#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


// ----------------------------------------------------------------
//                         S E T T I N G S
// ----------------------------------------------------------------


    //  esp32 lolin lite
    #define encoder0PinA  25                  // Rotary encoder gpio pin 
    #define encoder0PinB  33                  // Rotary encoder gpio pin 
    #define encoder0Press 32                  // Rotary encoder button gpio pin 
    #define OLEDC 26                          // oled clock pin (set to 0 for default) 
    #define OLEDD 27                          // oled data pin

    
//    // esp8266
//    #define encoder0PinA  14                  // Rotary encoder gpio pin, 14 = D5 on esp8266 
//    #define encoder0PinB  12                  // Rotary encoder gpio pin, 12 = D6 on esp8266
//    #define encoder0Press 13                  // Rotary encoder button gpio pin, 13 = D7 on esp8266
//    #define OLEDC 0                           // oled clock pin (set to 0 for default) 
//    #define OLEDD 0                           // oled data pin


    const bool menuLargeText = 0;             // If larger text should be displayed to make reading menus easier
    const int maxMenuItems = 20;              // max number of items used in any of the menus
    int OLEDDisplayTimeout = 10;              // inactivity trigger time on rotary encoder (seconds)
    const int itemTrigger = 1;                // rotary encoder - counts per tick (varies between encoders usually 1 or 2)
    const int topLine = 18;                   // y position of second bank of menu display (18 with two colour displays)
    const byte lineSpace1 = 9;                // line spacing for textsize 1
    const byte lineSpace2 = 17;               // line spacing for textsize 2
    const int displayMaxLines = 5;            // max lines that can be displayed in lower section of display in textsize1
    const bool reButtonPressedState = 0;      // gpio pin status when button is pressed
    

// -------------------------------------------------------------------------------------------------


// forward declarations 
  void ICACHE_RAM_ATTR doEncoder();
  void resetMenu();
  void serviceMenu();
  int checkEncoder();
  void serviceValue();
  void createList(int, String[]);
  

// menus
  enum menuModes {                          // all possible modes for the menu system
      off,                                  // disable menus
      menu,                                 // live menu
      list,                                 // choose from a suplied list of options
      value                                 // enter a value
  };    
  menuModes menuMode = off;                 // default mode at startup
  String menuTitle = "";                   // title of active menu
  int noOfMenuItems = 0;                    // number if menu items in the active menu
  int selectedMenuItem = 0;                 // when a menu item is selected it is stored here until actioned
  int highlightedMenuItem = 0;              // which menu item is highlighted 
  String menuItems[maxMenuItems+1];         // store for the menu item titles 

// value entry
  int mValueEntered = 0;                    // store for number entered by value entry menu  
  int mValueLow = 0;                        // lowest possible value
  int mValueHigh = 0;                       // highest possible value
  int mValueStep = 0;                       // step size

// rotary encoder  
  volatile int encoder0Pos = 0;             // current value selected with rotary encoder (updated in interrupt routine)
  volatile bool encoderPrevA;               // used to debounced rotary encoder 
  volatile bool encoderPrevB;               // used to debounced rotary encoder 
  bool reButtonState = 1;                   // previous state of the button
  int reButtonChanged = 0;                  // set to -1 when released and 1 when pressed
  //uint32_t reButtonTimer = 0;               // time button state last changed
  //int reButtonMinTime = 500;                // minimum milliseconds between allowed button status changes

// oled SSD1306 display connected to I2C
  #define OLED_ADDR 0x3C                    // OLED i2c address
  #define SCREEN_WIDTH 128                  // OLED display width, in pixels
  #define SCREEN_HEIGHT 64                  // OLED display height, in pixels
  #define OLED_RESET -1                     // Reset pin # (or -1 if sharing Arduino reset pin)
  // #define OLED_RESET LED_BUILTIN           // I have seen this is required for esp8266?
  Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


// -------------------------------------------------------------------------------------------------
//                                        custom menus go here
// -------------------------------------------------------------------------------------------------

// forward declarations
  void value1();
  

// demonstration menu 
void demoMenu() {
  resetMenu();                  // clear any previous menu
  menuMode = menu;              // enable menu mode
  noOfMenuItems = 8;            // set the number of items in this menu
  menuTitle = "demo_menu";          // menus title (used to identify it) 
  menuItems[1] = "item1";       // set the menu items
  menuItems[2] = "item2";
  menuItems[3] = "item3";
  menuItems[4] = "item4";
  menuItems[5] = "item5";
  menuItems[6] = "item6";
  menuItems[7] = "enter value";
  menuItems[8] = "Menus Off";
}  // demoMenu

// actions for demo_menu selections are put in here
void menuActions() {
  // actions when an item is selected in "demo_menu"
    if (menuTitle == "demo_menu") {
      if (selectedMenuItem == 2) {         // demo_menu item 2 selected
        if (serialDebug) Serial.println("demo_menu second item selected");
      }      
      if (selectedMenuItem == 7) {         // enter a value
        if (serialDebug) Serial.println("demo_menu enter value selected");
        value1();       // enter a value 
      }
      else if (selectedMenuItem == 8) {    // menus off selected
        resetMenu();    // turn menus off
      }
      selectedMenuItem = 0;                // clear menu item selected flag   
    } 

  // actions when an item is selected in "demo_list"
    if (menuTitle == "demo_list") {
      if (selectedMenuItem == 1) {         // demo_list item 1 selected
        if (serialDebug) Serial.println("demo_menu first item selected");
        demoMenu();                        // enable demo_menu
      }  
      if (selectedMenuItem == 2) {  
        if (serialDebug) Serial.println("demo_menu second item selected");
      }  
      selectedMenuItem = 0;                // clear menu item selected flag  
    }
    
}  // menuActions


// -------------------------------------------------------------------------------------------------


// demo of enter a value
void value1() {
  resetMenu();                  // clear any previous menu
  menuMode = value;             // enable value entry
  menuTitle = "demo_value";    // title (used to identify which number was entered)
  mValueLow = 0;                // minimum value allowed
  mValueHigh = 100;             // maximum value allowed
  mValueStep = 1;               // step size
  mValueEntered = 50;           // starting value
}


// actions for value entered put in here
void menuValues() {
  // action when value entered for "enter value"
  if (menuTitle == "demo_value") {
    if (serialDebug) Serial.println("Value entered for 'demo_value' was " + String(mValueEntered));
    demoMenu();
  }
}


// -------------------------------------------------------------------------------------------------


// ----------------------------------------------------------------
//                              -setup
// ----------------------------------------------------------------
// called from main setup

void oledSetup() {

  // configure gpio pins
    pinMode(encoder0Press, INPUT_PULLUP);
    pinMode(encoder0PinA, INPUT);
    pinMode(encoder0PinB, INPUT);

  // initialise the oled display
    if (OLEDC == 0) Wire.begin();
    else Wire.begin(OLEDD, OLEDC);
    if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
      Serial.println(("\nError initialising the oled display"));
    }

  // Interrupt for reading the rotary encoder position
    encoder0Pos = 0;   
    attachInterrupt(digitalPinToInterrupt(encoder0PinA), doEncoder, CHANGE); 

//  // display startup message
//    display.clearDisplay();    
//    display.setTextColor(WHITE);
//    display.setTextSize(1);
//    display.setCursor(0, topLine);
//    display.println("hello");
//    display.display();    

  resetMenu();         // reset menu system
  //demoMenu();          // enable demo_menu

  // create a demonstration menu from a list
    String q[]={"main menu", "2", "3", "4", "5", "6"};
    createList(6, q);      // enable demo list
    
}


// ----------------------------------------------------------------
//                              -loop
// ----------------------------------------------------------------
// called from main loop

void oledLoop() {

  if (menuMode == off) return;      // if menu system is turned off do nothing

  // update rotary encoder button status
    if (digitalRead(encoder0Press) != reButtonState) {       // if status has changed
      delay(50);
      if (digitalRead(encoder0Press) != reButtonState) {     // debounce
        reButtonState = digitalRead(encoder0Press);
        if (reButtonState == reButtonPressedState) reButtonChanged = 1;         // buton has been pressed
        else reButtonChanged = -1;                                              // buton has been released
      }
    }

  // menus
  if (menuMode == menu) {
    serviceMenu();       // run menu servicing routine
    menuActions();       // act on menu selections
  }

  // values
  if (menuMode == value) {
    serviceValue();      // run value entry servicing routine
    if (reButtonChanged == 1) {   
      reButtonChanged = 0;
      menuValues();      // a value has been entered
    }
  }
  
}


// ----------------------------------------------------------------
//                       -service active menu
// ----------------------------------------------------------------

void serviceMenu() {

    // rotary encoder
      if (encoder0Pos >= itemTrigger) {
        encoder0Pos -= itemTrigger;
        highlightedMenuItem++;
      }
      if (encoder0Pos <= -itemTrigger) {
        encoder0Pos += itemTrigger;
        highlightedMenuItem--;
      }      
      if (reButtonChanged == 1) {   
        reButtonChanged = 0;
        selectedMenuItem = highlightedMenuItem;     // flag that the item has been selected 
        if (serialDebug) Serial.println("menu '" + menuTitle + "' item '" + menuItems[highlightedMenuItem] + "' selected");
      }  

    const int _centreLine = displayMaxLines / 2 + 1;    // mid list point
    display.clearDisplay(); 
    display.setTextColor(WHITE);   

    // verify valid highlighted item
      if (highlightedMenuItem > noOfMenuItems) highlightedMenuItem = noOfMenuItems;
      if (highlightedMenuItem < 1) highlightedMenuItem = 1;

    // title
      display.setTextSize(2);    
      display.setCursor(0, 0);
      if (menuLargeText) display.println(menuItems[highlightedMenuItem]);
      else display.println(menuTitle);

    // menu
      display.setTextSize(1);
      display.setCursor(0, lineSpace2);
      for (int i=1; i <= displayMaxLines; i++) {
        int item = highlightedMenuItem - _centreLine + i;  
        if (item == highlightedMenuItem) display.setTextColor(BLACK, WHITE);
        else display.setTextColor(WHITE);
        if (item > 0 && item <= noOfMenuItems) display.println(menuItems[item]);
        else display.println(" ");
      }
      display.display();   
}


// ----------------------------------------------------------------
//                        -service value entry
// ----------------------------------------------------------------

void serviceValue() {
  // rotary encoder
    if (encoder0Pos >= itemTrigger) {
      encoder0Pos -= itemTrigger;
      mValueEntered-= mValueStep;
    }
    if (encoder0Pos <= -itemTrigger) {
      encoder0Pos += itemTrigger;
      mValueEntered+= mValueStep;
    }      
    if (mValueEntered < mValueLow) mValueEntered = mValueLow;
    if (mValueEntered > mValueHigh) mValueEntered = mValueHigh;

    // title
      display.clearDisplay(); 
      display.setTextColor(WHITE);  
      display.setTextSize(1);    
      display.setCursor(0, 0);
      display.println(menuTitle);
      
  // display vale entry on oled
      display.setCursor(10, lineSpace2 + 10);
      display.setTextSize(3);  
      display.println(mValueEntered);
      display.display();         

}


// ----------------------------------------------------------------
//                           -list create
// ----------------------------------------------------------------
// create a menu from suplied list

void createList(int _noOfElements, String _list[]) {
  resetMenu();                      // clear any previous menu
  menuMode = menu;                  // enable menu mode
  noOfMenuItems = _noOfElements;    // set the number of items in this menu
  menuTitle = "demo_list";          // menus title (used to identify it) 

  for (int i=1; i <= _noOfElements; i++) {
    menuItems[i] = _list[i-1];        // set the menu items  
  }
}


// ----------------------------------------------------------------
//                        -reset menu system
// ----------------------------------------------------------------

void resetMenu() {
  menuMode = off;
  selectedMenuItem = 0;
  encoder0Pos = 0;
  noOfMenuItems = 0;
  menuTitle = "";
  highlightedMenuItem = 0;
  reButtonChanged = 0;
  mValueEntered = 0;
  // clear oled display
    display.clearDisplay();    
    display.display(); 
}


// ----------------------------------------------------------------
//                     -interrupt for rotary encoder
// ----------------------------------------------------------------
// rotary encoder interrupt routine to update position counter when turned
//     interrupt info: https://www.gammon.com.au/forum/bbshowpost.php?id=11488

void ICACHE_RAM_ATTR doEncoder() {
     
  bool pinA = digitalRead(encoder0PinA);
  bool pinB = digitalRead(encoder0PinB);

  if ( (encoderPrevA == pinA && encoderPrevB == pinB) ) return;  // no change since last time (i.e. reject bounce)

  // same direction (alternating between 0,1 and 1,0 in one direction or 1,1 and 0,0 in the other direction)
         if (encoderPrevA == 1 && encoderPrevB == 0 && pinA == 0 && pinB == 1) encoder0Pos -= 1;
    else if (encoderPrevA == 0 && encoderPrevB == 1 && pinA == 1 && pinB == 0) encoder0Pos -= 1;
    else if (encoderPrevA == 0 && encoderPrevB == 0 && pinA == 1 && pinB == 1) encoder0Pos += 1;
    else if (encoderPrevA == 1 && encoderPrevB == 1 && pinA == 0 && pinB == 0) encoder0Pos += 1;
    
  // change of direction
    else if (encoderPrevA == 1 && encoderPrevB == 0 && pinA == 0 && pinB == 0) encoder0Pos += 1;   
    else if (encoderPrevA == 0 && encoderPrevB == 1 && pinA == 1 && pinB == 1) encoder0Pos += 1;
    else if (encoderPrevA == 0 && encoderPrevB == 0 && pinA == 1 && pinB == 0) encoder0Pos -= 1;
    else if (encoderPrevA == 1 && encoderPrevB == 1 && pinA == 0 && pinB == 1) encoder0Pos -= 1;

    else if (serialDebug) Serial.println("Error: invalid rotary encoder pin state - prev=" + String(encoderPrevA) + "," 
                                          + String(encoderPrevB) + " new=" + String(pinA) + "," + String(pinB));
    
  // update previous readings
    encoderPrevA = pinA;
    encoderPrevB = pinB;
}


// ---------------------------------------------- end ----------------------------------------------
