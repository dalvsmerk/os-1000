#pragma once

inline int from_ascii(char c) {
  return c - '0';
}

inline char to_ascii(int x) {
  return x + '0';
}
