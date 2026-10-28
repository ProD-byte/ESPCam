#pragma once
#include <Arduino.h>

// Maskiert Zeichen für einen JSON-String (z. B. SSIDs mit Anführungszeichen)
inline String jsonEscape(const String &s) {
  String out;
  out.reserve(s.length() + 4);
  for (char c : s) {
    switch (c) {
      case '"': out += "\\\""; break;
      case '\\': out += "\\\\"; break;
      case '\n': out += "\\n"; break;
      case '\r': out += "\\r"; break;
      case '\t': out += "\\t"; break;
      default:
        if ((uint8_t)c < 0x20) {
          char buf[8];
          snprintf(buf, sizeof(buf), "\\u%04x", c);
          out += buf;
        } else {
          out += c;
        }
    }
  }
  return out;
}
