 // WINTI PUZZLE
// By Juriaan Gregor
// Bijlmer Bajes Ontgrendeld

// Include our DmxLibrary
#include <DmxMaster.h>

// Define our pins
#define M1 5
#define M2 6
#define M3 7
#define M4 A3          // Winti Masker (A0)
#define DP1 11            // Winti Amulet override
#define DP2 12            // Winti Masker override
#define DMX 3
#define POWER 8
#define SAFETY 9
#define maxChannels 4
#define NOFIELD 557L
#define NEEDEDMAGNET 50

// Define our Variables
bool S1 = false;
bool S2 = false;
bool S3 = false;
bool S4 = false;
bool Masker = false;
bool State = false;
bool Changed = false;
bool Toggle = false;
bool D1L = false;
bool D2L = false;

void setup() {

  // Set up our Serial
  Serial.begin(9600);
  Serial.println("WINTI PUZZLE - STARTING UP");

  // Set our Magnet pins
  pinMode(M1, INPUT);
  pinMode(M2, INPUT);
  pinMode(M3, INPUT);
  pinMode(M4, INPUT);
  pinMode(POWER, OUTPUT);
  pinMode(SAFETY, OUTPUT);

  // Give high to POWER / Safety to switch to digital
  digitalWrite(POWER, HIGH);
  digitalWrite(SAFETY, HIGH);

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
   int raw4   = analogRead(M4);

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

   // Read and change the state of Sensor 2 (Inverse logic..)
   if (D2 == HIGH && S2 == true) {
     S2 = false;
     Changed = true;
     Serial.println("S2 - MOVED AWAY !");
   }
   else if(D2 == LOW && S2 == false) {
     S2 = true;
     Changed = true;
     Serial.println("S2 - MOVED TOWARDS");
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

   // Read and change the state of Sensor 4 (analogRead, because the stupid sensors think that amulet == masker..)
   if ((raw4 - NOFIELD >= NEEDEDMAGNET) && S4 == false) {
    S4 = true;
    Changed = true;
    Serial.println("S4 [WINTI MASKER] - MOVED TOWARDS");
   }
   else if ((raw4 - NOFIELD < NEEDEDMAGNET) && S4 == true) {
    S4 = false;
    Changed = true;
    Serial.println("S4 [WINTI MASKER] - MOVED AWAY !");
   }
    
   // State master..
   if (Changed == true) {
     if (S1 == true && S2 == true && S3 == true) {
      if (State == false) {
  
        // Set State to true
        State = true;

        // Print out
        Serial.println("STATE : AMULET ");
      }
     }

     if (S4 == true) {
      if (Masker == false) {

        // Set state to true
        Masker = true;

        // Print out :)
        Serial.println("STATE : DROPPING MASK!");
      }
     }
   }

   // Override..
   //int D1D        = digitalRead(DP1);
   //int D2D        = digitalRead(DP2);

   // Check or Changed will be set to true, because of the override..
   //if (D1D != D1L) {
     // Changed = true;
     // D1L = D1D;
   //}

   //if(D2D != D2L) {
   // Changed = true;
    //D2L = D2D;
   //}
   
   if (Changed) {
      if (State == true) {
        
        // TURN OFF THE LIGHTING
        digitalWrite(POWER, LOW);

        // Turn on the blacklight
        DmxMaster.write(1, 255);
      }
      //else if(D1D == false) {

        // Turn on the lighting
        //digitalWrite(POWER, HIGH);

        // Turn off the blacklight
        //DmxMaster.write(1, 0);
      //}

      // WINTI Masker 
      if (Masker == true) {

        // DROP THE MASK
        digitalWrite(SAFETY, LOW);
      } 
   }
   

   // Delay
   delay(20);
}
