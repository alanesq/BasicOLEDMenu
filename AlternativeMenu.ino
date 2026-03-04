/*

        Demo menu for combined oled and rotary encoder boards - 04Mar26

        oled board pins:
            Pin, Function, ESP32 Pin (Suggested)
            1, VCC, 3.3V?
            2, GND, GND
            3, Back key, GPIO 15
            4, TRIM_B (Encoder B), GPIO 4
            5, TRIM_A (Encoder A), GPIO 16
            6, PUSH (encoder press), GPIO 17
            7, I2C_SCL-oled, GPIO 18
            8, I2C_SDA-oled, GPIO 19
            9, Confirm key, GPIO 21
            
        Notes:
          snow on screen means using wrong driver for the board (SH1106 or SSD1306)            

*/

#include <Wire.h>
#include <Adafruit_GFX.h>

// === TOGGLE DRIVER HERE ===
// Comment this out to use SSD1306; uncomment to use SH1106
#define USE_SH1106 

#define OLED_SDA 19
#define OLED_SCL 18
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C                    // OLED i2c address

#ifdef USE_SH1106
  #include <Adafruit_SH110X.h>
  Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
#else
  #include <Adafruit_SSD1306.h>
  Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
#endif

// Pins (Adjust based on your ESP32 wiring)
#define ENCODER_A  16
#define ENCODER_B  4
#define SEL_BUTTON 17

// --- The Menu Class ---
class SimpleMenu {
  public:
    const char* title;
    const char** items;
    void (**actions)();
    int itemCount;
    int cursor = 0;

    SimpleMenu(const char* t, const char** i, void (**a)(), int count) {
      title = t;
      items = i;
      actions = a;
      itemCount = count;
    }

    void next() { cursor = (cursor + 1) % itemCount; }
    void prev() { cursor = (cursor - 1 + itemCount) % itemCount; }
    
    void execute() {
      if (actions[cursor] != nullptr) {
        actions[cursor](); 
      }
    }

    void draw() {
      display.clearDisplay();
      display.setTextSize(1);
      #ifdef USE_SH1106
        display.setTextColor(SH110X_WHITE);
      #else
        display.setTextColor(SSD1306_WHITE);  
      #endif
      display.setCursor(0,0);
      display.print("== "); display.print(title); display.println(" ==");
      display.println("");

      for (int i = 0; i < itemCount; i++) {
        display.print(i == cursor ? "> " : "  ");
        display.println(items[i]);
      }
      display.display();
    }
};

// --- Forward Declarations for Functions ---
void openSettings();
void goBack();
void ledOn();
void ledOff();

// --- Define Menu Data ---

// 1. Sub Menu: Settings
const char* setLabels[] = {"LED On", "LED Off", "<-- Back"};         // items on the menu
void (*setActions[])() = {ledOn, ledOff, goBack};                    // procedures menu items will call
SimpleMenu settingsMenu("SETTINGS", setLabels, setActions, 3);

// 2. Main Menu
const char* mainLabels[] = {"View Data", "Settings", "Reset"};
void (*mainActions[])() = {nullptr, openSettings, nullptr};          // nullptr for no action yet
SimpleMenu mainMenu("MAIN MENU", mainLabels, mainActions, 3);

// Global pointer to track which menu is active
SimpleMenu* currentMenu = &mainMenu;

// --- Action Logic ---
void openSettings() { currentMenu = &settingsMenu; }                 // switch menu to settings sub menu
void goBack()       { currentMenu = &mainMenu; }                     // switch back to main menu
void ledOn()        { Serial.println("LED ON"); }
void ledOff()       { Serial.println("LED OFF"); }

// -setup
void setup() {
  Serial.begin(115200);
  Serial.println("\n\nStarting Oled Sketch");  
  
  pinMode(ENCODER_A, INPUT_PULLUP);
  pinMode(ENCODER_B, INPUT_PULLUP);
  pinMode(SEL_BUTTON, INPUT_PULLUP);

  // i2c (oled)
  Wire.begin(OLED_SDA, OLED_SCL);       
  Wire.setClock(100000);        // optional: 100000 or 400000
  delay(200);

  Serial.println("Starting oled");
  #ifdef USE_SH1106
    // SH1106 initialization often requires 'true' to reset the display 
    if(!display.begin(0x3C, true)) { 
      Serial.println(F("SH1106 failed - trying 0x3D"));
      if(!display.begin(0x3D, true)) {
        Serial.println(F("SH1106 not found"));
        for(;;); // Don't proceed
      }
    }
  #else
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
      Serial.println(F("SSD1306 failed"));
      for(;;);
    }
  #endif

  // show startup message
  display.clearDisplay();
  display.setTextSize(1);
  #ifdef USE_SH1106
    display.setTextColor(SH110X_WHITE);
  #else
    display.setTextColor(SSD1306_WHITE);  
  #endif
  display.setCursor(0,0);
  display.println("Starting...");
  display.display(); 
  delay(3000);

  currentMenu->draw();
}

// -loop
void loop() {
  static int last_clk = HIGH;
  int clk = digitalRead(ENCODER_A);

  // Menu rotation
  if (clk != last_clk && clk == LOW) {
    if (digitalRead(ENCODER_B) != clk) currentMenu->next();
    else currentMenu->prev();
    currentMenu->draw();
  }
  last_clk = clk;

  // menu click
  if (digitalRead(SEL_BUTTON) == LOW) {
    delay(200); // debounce
    currentMenu->execute();
    currentMenu->draw();
    while(digitalRead(SEL_BUTTON) == LOW);
  }
}