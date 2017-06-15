/*
 * Eindpuzzel
 * ========================================================
 * @Author    Juriaan Gregor
 * @Date      24 March 2017
 * @Version   1.2
 */

 // Include our libraries
 #include <Ethernet.h>
 #include <EthernetUdp.h>
 #include <SPI.h>
 #include <OSCBundle.h>
 #include "pitches.h"

 // Define 
 #define TONEs 9
 #define REKENMACHINE A0            // Data line of the Rekenmachine
 #define REKENMACHINE_S A1
 #define SYMTEX A2                // Data line of the Symtex
 #define TRIGGER A3                 // Data line of the Trigger
 #define OPSLAG 22                  // Opslag
 #define EIND 24                    // Einddeur
 #define LT  2                   // Rooie bob (Trigger) LED
 #define LS1 3                    // Mlody (Symtex 1) LED
 #define LS2 4                     // Bambi (Symtex 2) LED
 #define LR1 5                     // Esra (Rekenmachine 1) LED
 #define LR2 7                      // Kim (Rekenmachine 2) LED
 
 // Settings
 IPAddress ip(192, 168, 0, 120); // 192, 168, 0, 118 (Debug IP) (Normal IP arduino : 192, 168, 0, 114)
 const unsigned int inPort    = 8888;
 const unsigned int outPort   = 9999;
 const int samples = 10;           // The amount of samples that we use.
 #define DEBUG;
 #ifdef DEBUG
 #include "DebugUtils.h">
 #endif
 
 // Variables that are needed for libraries
 EthernetUDP Udp;
 OSCBundle bundleOUT;

 // Variables
 // 0 : Trigger           -->  Rooie Bob (CHECK !)
 // 1 : Symtex 1          -->  Mlody
 // 2 : Symtex 2          -->  Bambi
 // 3 : Rekenmachine 1    -->  Esra
 // 4 : Rekenmachine 2    -->  Kim
 int DATA[3]        = {TRIGGER, SYMTEX, REKENMACHINE};
 int RELAIS[2]      = {OPSLAG, EIND};
 int LEDs[5]        = {LT,LS1,LS2,LR1,LR2};
 int LEDSTATES[5]   = {0, 0, 0, 0, 0};

 // 0 : Disconnected
 // 1 : Connected
 // 2 : Override (ON)
 // 3 : Override (OFF)
 int STATE[5]       = {
                      0, // Trigger
                      0, // Symtex
                      0, // Rekenmachine
                      0, // Einddeur 
                      0  // Slot opslag
 };
 
 int dataT              = 0;
 int dataS              = 0;
 int dataR              = 0;
 int bufferT            = 0;
 int bufferS            = 0;
 int bufferR            = 0;
 int currentT           = 0;
 int currentS           = 0;
 int currentR           = 0;
 boolean foundhigh      = false;
 int readingsT[samples];
 int readingsS[samples];
 int readingsR[samples];
 unsigned long lastEcho;
 unsigned long lastCalc = millis();
 unsigned long timerS = millis();
 unsigned long timerC = millis();
 unsigned long lastTime;
 int debounce = 1000;
                 
 // Functions static
 static byte mac[6];
 
 //everything on the network needs a unique MAC 
void read_mac() {
   byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
}
  
// Function dynamic
//converts the pin to an osc address
char * numToOSCAddress( int pin){
    static char s[10];
    int i = 9;
  
    s[i--]= '\0';
  do
    {
    s[i] = "0123456789"[pin % 10];
                --i;
                pin /= 10;
    }
    while(pin && i);
    s[i] = '/';
    return &s[i];
}

int checkState(int part) {
  if (part <= sizeof(STATE)) {
    return STATE[part]; 
  }
}

void setState(int part, int state) {
  int check = checkState(part);

  if (check != state) {
    if (part <= (sizeof(STATE) / sizeof(int))) {

      // Set our state
      STATE[part] = state;

      // Debug
      Serial.println("");
      Serial.print("[STATE] State of part ");
      Serial.print(part);
      Serial.print(" has been set to ");
      Serial.print(state);

      if (state == 1 || state == 2) {
        if (part == 0) {
           digitalWrite(LT, HIGH);
        }
        else if (part == 1) {
          digitalWrite(LS1, HIGH);
          digitalWrite(LS2, HIGH);
        }
        else if(part == 2) {
          digitalWrite(LR1, HIGH);
          digitalWrite(LR2, HIGH);
        }
        else if(part == 3) {
          digitalWrite(EIND, HIGH);
        }
        else if (part == 4) {
          digitalWrite(OPSLAG, HIGH);
        }
      }
      else {
        if (part == 0) {
          digitalWrite(LT, LOW);
        }
        else if(part == 1) {
          digitalWrite(LS1, LOW);
          digitalWrite(LS2, LOW);
        }
        else if(part == 2) {
          digitalWrite(LR1, LOW);
          digitalWrite(LR2, LOW);
        }
        else if(part == 3) {
          digitalWrite(EIND, LOW);
        }
        else if(part == 4) {
          digitalWrite(OPSLAG, LOW);
        }
      }

      // Create OSC message             
      char outputAddress[12];
      strcpy(outputAddress, "/current");
      strcat(outputAddress, numToOSCAddress(part));
      bundleOUT.add(outputAddress).add(state);
    }
  }
}

