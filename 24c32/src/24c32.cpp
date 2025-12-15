#include "24c32.h"
#include <algorithm>
#include <string.h>

TwentyFourC32 EEPROM24C32; // global instance

TwentyFourC32::TwentyFourC32()
  : _wire(&Wire),
    _i2c_addr(DEFAULT_I2C_ADDR),
    _commit_ms(DEFAULT_COMMIT_MS),
    _buffer_dirty(false),
    _last_commit(0)
{
  // nothing here — init() will populate buffer
}

void TwentyFourC32::begin(TwoWire &wire, int sda, int scl, uint8_t i2c_addr, unsigned long commit_ms) {
  _wire = &wire;
  _i2c_addr = i2c_addr;
  _commit_ms = commit_ms;

  // If pins provided (non-negative), attempt to initialize the bus (ESP32 style)
  if (sda >= 0 && scl >= 0) {
    _wire->begin(sda, scl);
  }
}

void TwentyFourC32::init() {
  readFullBuffer();
  memset(_page_dirty, 0, sizeof(_page_dirty));
  _buffer_dirty = false;
  _last_commit = millis();
  Serial.printf("EEPROM: Loaded %u bytes\n", (unsigned)EEPROM_SIZE);
}

void TwentyFourC32::readFullBuffer() {
  // Set internal address pointer to 0x0000
  _wire->beginTransmission(_i2c_addr);
  _wire->write((uint8_t)0x00);
  _wire->write((uint8_t)0x00);
  _wire->endTransmission(false);

  // Request all bytes
  size_t bytes = _wire->requestFrom((int)_i2c_addr, (int)EEPROM_SIZE);
  size_t i = 0;
  for (; i < bytes && i < EEPROM_SIZE; ++i) {
    _buffer[i] = _wire->read();
  }
  // If fewer bytes returned, zero the rest
  if (i < EEPROM_SIZE) {
    memset(_buffer + i, 0, EEPROM_SIZE - i);
  }
}

bool TwentyFourC32::update(int addr, uint8_t value) {
  if (addr < 0 || addr >= (int)EEPROM_SIZE) return false;
  if (_buffer[addr] != value) {
    _buffer[addr] = value;
    _buffer_dirty = true;
    _page_dirty[addr / PAGE_SIZE] = true;
    return true;
  }
  return false;
}

uint8_t TwentyFourC32::read(int addr) const {
  if (addr < 0 || addr >= (int)EEPROM_SIZE) return 0;
  return _buffer[addr];
}

void TwentyFourC32::writePage(int page_idx) {
  if (page_idx < 0 || page_idx >= (int)(EEPROM_SIZE / PAGE_SIZE)) return;

  int page_start = page_idx * PAGE_SIZE;
  int page_end = std::min(page_start + PAGE_SIZE, (int)EEPROM_SIZE);

  _wire->beginTransmission(_i2c_addr);
  _wire->write((uint8_t)((page_start >> 8) & 0xFF)); // MSB
  _wire->write((uint8_t)(page_start & 0xFF));        // LSB
  for (int i = page_start; i < page_end; ++i) {
    _wire->write(_buffer[i]);
  }
  _wire->endTransmission();
  delay(5); // typical write cycle
}

void TwentyFourC32::commit() {
  if (!_buffer_dirty) return;

  int pages_written = 0;
  const int pages = EEPROM_SIZE / PAGE_SIZE;
  for (int page_idx = 0; page_idx < pages; ++page_idx) {
    if (!_page_dirty[page_idx]) continue;
    writePage(page_idx);
    _page_dirty[page_idx] = false;
    ++pages_written;
  }

  _buffer_dirty = false;
  _last_commit = millis();
  Serial.printf("EEPROM: %u pages committed\n", (unsigned)pages_written);
}

void TwentyFourC32::commitIfNeeded() {
  if (_buffer_dirty && (millis() - _last_commit > _commit_ms)) {
    commit();
  }
}

bool TwentyFourC32::isDirty() const {
  return _buffer_dirty;
}

unsigned long TwentyFourC32::lastCommit() const {
  return _last_commit;
}

const uint8_t* TwentyFourC32::getBuffer() const {
  return _buffer;
}

//
// Backwards-compatible wrappers
//
void eeprom_begin(TwoWire &wire, int sda, int scl, uint8_t i2c_addr, unsigned long commit_ms) {
  EEPROM24C32.begin(wire, sda, scl, i2c_addr, commit_ms);
}
void eeprom_init() { EEPROM24C32.init(); }
bool eeprom_update(int addr, uint8_t val) { return EEPROM24C32.update(addr, val); }
uint8_t eeprom_read(int addr) { return EEPROM24C32.read(addr); }
void eeprom_commit() { EEPROM24C32.commit(); }
void eeprom_commitIfNeeded() { EEPROM24C32.commitIfNeeded(); }
bool eeprom_isDirty() { return EEPROM24C32.isDirty(); }
unsigned long eeprom_lastCommit() { return EEPROM24C32.lastCommit(); }
const uint8_t* eeprom_getBuffer() { return EEPROM24C32.getBuffer(); }
