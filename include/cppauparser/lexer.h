// Copyright 2012 Esun Kim

#ifndef _CPPAUPARSER_LEXER_H_
#define _CPPAUPARSER_LEXER_H_

#include "base.h"
#include "grammar.h"
#include "strs.h"
#include <vector>
#include <utility>
#include <memory>

namespace cppauparser {

struct CppAuParserDecl Token {
  const Symbol* symbol;
  utf8_substring lexeme;
  std::pair<int, int> position;

 public:
  Token();
  Token(const Symbol* symbol,
    utf8_substring lexeme,
    std::pair<int, int> position);

  utf8_string GetString() const;
};

class CppAuParserDecl LexerBuffer {
 public:
  LexerBuffer();
  ~LexerBuffer();

  byte* GetBuffer();
  size_t GetBufferSize();
  void SetBuffer(byte* buf, size_t size, bool sharable);

  void Clear();
  void Swap(LexerBuffer& b);

 private:
  byte* buf_;
  size_t buf_size_;
  bool buf_sharable_;

  CPPAUPARSER_UNCOPYABLE(LexerBuffer);
};

class CppAuParserDecl Lexer {
 public:
  explicit Lexer(const Grammar& grammar);
  ~Lexer();

  bool LoadFile(const PATHCHAR* file_path);
  bool LoadString(const char* buf);
  bool LoadBuffer(const byte* buf, size_t size);
  void Unload();
  void ResetCursor();

  std::shared_ptr<LexerBuffer> ReleaseBuffer();

 private:
  void PeekToken(Token* token);
  void AdvancePeekBuffer();
  void AdvanceBuffer(size_t n);

 public:
  void ReadToken(Token* token);

  int GetLine() const;
  int GetColumn() const;
  std::pair<int, int> GetPosition() const;

 private:
  const Grammar& grammar_;

  LexerBuffer allocator_;
  byte* buf_;
  byte* buf_cur_;
  byte* buf_end_;
  byte* buf_peek_;

  int line_;
  int column_;

  struct Group {
    const SymbolGroup* symbol_group;
    utf8_substring text;
  };
  std::vector<Group> group_stack_;

  CPPAUPARSER_UNCOPYABLE(Lexer);
};

}

#endif  // _CPPAUPARSER_LEXER_H_
