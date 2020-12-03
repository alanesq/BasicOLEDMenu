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

const String stitle = "simple_menu";         // script title
const String sversion = "03Dec20";           // script version

int OLEDDisplayTimeout = 5;                  // timeout on the oled display (seconds)

//#include <MemoryFree.h>                     // used to display free memory on Arduino (useful as it can be very limited)
const bool oledDebug=1;                      // debug enable on serial for oled.h
  
  //#include <MemoryFree.h>                    // used to display free memory on Arduino (useful as it can be very limited)
  #include <Adafruit_GFX.h>
  #include <Adafruit_SSD1306.h>
  
  // rotary encoder, gpio pins vary depending on board being used
    volatile int encoder0Pos = 0;             // current value selected with rotary encoder (updated in interrupt routine)
    bool reButtonState = 0;                   // current debounced state of the button
    uint32_t reButtonTimer = millis();        // time button state last changed
    int reButtonMinTime = 500;                // minimum milliseconds between allowed button status changes
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
    #else
      #error Unsupported board - must be esp32, esp8266 or Arduino Uno
    #endif
  
  // oled menu
    const byte menuMax = 5;                   // max number of menu items
    const byte lineSpace1 = 10;               // line spacing
    const byte lineSpace2 = 12;               // line spacing
    String menuOption[menuMax];               // options displayed in menu
    byte menuCount = 0;                       // which menu item is curently highlighted 
    int itemTrigger = 1;                      // menu item selection change trigger level
    String menuTitle = "";                    // current menu ID number (blank = none)
    byte menuItemClicked = 100;               // menu item has been clicked flag (100=none)
    uint32_t lastREActivity = millis();       // time last activity was seen on rotary encoder
  
  // forward declarations
    void doEncoder();
    void setMenu(byte, String);
    int enterValue(String, int, int, int, int);
    void menuItemSelection();
    bool menuCheck();
    void staticMenu();
    void oledLineText(byte, bool, String);
    int chooseFromList(byte, String, String[]);
    int chooseFromListDisplayList (byte, int, String[]);
    void reWaitKeypress(int);
    
  
  // oled SSD1306 display connected to I2C (SDA, SCL pins)
    #define OLED_ADDR 0x3C                    // OLED i2c address
    #define SCREEN_WIDTH 128                  // OLED display width, in pixels
    #define SCREEN_HEIGHT 64                  // OLED display height, in pixels
    #define OLED_RESET -1                     // Reset pin # (or -1 if sharing Arduino reset pin)
    Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);



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

  // configure gpio pins
    pinMode(encoder0Press, INPUT);
    pinMode(encoder0PinA, INPUT);
    pinMode(encoder0PinB, INPUT);

  // initialise the oled display
    if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
      if (oledDebug) Serial.println(("\nError initialising the oled display"));
    }
    
  // Display splash screen on OLED
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.print(stitle);
    display.setCursor(0, lineSpace1 * 1);
    display.print(sversion);
    display.setCursor(0, lineSpace1 * 2);
    display.print(boardType);
    display.setCursor(0, lineSpace1 * 3);
    //display.print(freeMemory());
    display.display();
    // delay(2000);


  // Interrupt for reading the rotary encoder position
    attachInterrupt(digitalPinToInterrupt(encoder0PinA), doEncoder, CHANGE); 

  Main_Menu();    // start the menu displaying - see menuItemActions() to alter the menus

}


//  -------------------------------------------------------------------------------------------


void loop() {

    // if a oled menu is active service it 
    if (menuTitle != "") {                                  // if a menu is active
      menuCheck();                                          // check if encoder selection button is pressed
      menuItemSelection();                                  // check for change in menu item highlighted
      staticMenu();                                         // display the menu 
      menuItemActions();                                    // act if a menu item has been clicked
    } 


  
  //     <<< your code here >>>>
  
    
  delay(50);
}



