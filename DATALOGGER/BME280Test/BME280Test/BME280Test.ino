#include <Wire.h>
#include <Adafruit_BME280.h>

Adafruit_BME280 bmeInside; // use I2C interface
Adafruit_BME280 bmeOutside; // use I2C interface
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  bmeInside.begin(0x76);
  bmeOutside.begin(0x77);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.print(bmeInside.readTemperature());
  Serial.println("degCInside");
  Serial.print(bmeOutside.readTemperature());
  Serial.println("degCOutside");
  delay(3000);
}
