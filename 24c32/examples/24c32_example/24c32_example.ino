#include <Arduino.h>
#include <Wire.h>
#include <24c32.h>

void setup() {
  Serial.begin(115200);
  while (!Serial) {}
  delay(50);
  Serial.println("24C32 Example");

  // Initialize Wire and library (ESP32-C3 pins used as example)
  eeprom_begin(Wire, 8, 9, 0x50, 30000UL);
  eeprom_init();

  // Example: write and read
  eeprom_update(0, 0x42);
  eeprom_commit(); // force commit immediately
  Serial.printf("Addr 0 = 0x%02X\n", eeprom_read(0));
}

void loop() {
  // Periodic auto-commit if needed
  eeprom_commitIfNeeded();
  delay(500);
}
