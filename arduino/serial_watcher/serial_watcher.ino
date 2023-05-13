#include <Arduino.h>

void setup() {
  Serial.begin(9600);
}

void loop() {
  // Read analog input from pin 0
  int sensorValue = analogRead(A0);

  // Get the current timestamp
  unsigned long currentTime = millis();

  // Print the timestamp and sensor value to the serial port
  Serial.print(currentTime);
  Serial.print(",");
  Serial.println(sensorValue);

  delay(1000); // Delay for 1 second
}
