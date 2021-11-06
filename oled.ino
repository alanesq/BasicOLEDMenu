/**************************************************************************************************
 *  
 *      OLED display simple none blocking menu System - i2c version SSD1306 - 05Nov21
 *                                   
 * 
 **************************************************************************************************
      
 The sketch displays a menu on the oled and when an item is selected it sets a 
 flag and waits until the event is acted upon.  Max menu items on a 128x64 oled 
 is four.
 
 Notes:   text size 1 = 21 x 8 characters
          text size 2 = 10 x 4

 For more oled info    see: https://randomnerdtutorials.com/guide-for-oled-display-with-arduino/

                    
 See the "menus below here" section for examples of use
 The menu is none blocking so your sketch can carry on with other tasks whilst the menu is in use.
 When a menu item is selected it is actioned in the 'menuActions()' section

 Note: If the desplay becomes corrupted it may be a bad contact on the rotary encoder.
 
  
 **************************************************************************************************/


#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


// ----------------------------------------------------------------
//                         S E T T I N G S
// ----------------------------------------------------------------


    const bool serialDebug = 1;               // show debug info on serial
    const int iLED = 22;                      // onboard LED

    //  esp32 lolin lite
    #define encoder0PinA  25                  // Rotary encoder gpio pin 
    #define encoder0PinB  33                  // Rotary encoder gpio pin 
    #define encoder0Press 32                  // Rotary encoder button gpio pin 
    #define OLEDC 26                          // oled clock pin (set to 0 for default) 
    #define OLEDD 27                          // oled data pin
    #define OLEDE 0                           // oled enable pin

//    //  esp32 HiLetGo - https://robotzero.one/heltec-wifi-kit-32/
//    #define encoder0PinA  25                  // Rotary encoder gpio pin 
//    #define encoder0PinB  33                  // Rotary encoder gpio pin 
//    #define encoder0Press 32                  // Rotary encoder button gpio pin 
//    #define OLEDC 15                          // oled clock pin (set to 0 for default) 
//    #define OLEDD 4                           // oled data pin
//    #define OLEDE 16                          // oled enable pin
    
//    // esp8266
//    #define encoder0PinA  14                  // Rotary encoder gpio pin, 14 = D5 on esp8266 
//    #define encoder0PinB  12                  // Rotary encoder gpio pin, 12 = D6 on esp8266
//    #define encoder0Press 13                  // Rotary encoder button gpio pin, 13 = D7 on esp8266
//    #define OLEDC 0                           // oled clock pin (set to 0 for default) 
//    #define OLEDD 0                           // oled data pin
//    #define OLEDE 0                           // oled enable pin

    // oLED
      #define OLED_ADDR 0x3C                    // OLED i2c address
      #define SCREEN_WIDTH 128                  // OLED display width, in pixels (usually 128)
      #define SCREEN_HEIGHT 64                  // OLED display height, in pixels (64 for larger oLEDs)
      #define OLED_RESET -1                     // Reset pin gpio (or -1 if sharing Arduino reset pin)

    // Misc
      int menuTimeout = 20;                     // menu inactivity timeout (seconds)
      const bool menuLargeText = 0;             // show larger text when possible (if struggling to read the small text)
      const int maxMenuItems = 12;              // max number of items used in any of the menus (keep as low as possible to save memory)
      const int itemTrigger = 1;                // rotary encoder - counts per tick (varies between encoders usually 1 or 2)
      const int topLine = 18;                   // y position of lower area of the display (18 with two colour displays)
      const byte lineSpace1 = 9;                // line spacing for textsize 1 (small text)
      const byte lineSpace2 = 17;               // line spacing for textsize 2 (large text)
      const int displayMaxLines = 5;            // max lines that can be displayed in lower section of display in textsize1 (5 on larger oLeds)
      const int MaxMenuTitleLength = 10;        // max characters per line when using text size 2 (usually 10)
      const bool reButtonPressedState = LOW;    // gpio pin state when the button is pressed (usually LOW)
    

// -------------------------------------------------------------------------------------------------


