#pragma once
#include <Arduino.h>

class Button {
public:
  Button(int pin, bool usePullup = true, uint32_t debounceMs = 35)
  : _pin(pin), _usePullup(usePullup), _debounceMs(debounceMs) {}

  void begin() {
    if (_pin < 0) return;
    pinMode(_pin, _usePullup ? INPUT_PULLUP : INPUT);
  }

  // Call frequently (every loop)
  void update() {
    if (_pin < 0) return;

    bool raw = rawDown();
    uint32_t t = millis();

    if (raw != _lastRaw) {
      _lastRaw = raw;
      _lastRawChangeMs = t;
    }

    if ((t - _lastRawChangeMs) >= _debounceMs) {
      if (_isDown != raw) {
        _isDown = raw;

        if (_isDown) {                 // pressed
          _pressStartMs = t;
          _consumedLong = false;
          _pressedEvent = true;
        } else {                       // released
          _releasedEvent = true;
        }
      }
    }
  }

  // Events (true once until next update() cycle clears them)
  bool pressed() {
    bool v = _pressedEvent;
    _pressedEvent = false;
    return v;
  }

  bool released() {
    bool v = _releasedEvent;
    _releasedEvent = false;
    return v;
  }

  // Long press fires once per press. Marks the press as "consumed"
  // so you can prevent short action on release.
  bool longPress(uint32_t longMs = 650) {
    if (_pin < 0) return false;
    if (_isDown && !_consumedLong && (millis() - _pressStartMs >= longMs)) {
      _consumedLong = true;
      return true;
    }
    return false;
  }

  // Use after release(): returns true only if long press wasn't fired.
  bool shortReleaseNotConsumed() const {
    return !_consumedLong;
  }

  bool isDown() const { return _isDown; }

private:
  // Assumes INPUT_PULLUP wiring (pressed = LOW). If you ever wire differently,
  // you can adjust this function.
  bool rawDown() const { return digitalRead(_pin) == LOW; }

  int _pin;
  bool _usePullup;
  uint32_t _debounceMs;

  bool _isDown = false;
  bool _lastRaw = false;
  uint32_t _lastRawChangeMs = 0;

  uint32_t _pressStartMs = 0;
  bool _consumedLong = false;

  bool _pressedEvent = false;
  bool _releasedEvent = false;
};