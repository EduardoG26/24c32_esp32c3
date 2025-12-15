#ifndef TWENTYFOURC32_H
#define TWENTYFOURC32_H

#include <Arduino.h>
#include <Wire.h>
#include <stdint.h>

class TwentyFourC32 {
public:
  // Defaults match 24C32 / ESP32-C3 sketch
  static constexpr uint8_t DEFAULT_I2C_ADDR = 0x50;
  static constexpr uint16_t EEPROM_SIZE = 4096;   // 32Kbit = 4KB
  static constexpr uint8_t PAGE_SIZE = 32;        // 24C32 Page Limit
  static constexpr unsigned long DEFAULT_COMMIT_MS = 30000UL;

  TwentyFourC32();

  // begin: optionally provide Wire instance and pins (ESP32-C3 SDA/SCL),
  // device address and auto-commit interval.
  // If you call begin with custom pins the library will call wire.begin(sda, scl).
  void begin(TwoWire &wire = Wire, int sda = 8, int scl = 9,
             uint8_t i2c_addr = DEFAULT_I2C_ADDR,
             unsigned long commit_ms = DEFAULT_COMMIT_MS);

  // Load EEPROM into RAM buffer (blocking)
  void init();

  // Update a byte in the RAM buffer (marks page dirty). Returns true if changed.
  bool update(int addr, uint8_t value);

  // Read a byte from the RAM buffer
  uint8_t read(int addr) const;

  // Commit all dirty pages to EEPROM (blocking)
  void commit();

  // Commit if auto-commit timeout elapsed (no-op if not dirty)
  void commitIfNeeded();

  // Helpers
  bool isDirty() const;
  unsigned long lastCommit() const;
  const uint8_t* getBuffer() const;

private:
  TwoWire* _wire;
  uint8_t _i2c_addr;
  unsigned long _commit_ms;

  uint8_t _buffer[EEPROM_SIZE];
  bool _buffer_dirty;
  unsigned long _last_commit;
  bool _page_dirty[EEPROM_SIZE / PAGE_SIZE];

  // internal helpers
  void readFullBuffer();
  void writePage(int page_idx);
};

//
// Backwards-compatible global instance + C-style API
//
extern TwentyFourC32 EEPROM24C32;

// Old-style function names for compatibility with sketches:
void eeprom_begin(TwoWire &wire = Wire, int sda = 8, int scl = 9,
                  uint8_t i2c_addr = TwentyFourC32::DEFAULT_I2C_ADDR,
                  unsigned long commit_ms = TwentyFourC32::DEFAULT_COMMIT_MS);
void eeprom_init();
bool eeprom_update(int addr, uint8_t val);
uint8_t eeprom_read(int addr);
void eeprom_commit();
void eeprom_commitIfNeeded();
bool eeprom_isDirty();
unsigned long eeprom_lastCommit();
const uint8_t* eeprom_getBuffer();

#endif // TWENTYFOURC32_H
