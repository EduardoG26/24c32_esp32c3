# 24c32 Arduino Library

Driver for the 24C32 I²C EEPROM with a RAM buffer, page-write and auto-commit behavior.

Features
- Full RAM buffer of the EEPROM (4KB)
- Page-level dirty tracking (32 byte pages)
- Auto-commit after configurable timeout
- Simple object-oriented API and backwards-compatible free functions

Installation
1. Copy the `24c32` folder into your Arduino `libraries/` directory, or add it to your project.
2. Restart the Arduino IDE so it detects the new library.

Quick usage
- The library exposes a TwentyFourC32 class (global instance `EEPROM24C32`) and a set of backwards-compatible C-style functions:
  - eeprom_begin(...)
  - eeprom_init()
  - eeprom_update(addr, value)
  - eeprom_read(addr)
  - eeprom_commit()
  - eeprom_commitIfNeeded()
  - eeprom_isDirty()
  - eeprom_lastCommit()
  - eeprom_getBuffer()

Note: There are no runtime "setter" functions. Configure I²C address, pins and commit interval when calling `begin()` (or by using the class API before `init()`).

API overview

Class-based API (preferred)
- void TwentyFourC32::begin(TwoWire &wire = Wire, int sda = 8, int scl = 9, uint8_t i2c_addr = 0x50, unsigned long commit_ms = 30000UL)
  - Initialize the driver. If `sda`/`scl` are non-negative the library will call `Wire.begin(sda, scl)` (ESP-style).
  - Provide custom I²C address and auto-commit interval here.
- void TwentyFourC32::init()
  - Load the complete EEPROM contents into the internal RAM buffer (blocking).
- bool TwentyFourC32::update(int addr, uint8_t value)
  - Update one byte in the RAM buffer and mark its page dirty. Returns true if the byte changed.
- uint8_t TwentyFourC32::read(int addr) const
  - Read a byte from the RAM buffer.
- void TwentyFourC32::commit()
  - Write all dirty pages to the EEPROM (blocking).
- void TwentyFourC32::commitIfNeeded()
  - Commit automatically if the buffer is dirty and the commit timeout elapsed.
- bool TwentyFourC32::isDirty() const
  - Returns whether the RAM buffer has pending changes.
- unsigned long TwentyFourC32::lastCommit() const
  - Timestamp (millis) of the last commit.
- const uint8_t* TwentyFourC32::getBuffer() const
  - Pointer to the internal RAM buffer (read-only).

C-style wrappers (backwards-compatible)
- eeprom_begin(...)
- eeprom_init()
- eeprom_update(addr, value)
- eeprom_read(addr)
- eeprom_commit()
- eeprom_commitIfNeeded()
- eeprom_isDirty()
- eeprom_lastCommit()
- eeprom_getBuffer()

Example
```cpp
#include <Arduino.h>
#include <Wire.h>
#include <24c32.h>

void setup() {
  Serial.begin(115200);
  while (!Serial) {}
  delay(50);

  // Initialize Wire and library. For ESP32-C3 default pins use SDA=8, SCL=9.
  // You can also change the I2C address and commit interval here.
  eeprom_begin(Wire, 8, 9, 0x50, 30000UL); // address 0x50, commit every 30s
  eeprom_init(); // load EEPROM into RAM buffer

  // Write and read via the RAM buffer (commits happen on commit() or automatically)
  eeprom_update(0, 0x42);
  eeprom_commit(); // force commit immediately
  Serial.printf("Addr 0 = 0x%02X\n", eeprom_read(0));
}

void loop() {
  // Call periodically to allow the driver to auto-commit when needed
  eeprom_commitIfNeeded();
  delay(500);
}
```

Notes and tips
- Configure the library via `begin()` before calling `init()`. There are intentionally no runtime setter functions — to change configuration re-call `eeprom_begin()` (and then `eeprom_init()` if you need to reload the buffer).
- The library assumes a 24C32 device: 4096 bytes total, 32-byte pages.
- The driver uses page-level writes; only pages marked dirty are written back during commit.
- `eeprom_init()` reads the entire EEPROM into RAM. If your hardware doesn't return the full size in one request, the remaining RAM buffer is zeroed.

Example sketch
- See `examples/24c32_example/24c32_example.ino` for a working example.

License
- MIT (see repository)