void routeRead(OSCMessage &msg, int addrOffset) {
  Serial.println("[OSC] Route Read has been triggered !");

  for(byte pin = 0; pin < NUM_ANALOG_INPUTS; pin++) {
    int pinMatched = msg.match(numToOSCAddress(pin), addrOffset);
    if (pinMatched) {
      pinMode(analogInputToDigitalPin(pin), INPUT);
      char outputAddress[9];
      strcpy(outputAddress, "/read");
      strcat(outputAddress, numToOSCAddress(pin));

      // Do the analog read and send the results
      bundleOUT.add(outputAddress).add((int32_t)analogRead(pin));
    }
  }
}

void routeState(OSCMessage &msg, int addrOffset) {
  //Serial.println("[OSC] Route State has been triggered !");

  for (byte pin = 0; pin < NUM_DIGITAL_PINS; pin++) {
    int pinMatched = msg.match(numToOSCAddress(pin), addrOffset);
    if (pinMatched) {
       
       // Get our current state
       int currentState   = checkState(pin);
      // Serial.println("[STATE NR]");
       //Serial.print(currentState);
       char outputAddress[12];
       strcpy(outputAddress, "/current");
       strcat(outputAddress, numToOSCAddress(pin));

       // Tell us :)
       bundleOUT.add(outputAddress).add(currentState);
    }
  }
}

void routeSet(OSCMessage &msg, int addrOffset) {
  //Serial.println("[OSC] Route set has been triggered !");

  for (byte pin = 0; pin < NUM_DIGITAL_PINS; pin++) {
    int pinMatched = msg.match(numToOSCAddress(pin), addrOffset);
    if (pinMatched) {

       // Debug
       //Serial.println("[PIN] ");
       //Serial.print(numToOSCAddress(pin));

       // Set our state
       int val = msg.getInt(0);
       //Serial.println("[VALUE]");
       //Serial.print(val);
       setState(pin, val);
    }
  }
}

void routeDeur(OSCMessage &msg, int addrOffset) {
  
  // Serial.println("[OSC - Deur] Route deur has been triggered !");
  for (byte pin = 0; pin < NUM_DIGITAL_PINS; pin++) {
    int pinMatched = msg.match(numToOSCAddress(pin), addrOffset);
    if (pinMatched) {
       // Debug
       //Serial.println("[PIN] ");
       //Serial.print(numToOSCAddress(pin));

       // Set our state
       int val = msg.getInt(0);
       //Serial.println("[VALUE]");
       //Serial.print(val);
       // 0 : Opslag
       // 1 : Einddeur

       if (pin == 0) {
          setState(4, val);
       }
       if (pin == 1) {
          setState(3, val);
       }
    }
  }
  
}

void readTrigger() {

    int currentState      = checkState(0);
    if (currentState < 2) {

      // Read
      int val = analogRead(TRIGGER);
      readingsT[currentT] = val;
      Serial.println(val);
  
      // Add the reading to the total
      bufferT = bufferT + readingsT[currentT];
  
      // Advance to the next reading
      currentT = currentT + 1;
  
      // If we are at the end of the array
      if (currentT >= 2) {
        
        // Reset our indexes
        dataT = bufferT / 2;
        currentT = 0;   
        bufferT = 0;
  
        // Check the data and fire our state machine.
        if (dataT > 0 && dataT < 1022) {
          setState(0, 0);  
        }
        else if(dataT == 0) {
          setState(0, 1);
        }
      }
    }
    
}

void readSymtex() { 
  int currentState      = checkState(1);
  if (currentState < 2) {
    tone(TONEs, NOTE_D8, 20);

    delay(2);
  
    // Read our pulse length
    int pulseLength = pulseIn(SYMTEX, HIGH);
    Serial.println(pulseLength);
  
    if (pulseLength > 90 && pulseLength < 100) {
      //if (millis() - timerS > 1000) {
        //timerS = millis();
        setState(1, 1);
      //}
    }
    else {
      //if (millis() - timerS > 1000) {
        //timerS = millis();
        setState(1, 0);
      //}
    }
  }
}

