/**********************************************************************
 * 24C32 I²C EEPROM Driver - ESP32-C3 Boilerplate
 * RAM-Puffer | Auto-Vergleich | Page-Write | Timer-Commit
 * 1.000.000 Zyklen/Byte | C-Style | Flash-sicher
 *********************************************************************/

#include <Wire.h>

// === CONFIG ===
#define EEPROM_ADDR   0x50        // A0-A2 = GND
#define EEPROM_SIZE   4096        // 32Kbit = 4KB
#define PAGE_SIZE     32          // 24C32 Page Limit
#define COMMIT_MS     30000UL     // 30s Auto-Commit

// === GLOBALS ===
byte eeprom_buffer[EEPROM_SIZE];
bool buffer_dirty = false;
unsigned long last_commit = 0;
bool page_dirty[EEPROM_SIZE / PAGE_SIZE];  // 128 Pages

// === API ===
void eeprom_init(void);
bool eeprom_update(int addr, byte val);
byte eeprom_read(int addr);
void eeprom_commit(void);

// === SETUP ===
void setup() {
  Serial.begin(115200);
  while (!Serial);
  delay(100);
  Serial.println("Reset.");
  
  // Wire: SDA=GPIO8, SCL=GPIO9 (ESP32-C3 Default)
  Wire.begin(8, 9);
  Wire.setClock(400000);  // 400kHz
  
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
                  millis()/1000, buffer_dirty ? "YES" : "NO");
  }
  
  // AUTO-COMMIT (30s)
  if (buffer_dirty && millis() - last_commit > COMMIT_MS) {
    eeprom_commit();
    Serial.println(F("EEPROM: COMMIT OK"));
  }
  
  // === DEINE ANWENDUNG HIER ===
  // eeprom_update(42, sensor_value);
}

// === EEPROM IMPLEMENTATION ===
void eeprom_init(void) {
  // Vollständigen Puffer laden
  Wire.beginTransmission(EEPROM_ADDR);
  Wire.write(0); Wire.write(0);  // Addr 0x0000
  Wire.endTransmission(false);
  
  uint16_t bytes_read = Wire.requestFrom(EEPROM_ADDR, (uint8_t)EEPROM_SIZE);
  for(uint16_t i = 0; i < bytes_read; i++) {
    eeprom_buffer[i] = Wire.read();
  }
  
  memset(page_dirty, 0, sizeof(page_dirty));
  buffer_dirty = false;
  last_commit = millis();
  Serial.printf("EEPROM: Loaded %u bytes\n", bytes_read);
}

bool eeprom_update(int addr, byte val) {
  if (addr < 0 || addr >= EEPROM_SIZE) return false;
  
  if (eeprom_buffer[addr] != val) {
    eeprom_buffer[addr] = val;
    buffer_dirty = true;
    page_dirty[addr / PAGE_SIZE] = true;
    return true;
  }
  return false;
}

byte eeprom_read(int addr) {
  if (addr < 0 || addr >= EEPROM_SIZE) return 0;
  return eeprom_buffer[addr];
}

void eeprom_commit(void) {
  if (!buffer_dirty) return;
  
  uint8_t pages_written = 0;
  for(int page_idx = 0; page_idx < (EEPROM_SIZE / PAGE_SIZE); page_idx++) {
    if (!page_dirty[page_idx]) continue;
    
    int page_start = page_idx * PAGE_SIZE;
    int page_end = min(page_start + PAGE_SIZE, EEPROM_SIZE);
    
    Wire.beginTransmission(EEPROM_ADDR);
    Wire.write(page_start >> 8);     // MSB
    Wire.write(page_start & 0xFF);   // LSB
    for(int i = page_start; i < page_end; i++) {
      Wire.write(eeprom_buffer[i]);
    }
    Wire.endTransmission();
    
    delay(5);  // 5ms Write Cycle
    pages_written++;
    page_dirty[page_idx] = false;
  }
  
  buffer_dirty = false;
  last_commit = millis();
  Serial.printf("EEPROM: %u pages committed\n", pages_written);
}
