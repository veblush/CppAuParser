// Copyright [year] <Copyright Owner>

#include "strs.h"
#include <stdio.h>
#include <stdarg.h>

namespace cppauparser {

utf8_string utf8_format(const char* fmt, ...) {
  // try with stack buffer
  char buf[4];
  va_list ap;
  va_start(ap, fmt);
  int r = vsnprintf(buf, sizeof(buf), fmt, ap);
  va_end(ap);
  if ((unsigned)r < sizeof(buf)) {
    // fit-in
    return utf8_string(reinterpret_cast<byte*>(buf));
  } else {
    // find buffer size by doubling a size of buffer
    utf8_string ret;
    size_t buf_size = sizeof(buf) * 2;
    while (true) {
      ret.resize(buf_size);
      va_start(ap, fmt);
      r = vsnprintf(reinterpret_cast<char*>(&ret[0]), buf_size, fmt, ap);
      va_end(ap);
      if ((unsigned)r < buf_size) {
        ret.resize(r);
        return ret;
      } else {
        buf_size = buf_size * 2;
      }
    }
  }
  return reinterpret_cast<byte*>(buf);
}

const uint16_t* convert_utf16_to_utf8_string(const uint16_t* utf16_str,
                                             utf8_string* utf8_str) {
  // calculate the size of utf8 string
  size_t utf8_len = 0;
  const uint16_t* cur = utf16_str;
  for ( ; *cur != 0; cur += 1) {
    uint16_t c = *cur;
    if (c < 0x80) {
      utf8_len += 1;
    } else if (c < 0x800) {
      utf8_len += 2;
    } else {
      utf8_len += 3;
    }
  }

  // allocate utf8 string
  utf8_str->resize(utf8_len);
  if (utf8_len == 0) {
    return utf16_str + 1;
  }

  // convert utf16 to utf8
  byte* buf = &(*utf8_str)[0];
  for (const uint16_t* cur = utf16_str; *cur != 0; cur += 1) {
    uint16_t c = *cur;
    if  (c < 0x80) {
      *(buf++) = byte(c);
    } else if (c < 0x800) {
      *(buf++) = byte(0xC0 | (c >> 6));
      *(buf++) = byte(0x80 | (c & 0x3F));
    } else {
      *(buf++) = byte(0xE0 | (c >> 12));
      *(buf++) = byte(0x80 | ((c >> 6) & 0x3F));
      *(buf++) = byte(0x80 | (c & 0x3F));
    }
  }

  return cur + 1;
}

}