int demonstrationValue = 0;        // used to show how value can be changed via menu system

// forward declarations 
  void ICACHE_RAM_ATTR doEncoder();
   

// menus
  // possible modes for the menu system
  enum menuModes {                          
      off,                                  // display is off
      menu,                                 // a menu is active
      value,                                // 'enter a value' is active
      message,                              // displaying a message
      blocking                              // a blocking procedure is in progress (see enter value)
  };  
  menuModes menuMode = off;                 // default mode at startup is off
  
  String menuTitle = "";                    // title of active menu
  int noOfMenuItems = 0;                    // number if menu items in the active menu
  int selectedMenuItem = 0;                 // when a menu item is selected it is flagged here until actioned
  int highlightedMenuItem = 0;              // which menu item is curently highlighted 
  String menuItems[maxMenuItems+1];         // store for the menu item titles 
  uint32_t lastMenuActivity = 0;            // time the menu last had any activity (can be used for timeout)

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
//                                         menus below here
// -------------------------------------------------------------------------------------------------

// forward declarations for this section
  void value1();
  void demoMenu();

  
// called when the default menu is required
void defaultMenu() {
  demoMenu();
}


//                -----------------------------------------------


// demonstration menu 
void demoMenu() {
  resetMenu();                  // clear any previous menu
  menuMode = menu;              // enable menu mode
  noOfMenuItems = 8;            // set the number of items in this menu
  menuTitle = "demo_menu";      // menus title (used to identify it) 
  menuItems[1] = "item1";       // set the menu items
  menuItems[2] = "item2";
  menuItems[3] = "item3";
  menuItems[4] = "Quick menu";
  menuItems[5] = "Enter value";
  menuItems[6] = "Enter value-blocking";
  menuItems[7] = "Message";
  menuItems[8] = "Menus Off";
}  // demoMenu


// actions for menu selections are put in here
void menuActions() {
  
  // actions when an item is selected in "demo_menu"
  if (menuTitle == "demo_menu") {

    // demonstrate quickly create a menu from a list
    if (selectedMenuItem == 4) {      
      if (serialDebug) Serial.println("demo_menu demo menu from list");
      String tList[]={"main menu", "2", "3", "4", "5", "6"};
      createList("demo_list", 6, &tList[0]);  
    }

    // demonstrate usage of 'enter a value' (none blocking)
    if (selectedMenuItem == 5) {      
      if (serialDebug) Serial.println("demo_menu none blocking enter value selected");
      value1();       // enter a value 
    }

    // demonstrate usage of 'enter a value' (blocking) which is quick and easy but stops all other tasks until the value is entered
    if (selectedMenuItem == 6) {      
      if (serialDebug) Serial.println("demo_menu blocking enter a value selected");
      menuTitle = "blocking";       // title 
      mValueLow = 0;                // minimum value allowed
      mValueHigh = 100;             // maximum value allowed
      mValueStep = 1;               // step size
      mValueEntered = 50;           // starting value  
      int _entered = serviceValue(1);
      Serial.println("The value entered was " + String(_entered));   
      defaultMenu();                                 
    }     

    // demonstrate usage of message
    if (selectedMenuItem == 7) {         
      if (serialDebug) Serial.println("demo_menu message selected");
      displayMessage("Message", "Hello\nThis is a demo\nmessage.");    // 21 chars per line, "\n" = next line                              
    }      

    // turn menu/oLED off
    else if (selectedMenuItem == 8) {    
      resetMenu();    // turn menus off
    }
    
    selectedMenuItem = 0;                // clear menu item selected flag   
  }


  // actions when an item is selected in demo_list
  if (menuTitle == "demo_list") {

    // back to main menu
    if (selectedMenuItem == 1) {         
      if (serialDebug) Serial.println("demo_list back to main menu selected");
      defaultMenu();
    }

    selectedMenuItem = 0;                // clear menu item selected flag   
  }

    
}  // menuActions


//                -----------------------------------------------