void readCalc() {

  int currentState      = checkState(2);
  if (currentState < 2) {

    if (millis() - timerC > 1000) {
      // Read our pulse..
      int pulseLength = analogRead(REKENMACHINE);
    
      if (pulseLength < 20) {
          pulseLength = 0;
      }
     // Serial.println(pulseLength);
      
      int pulseLengthS = analogRead(REKENMACHINE_S);
      if (pulseLengthS < 20) {
        pulseLengthS = 0;
      }
  
      readingsR[currentR] = pulseLength;
      bufferR = bufferR + readingsR[currentR];
      currentR = currentR + 1;
      readingsR[currentR] = pulseLengthS;
      bufferR = bufferR + readingsR[currentR];
      currentR = currentR + 1;
      if (currentR >= 6) {
        dataR = bufferR / 6;
        currentR = 0;
        bufferR = 0;
  
        //Serial.println("DATA ");
        //Serial.print(dataR);
        //Serial.println("");
  
        if (dataR == 0) {
          setState(2, 1);
          timerC = millis();
        }
        else {
          setState(2, 0);
          timerC = millis();
        }
      }
    }
  }
}

void echo() {
    unsigned long current = millis();
    if (lastEcho + 1000 < millis()) {
      // Create OSC bundle as response for Max MSP
      bundleOUT.add("/echo").add(HIGH);
      
      lastEcho = millis();
      
    }
}



void setup() {

  // Set our LED outputs
  for (int i = 0; i < 5; i++) {
    pinMode(LEDs[i], OUTPUT);
  }

  // Set our input data lines
  //for (int i = 0; i < 3; i++) {
   // pinMode(DATA[i], INPUT);
  //}

  pinMode(REKENMACHINE, INPUT);
  pinMode(REKENMACHINE_S, INPUT);
  pinMode(SYMTEX, INPUT);
  pinMode(TRIGGER, INPUT);
  
  // Set our Relais data lines
  for (int i = 0; i < 2; i++) {
    pinMode(RELAIS[i], OUTPUT);
  }

  // TO-DO SET OUR RELAIS PINS
  
  // Turn on our Ethernet shield
  read_mac();
  Ethernet.begin(mac, ip);
  Udp.begin(inPort);

  // Turn on the Serial !
  Serial.begin(115200);

  // Start our LED Testcase
  Serial.println("EINDPUZZEL - STARTING UP !");
  Serial.println("TESTCASE - CHECKING LEDS !");
  setState(0, 1);
  delay(1000);
  setState(0, 0);
  setState(1, 1);
  delay(1000);
  setState(1, 0);
  setState(2, 1);
  delay(1000);
  setState(2, 0);
  delay(1000);

  // Set our relais 
  digitalWrite(OPSLAG,LOW);
  digitalWrite(EIND,LOW);
  
  // Set our ECHO
  lastEcho = millis();
}

void loop() {
    lastTime = millis();
    
    // Read out Trigger
    //readTrigger();

    // Read our Symtex
    //readSymtex();

    //Serial.println("Hello !");

    // Read our Calculator
    //readCalc();

    // Echo
    echo();

    // Maintain our Internet
    //Ethernet.maintain();
    
    // Read the override if needed
    OSCBundle bundleIN;
    int size    = Udp.parsePacket();

    if (size > 0) {

      while(size--)
         bundleIN.fill(Udp.read());
      if(!bundleIN.hasError()){
        bundleIN.route("/set", routeSet);
      } 
      //Serial.print("packet send !");
      //DEBUG_PRINT("\nReceived packet of size ");
      //DEBUG_PRINT(size);
      //DEBUG_PRINT("From IP :  ");
      //IPAddress remote = Udp.remoteIP();
      //DEBUG_PRINT(remote);
      //DEBUG_PRINT(" on port : ");
      //DEBUG_PRINT(Udp.remotePort());
    }

    // Send the response bundle back to where it  came from !
    Udp.beginPacket(Udp.remoteIP(), outPort);
    bundleOUT.send(Udp);
    Udp.endPacket();
    bundleOUT.empty(); // Clear the bundle so that we are sure that it is ready to use.

    delay(100); // (10 FPS)
    //unsigned long timeBetween = millis() - lastTime;
    //Serial.println(timeBetween);
}
