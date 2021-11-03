//
//    oled testbed - 01Nov21
//


const int serialDebug = 1;

#include "oled.h"


//            --------------------------- settings -------------------------------


  const int iLED = 22;                    // onboard indicator led gpio pin
  
  
//            --------------------------------------------------------------------




// ----------------------------------------------------------------
//                            -Setup
// ---------------------------------------------------------------- 

void setup() {
  Serial.begin(115200); while (!Serial); delay(50);       // start serial comms   
  Serial.println("\n\n\n\Starting menu demo\n");

  pinMode(iLED, OUTPUT);     // onboard indicator led

  oledSetup();
}


// ----------------------------------------------------------------
//                            -Loop
// ---------------------------------------------------------------- 

void loop() {

  oledLoop();
    
}

// ---------------------------------------------------------------- 
