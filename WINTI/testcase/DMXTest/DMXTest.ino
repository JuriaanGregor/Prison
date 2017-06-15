#include <DmxMaster.h>

void setup() {

  // Set our Serial
  Serial.begin(9600);
  Serial.println("STARTING");

  // The Pin that the Arduino uses to send the DMX data to our shield.
  DmxMaster.usePin(3);

  // How many channels are allowed to be sent (Max 512)
  DmxMaster.maxChannel(4);

  // SET EVERTHING TO 0
  for (int i = 0; i <= 4; i++) {
    DmxMaster.write(i + 1, 0);
  }
}

void loop() {
  int brightness;
  for (brightness = 0; brightness <= 255; brightness++) {

    // Update our DMX channels to showcase the new Brightness
    DmxMaster.write(1, brightness); 

    Serial.println("SENDING DMX");

    // Small delay
    delay(10);
  }
}
