// Copyright 2012 Esun Kim

#ifndef _CPPAUPARSER_STRS_H_
#define _CPPAUPARSER_STRS_H_

#include "base.h"
#include <stdint.h>
#include <string>

namespace cppauparser {

// byte
typedef unsigned char byte;

// utf8_string
typedef std::basic_string<byte> utf8_string;

// standard string format function that return value as utf8_string
CppAuParserDecl utf8_string utf8_format(const char* fmt, ...);

// convert utf16 to utf8 string
CppAuParserDecl const uint16_t* convert_utf16_to_utf8_string(
    const uint16_t* utf16_str, utf8_string* utf8_str);

// utf8_substring
class CppAuParserDecl utf8_substring {
 public:
  inline utf8_substring()
    : str_(nullptr), len_(0) {
  }

  inline utf8_substring(const byte* str, size_t len)
    : str_(str), len_(len) {
  }

 public:
  inline const byte* c_str() const {
    return str_;
  }

  inline size_t size() const {
    return len_;
  }

  inline bool empty() const {
    return len_ == 0;
  }

  inline utf8_string get_string() const {
    return utf8_string(str_, len_);
  }

 private:
  const byte* str_;
  size_t len_;
};

}

#endif  // _CPPAUPARSER_STRS_H_
