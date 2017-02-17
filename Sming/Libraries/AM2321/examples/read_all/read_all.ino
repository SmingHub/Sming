#include <Wire.h>
#include <AM2321.h>

AM2321 ac;

void setup() {
  Serial.begin(9600);
}

void loop() {
  Serial.print("(");
  Serial.print(ac.read());
  Serial.print(",");
  Serial.print(ac.temperature);
  Serial.print(",");
  Serial.print(ac.humidity);
  Serial.println(")");
  delay(3000);
  
  Serial.println(ac.uid());
  delay(3000);
}