// demonstration enter a value
void value1() {
  resetMenu();                  // clear any previous menu
  menuMode = value;             // enable value entry
  menuTitle = "demo_value";     // title (used to identify which number was entered)
  mValueLow = 0;                // minimum value allowed
  mValueHigh = 100;             // maximum value allowed
  mValueStep = 1;               // step size
  mValueEntered = 50;           // starting value
}


// actions for value entered put in here

void menuValues() {
  
  // action when value entered for "enter value" (none blocking)
  if (menuTitle == "demo_value") {
    String tString = String(mValueEntered);
    if (serialDebug) Serial.println("The value " + tString + " was entered");
    displayMessage("ENTERED", "\nYou entered\nthe value\n    " + tString);
    // alternatively use 'resetMenu()' here to turn menus off after value entered - or use 'defaultMenu()' to re-start the default menu
  }
  
}


// -------------------------------------------------------------------------------------------------
//                                         menus above here
// -------------------------------------------------------------------------------------------------




// ----------------------------------------------------------------
//                              -setup
// ----------------------------------------------------------------

void setup() {

  Serial.begin(115200); while (!Serial); delay(50);       // start serial comms   
  Serial.println("\n\n\n\Starting menu demo\n");

  pinMode(iLED, OUTPUT);     // onboard indicator led  

  // configure gpio pins for rotary encoder
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


  // display greeting message - pressing button will start menu 
    displayMessage("STARTED", "\nMenu system\ndemonstration\nsketch");       
  
  //defaultMenu();       // start the default menu
    
}


// ----------------------------------------------------------------
//                              -loop
// ----------------------------------------------------------------

void loop() {

  reUpdateButton();       // update rotary encoder button status

  if (menuMode == off) return;      // if menu system is turned off do nothing more

  // if menu displayed for too long then turn it off
    if ( (unsigned long)(millis() - lastMenuActivity) > (menuTimeout * 1000) ) {
      resetMenu();  
      return;
    }

  // menus
  if (menuMode == menu) {
    serviceMenu();       // run menu servicing routine
    menuActions();       // act on menu selections
  }

  // values
  if (menuMode == value) {
    serviceValue(0);      // run value entry servicing routine
    if (reButtonChanged == 1) {   
      reButtonChanged = 0;
      menuValues();      // a value has been entered
    }
  }

  // message
  if (menuMode == message) {
    if (reButtonChanged == 1) defaultMenu();       // clear message when button pressed
  }
  
}



