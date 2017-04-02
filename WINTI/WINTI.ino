
// WINTI PUZZLE
// By Juriaan Gregor
// Bijlmer Bajes Ontgrendeld

// Include our DmxLibrary
#include <DmxMaster.h>

// Define our pins
#define M1 10
#define M2 11
#define M3 12
#define DMX 3
#define POWER 2
#define maxChannels 4
#define ledPin 13

// Define our Variables
bool S1 = false;
bool S2 = false;
bool S3 = false;
bool State = false;
bool Changed = false;
bool Toggle = false;

void setup() {

  // Set up our Serial
  Serial.begin(9600);
  Serial.println("WINTI PUZZLE - STARTING UP");

  // Set our Magnet pins
  pinMode(M1, INPUT);
  pinMode(M2, INPUT);
  pinMode(M3, INPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(POWER, OUTPUT);

  // Turn on our normally lighting by giving a LOW
  digitalWrite(POWER, LOW); // Normally Closed --> LOW = Stroom

  // Setting up our DMX
  DmxMaster.usePin(DMX);
  DmxMaster.maxChannel(maxChannels);

  // Nullify all our channels
  for (int c = 0; c <= 4; c++) {
    DmxMaster.write(c + 1, 0);
  }

  // Turn on the Light 
  DmxMaster.write(1, 255);
  delay(1000);
  DmxMaster.write(1, 0);
}

void loop() {

   // Read all our channels that we need :)
   int D1 = digitalRead(M1);
   int D2 = digitalRead(M2);
   int D3 = digitalRead(M3);

   // Read and change the State of Sensor 1
   if (D1 == HIGH && S1 == false) {
     S1 = true;
     Changed = true;
     Serial.println("S1 - MOVED TOWARDS !");
   }
   else if(D1 == LOW && S1 == true) {
     S1 = false;
     Changed = true;
     Serial.println("S1 - MOVED AWAY");
   }

   // Read and change the state of Sensor 2
   if (D2 == HIGH && S2 == false) {
     S2 = true;
     Changed = true;
     Serial.println("S2 - MOVED TOWARDS !");
   }
   else if(D2 == LOW && S2 == true) {
     S2 = false;
     Changed = true;
     Serial.println("S2 - MOVED AWAY");
   }

   // Read and change the state of Sensor 3
   if (D3 == HIGH && S3 == false) {
     S3 = true;
     Changed = true;
     Serial.println("S3 - MOVED TOWARDS !");
   }
   else if(D3 == LOW && S3 == true) {
     S3 = false;
     Changed = true;
     Serial.println("S3 - MOVED AWAY");
   }

   // Turn on the Blacklight if all States are true
   if (Changed == true) {
     if (S1 == true && S2 == true && S3 == true) {
      if (State == false) {
  
        // Set State to true
        State = true;
        Serial.println("STATE : CURRENTLY RUNNING ");
        
        // TURN OFF NORMALLY LIGHTING
        digitalWrite(POWER, HIGH);
        
        // SEND DMX
        DmxMaster.write(1, 255);
      }
     }
     else if(State == true) {
        
        // Print our state
        Serial.println("STATE : RESTORING LIGHTING");

        if (Toggle) {
          // SEND DMX TO NULLIFY
          DmxMaster.write(1, 0);
          
          // Turn on the normally ligting
          digitalWrite(POWER, LOW);
        }
        
        // Set State to false
        State = false;
     }

     Changed = false;
   }

   // Delay
   delay(20);
}