// -------------------------------------------------------------------------------------------------
//                                        customise the menus below
// -------------------------------------------------------------------------------------------------

// Useful commands:
//      void reWaitKeypress(20000);         = wait for the button to be pressed on the rotary encoder (timeout in 20 seconds if not)
//      chooseFromList(8, "TestList", q);   = choose from the list of 8 items in a string array 'q'
//      enterValue("Testval", 15, 0, 30);   = enter a value between 0 and 30 (with a starting value of 15)

  
// Available Menus

  // main menu
  void Main_Menu() {
      menuTitle = "Main Menu";                                        // set the menu title
      setMenu(0,"");                                               // clear all menu items
      setMenu(0,"list");                                           // choose from a list
      setMenu(1,"enter a value");                                  // enter a value
      setMenu(2,"message");                                        // display a message
      setMenu(3,"MENU 2");                                         // change menu
  }

  
  // menu 2
  void menu2() {
      menuTitle = "Menu 2";  
      setMenu(0,""); 
      setMenu(0,"menu off");
      setMenu(1,"RETURN");
  }

   
// -------------------------------------------------------------------------------------------------



// menu action procedures
//   check if menu item has been selected with:  if (menuTitle == "<menu name>" && menuItemClicked==<item number 1-4>)

void menuItemActions() {
    
  if (menuItemClicked == 100) return;                                 // if no menu item has been clicked exit function



  //  --------------------- Main Menu Actions ------------------

    if (menuTitle == "Main Menu" && menuItemClicked==0) {
      menuItemClicked=100;                                            // flag that the button press has been actioned (the menu stops and waits until this)             
      String q[] = {"item0","item1","item2","item3","item4","item5","item6","item7"};
      int tres=chooseFromList(8, "TestList", q);
      Serial.println("Menu: item " + String(tres) + " chosen from list");
    }
    
    if (menuTitle == "Main Menu" && menuItemClicked==1) {
      menuItemClicked=100;                                            // flag that the button press has been actioned (the menu stops and waits until this)             
      int tres=enterValue("Testval", 1, 50, 0, 100);                  // enter a value (title, start value, step size, low limit, high limit)
      Serial.println("Menu: Value set = " + String(tres));
    }
  
    if (menuTitle == "Main Menu" && menuItemClicked==2) {
      menuItemClicked=100;                                            // flag that the button press has been actioned (the menu stops and waits until this)             
      Serial.println("Menu: display message selected");
      // display a message
        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(WHITE);
        display.setCursor(20, 20);
        display.print("Hello");
        display.display();
      reWaitKeypress(20000);                                          // wait for key press on rotary encoder
    }

    if (menuTitle == "Main Menu" && menuItemClicked==3) {
      menuItemClicked=100;                                            
      Serial.println("Menu: Menu 2 selected");
      menu2();                                                        // show a different menu
    }

    
  //  --------------------- Menu 2 Actions ---------------------
  
    if (menuTitle == "Menu 2" && menuItemClicked==0) {
      menuItemClicked=100;                                            
      Serial.println("Menu: menu off");
      menuTitle = "";                                                 // turn menu off
      display.clearDisplay();
      display.display(); 
    }
  
    if (menuTitle == "Menu 2" && menuItemClicked==1) {
      menuItemClicked=100;                                            
      Serial.println("Menu: Main Menu selected");
      Main_Menu();                                                    // show main menu
    }

}


// -------------------------------------------------------------------------------------------------
//                                        customise the menus above
// -------------------------------------------------------------------------------------------------


   
//  -------------------------------------------------------------------------------------------
//  ------------------------------------- menu procedures -------------------------------------
//  -------------------------------------------------------------------------------------------

// set menu item
// pass: new menu items number, name         (blank iname clears all entries)