// update rotary encoder button status
void reUpdateButton() {
  // update rotary encoder button status
    if (digitalRead(encoder0Press) != reButtonState) {       // if status has changed
      delay(50);
      if (digitalRead(encoder0Press) != reButtonState) {     // debounce
        reButtonState = digitalRead(encoder0Press);
        if (reButtonState == reButtonPressedState) {
          reButtonChanged = 1;                               // buton has been pressed
          if (menuMode == off) defaultMenu();                // start default menu if menus are off
        }
        else reButtonChanged = -1;                           // buton has been released
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
        lastMenuActivity = millis();   // log time 
      }
      if (encoder0Pos <= -itemTrigger) {
        encoder0Pos += itemTrigger;
        highlightedMenuItem--;
        lastMenuActivity = millis();   // log time 
      }      
      if (reButtonChanged == 1) {   
        reButtonChanged = 0;
        selectedMenuItem = highlightedMenuItem;     // flag that the item has been selected 
        lastMenuActivity = millis();   // log time 
        if (serialDebug) Serial.println("menu '" + menuTitle + "' item '" + menuItems[highlightedMenuItem] + "' selected");
      }  

    const int _centreLine = displayMaxLines / 2 + 1;    // mid list point
    display.clearDisplay(); 
    display.setTextColor(WHITE);   

    // verify valid highlighted item
      if (highlightedMenuItem > noOfMenuItems) highlightedMenuItem = noOfMenuItems;
      if (highlightedMenuItem < 1) highlightedMenuItem = 1;

    // title
      display.setCursor(0, 0);
      if (menuLargeText) {
        display.setTextSize(2); 
        display.println(menuItems[highlightedMenuItem].substring(0, MaxMenuTitleLength));
      } else {
        if (menuTitle.length() > MaxMenuTitleLength) display.setTextSize(1);
        else display.setTextSize(2);          
        display.println(menuTitle);
      }

    // menu
      display.setTextSize(1);
      display.setCursor(0, topLine);
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
// if _blocking set to 1 then all other tasks are stopped until a value is entered

int serviceValue(bool _blocking) {

  const int _valueSpacing = 5;      // spacing tweak for the displayed value y position
  if (_blocking) menuMode = blocking; 
  
  do {
    
    // rotary encoder
      if (encoder0Pos >= itemTrigger) {
        encoder0Pos -= itemTrigger;
        mValueEntered-= mValueStep;
        lastMenuActivity = millis();   // log time 
      }
      if (encoder0Pos <= -itemTrigger) {
        encoder0Pos += itemTrigger;
        mValueEntered+= mValueStep;
        lastMenuActivity = millis();   // log time 
      }      
      if (mValueEntered < mValueLow) {
        mValueEntered = mValueLow;
        lastMenuActivity = millis();   // log time 
      }
      if (mValueEntered > mValueHigh) {
        mValueEntered = mValueHigh;
        lastMenuActivity = millis();   // log time 
      }
  
      display.clearDisplay(); 
      display.setTextColor(WHITE); 
      
      // title
        display.setCursor(0, 0);
        if (menuTitle.length() > MaxMenuTitleLength) display.setTextSize(1);
        else display.setTextSize(2);          
        display.println(menuTitle);
        
      // value selected
        display.setCursor(10, topLine + _valueSpacing);
        display.setTextSize(3);  
        display.println(mValueEntered);  
  
      // range
        display.setCursor(0, topLine + (lineSpace1 * (displayMaxLines-1)) );
        display.setTextSize(1); 
        display.println(String(mValueLow) + " to " + String(mValueHigh));
  
      display.display();  

      reUpdateButton();        // check status of button

  } while (_blocking && reButtonChanged != 1);        // if in blocking mode repeat until button is pressed

  if (_blocking) menuMode = off;
   
  return mValueEntered;        // used when in blocking mode
          
}



// ----------------------------------------------------------------
//                           -list create
// ----------------------------------------------------------------
// create a menu from a list 
// e.g.       String tList[]={"main menu", "2", "3", "4", "5", "6"};
//            createList("demo_list", 6, &tList[0]);  

void createList(String _title, int _noOfElements, String *_list) {
  resetMenu();                      // clear any previous menu
  menuMode = menu;                  // enable menu mode
  noOfMenuItems = _noOfElements;    // set the number of items in this menu
  menuTitle = _title;               // menus title (used to identify it) 

  for (int i=1; i <= _noOfElements; i++) {
    menuItems[i] = _list[i-1];        // set the menu items  
  }
}


// ----------------------------------------------------------------
//                         -message display 
// ----------------------------------------------------------------
// 21 characters per line, use "\n" for next line
// assistant:  <     line 1        ><     line 2        ><     line 3        ><     line 4         >  

 void displayMessage(String _title, String _message) {
  resetMenu();
  menuMode = message;

  display.clearDisplay();    
  display.setTextColor(WHITE);

  // title
    display.setCursor(0, 0);
    if (menuLargeText) {
      display.setTextSize(2); 
      display.println(_title.substring(0, MaxMenuTitleLength));
    } else {
      if (_title.length() > MaxMenuTitleLength) display.setTextSize(1);
      else display.setTextSize(2);          
      display.println(_title);
    }

  // message
    display.setCursor(0, topLine);
    display.setTextSize(1);
    display.println(_message);
  
  display.display();     
  
 }


// ----------------------------------------------------------------
//                        -reset menu system
// ----------------------------------------------------------------

void resetMenu() {
  // reset all menu variables / flags
    menuMode = off;
    selectedMenuItem = 0;
    encoder0Pos = 0;
    noOfMenuItems = 0;
    menuTitle = "";
    highlightedMenuItem = 0;
    reButtonChanged = 0;
    mValueEntered = 0;
  
  lastMenuActivity = millis();   // log time 
  
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
