#include <24c32.h>

// === SETUP ===
void setup() {
  Serial.begin(115200);
  while (!Serial);
  delay(100);
  Serial.println("Reset.");
  
  // Initialize Wire and library: SDA=GPIO8, SCL=GPIO9 (ESP32-C3 Default)
  eeprom_begin(Wire, 8, 9, 0x50, 30000UL);
  // Set wire clock to 400kHz
  Wire.setClock(400000);
  
  eeprom_init();
  pinMode(8, OUTPUT);
  
  Serial.println(F("=== 24C32 EEPROM READY ==="));
}

// === MAIN LOOP ===
void loop() {
  static unsigned long heartbeat = 0;
  static unsigned long stats = 0;
  
  // HEARTBEAT (500ms)
  if (millis() - heartbeat > 500) {
    digitalWrite(8, !digitalRead(8));
    heartbeat = millis();
  }
  
  // STATS (5s)
  static unsigned long last_stats = 0;
  if (millis() - last_stats > 5000) {
    last_stats = millis();
    Serial.printf("[Uptime: %lu] Dirty: %s\n", 
                  millis()/1000, eeprom_isDirty() ? "YES" : "NO");
  }
  
  // AUTO-COMMIT (30s)
  eeprom_commitIfNeeded();
  
  // === YOUR APPLICATION HERE ===
  // eeprom_update(42, sensor_value);
}