void setMenu(byte inum, String iname) {
  if (inum >= menuMax) return;    // invalid number
  if (iname == "") {              // clear all menu items
    for (int i; i < menuMax; i++)  menuOption[i] = "";
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
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(lineSpace1, 0);
    display.print(menuTitle);
    display.drawLine(0, lineSpace1, display.width(), lineSpace1, WHITE);

  // menu options
    int i=0;
    while (i < menuMax && menuOption[i] != "") {                           // if menu item is not blank display it
      if (i == menuItemClicked) display.setTextColor(BLACK,WHITE);         // if this item has been clicked
      else display.setTextColor(WHITE,BLACK);
      display.setCursor(10, 12 + (i*lineSpace1));
      display.print(menuOption[i]);
      i++;
    }

  // highlighted item if none yet clicked
    if (menuItemClicked == 100) {
      display.setCursor(2, 12 + (menuCount * lineSpace1));
      display.print(">");
    }
 
  display.display();          // update display
}

//  --------------------------------------

// rotary encoder button

bool menuCheck() {
  if (digitalRead(encoder0Press) == reButtonState) return 0;    // no change
  delay(40);
  if (digitalRead(encoder0Press) == reButtonState) return 0;    // debounce
  if (millis() - reButtonTimer  < reButtonMinTime) return 0;    // if too soon since last change

  // button status has changed
  reButtonState = !reButtonState;   
  reButtonTimer = millis();                                     // update timer

  // oled menu action on button press 
  if (reButtonState==LOW) {                                     // if button is now pressed      
    lastREActivity = millis();                                  // log time last activity seen (don't count button release as activity)
    if (menuItemClicked != 100 || menuTitle == "") return 1;    // menu item already selected or there is no live menu
    Serial.println("menu '" + menuTitle + "' item " + String(menuCount) + " selected");
    menuItemClicked = menuCount;                                // set item selected flag
  }
  return 1;
}


//  --------------------------------------


// wait for key press on rotary encoder
//    pass timeout in ms
void reWaitKeypress(int timeout) {          
        uint32_t tTimer = millis();   // log time
        // wait for button to be released
          while ( (digitalRead(encoder0Press) == LOW) && (millis() - tTimer < timeout) ) {        // wait for button release
            yield();                  // service any web page requests
            delay(20);
          }
        // clear rotary encoder position counter
          noInterrupts();
            encoder0Pos = 0;       
          interrupts();
        // wait for button to be pressed or encoder to be turned
          while ( (digitalRead(encoder0Press) == HIGH) && (encoder0Pos == 0) && (millis() - tTimer < timeout) ) {
            yield();                  // service any web page requests
            delay(20);
          }
}


//  --------------------------------------

// handle menu item selection

void menuItemSelection() {
    if (encoder0Pos >= itemTrigger) {
      noInterrupts();
        encoder0Pos = 0;
      interrupts();
      lastREActivity = millis();                            // log time last activity seen
      if (menuCount+1 < menuMax) menuCount++;               // if not past max menu items move
      if (menuOption[menuCount] == "") menuCount--;         // if menu item is blank move back
    }
    if (encoder0Pos <= -itemTrigger) {
      noInterrupts();
        encoder0Pos = 0;
      interrupts();
      lastREActivity = millis();                            // log time last activity seen
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
  if (digitalRead(encoder0PinA) == HIGH) {
    if (digitalRead(encoder0PinB) == LOW) encoder0Pos -= 1;
    else encoder0Pos += 1;
  } else {
    if (digitalRead(encoder0PinB) == LOW ) encoder0Pos += 1;
    else encoder0Pos -= 1;
  }
}

//  --------------------------------------

// enter a value using the rotary encoder
//   pass Value title, starting value, step size, low limit , high limit
//   returns the chosen value
int enterValue(String title, int start, int stepSize, int low, int high) {
  uint32_t tTimer = millis();                          // log time of start of function
  // display title
    display.clearDisplay();  
    if (title.length() > 8) display.setTextSize(1);  // if title is longer than 8 chars make text smaller
    else display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.print(title);
    display.display();                                  // update display
  int tvalue = start;
  while ( (digitalRead(encoder0Press) == LOW) && (millis() - tTimer < (OLEDDisplayTimeout * 1000)) ) delay(5);    // wait for button release
  tTimer = millis();
  while ( (digitalRead(encoder0Press) == HIGH) && (millis() - tTimer < (OLEDDisplayTimeout * 1000)) ) {   // while button is not pressed and still within time limit
    if (encoder0Pos >= itemTrigger) {                    // encoder0Pos is updated via the interrupt procedure
      tvalue -= stepSize;
      noInterrupts();                                   // stop interrupt changing the value whilst it is changed here
        encoder0Pos -= itemTrigger;
      interrupts();
      tTimer = millis(); 
    } 
    else if (encoder0Pos <= -itemTrigger) {
      tvalue += stepSize;
      noInterrupts();
        encoder0Pos += itemTrigger;
      interrupts();
      tTimer = millis();
    }  
    // value limits
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
    yield();                                            // service any web page requests
  }
  return tvalue;
}

//  --------------------------------------
        
// choose from list using rotary encoder
//  pass the number of items in list (max 8), list title, list of options in a string array

int chooseFromList(byte noOfElements, String listTitle, String list[]) {

  const byte noList = 10;                                // max number of items to list
  uint32_t tTimer = millis();                            // log time of start of function
  int highlightedItem = 0;                               // which item in list is highlighted
  int xpos, ypos;
  
  // display title
    display.clearDisplay();  
    display.setTextSize(1);
    display.setTextColor(WHITE,BLACK);
    display.setCursor(10, 0);
    display.print(listTitle);
    display.drawLine(0, lineSpace1, display.width(), lineSpace1, WHITE);

  // scroll through list
    while ( (digitalRead(encoder0Press) == LOW) && (millis() - tTimer < (OLEDDisplayTimeout * 1000)) ) delay(5);    // wait for button release
    tTimer = millis();
    while ( (digitalRead(encoder0Press) == HIGH) && (millis() - tTimer < (OLEDDisplayTimeout * 1000)) ) {   // while button is not pressed and still within time limit
      if (encoder0Pos >= itemTrigger) {                    // encoder0Pos is updated via the interrupt procedure
        noInterrupts();
          encoder0Pos = 0;
        interrupts();
        highlightedItem++;
        tTimer = millis(); 
      }
      if (encoder0Pos <= -itemTrigger) {
        noInterrupts();
          encoder0Pos = 0;
        interrupts();
        highlightedItem--;
        tTimer = millis();
      }  
      // value limits
        if (highlightedItem > noOfElements - 1) highlightedItem = noOfElements - 1;        
        if (highlightedItem < 0) highlightedItem = 0;
      // display the list
        for (int i=0; i < noOfElements; i++) {
            if (i < (noList/2)) {
              xpos = 0; 
              ypos = lineSpace1 * (i+1) + 2;
            } else {
              xpos = display.width() / 2;
              ypos = lineSpace1 * (i-((noList/2)-1)) + 2;
            }
            display.setCursor(xpos, ypos);
            if (i == highlightedItem) display.setTextColor(BLACK,WHITE);
            else display.setTextColor(WHITE,BLACK);
            display.print(list[i]);
        }
      display.display();                                    // update display
      yield();                                              // service any web page requests
    }
  
  // if it timed out set selection to cancel (i.e. item 0)
    if (millis() - tTimer >= (OLEDDisplayTimeout * 1000)) highlightedItem=0;      

//  // wait for button to be released (up to 1 second)
//    tTimer = millis();                         // log time
//    while ( (digitalRead(encoder0Press) == LOW) && (millis() - tTimer < 1000) ) {
//      yield();        // service any web page requests
//      delay(20);
//    }

    return highlightedItem;
}

// ---------------------------------------------- end ----------------------------------------------
