// Copyright 2012 Esun Kim

#ifndef _CPPAUPARSER_PARSER_H_
#define _CPPAUPARSER_PARSER_H_

#include "base.h"
#include "grammar.h"
#include "lexer.h"
#include <vector>
#include <memory>
#include <utility>

namespace cppauparser {

namespace ParseResultType {
enum T {
  kAccept = 1,
  kShift = 2,
  kReduce = 3,
  kReduceEliminated = 4,
  kError = 5,
};
};

struct CppAuParserDecl ParseItem {
  const LALRState* state;
  const Production* production;
  Token token;
  mutable void* data;

 public:
  utf8_string GetString() const;
};

struct CppAuParserDecl ParseReduction {
  const Production* production;
  ParseItem* head;
  std::vector<ParseItem>* handles;

 public:
  utf8_string GetString() const;
};

namespace ParseErrorType {
enum T {
    kNone = 0,
    kLexicalError = 1,
    kSyntaxError = 2,
    kInternalError = 3
};
};

struct CppAuParserDecl ParseErrorInfo {
  ParseErrorType::T type;
  std::pair<int, int> position;
  const LALRState* state;
  Token token;
  std::vector<const Symbol*> expected_symbols;

 public:
  ParseErrorInfo();
  ParseErrorInfo(ParseErrorType::T type, std::pair<int, int> position,
                  const LALRState* state, Token token);
  utf8_string GetString() const;
};

class CppAuParserDecl Parser {
 public:
  explicit Parser(const Grammar& grammar);
  ~Parser();

 public:
  bool LoadFile(const PATHCHAR* file_path);
  bool LoadString(const char* buf);
  bool LoadBuffer(const byte* buf, size_t size);
  void ResetCursor();

  std::shared_ptr<LexerBuffer> ReleaseBuffer();

  ParseResultType::T ParseStep();
  ParseResultType::T ParseReduce();
  ParseResultType::T ParseAll();

  template<typename T>
  ParseResultType::T ParseAll(T& handler) {
    while (true) {
      ParseResultType::T ret = ParseStep();
      handler(ret, *this);
      if (ret == ParseResultType::kAccept ||
          ret == ParseResultType::kError) {
        return ret;
      }
    }
  }

  const LALRState* GetState() const;
  const ParseItem& GetTop() const;
  const std::vector<ParseItem>& GetStack() const;
  const Token& GetToken() const;
  const ParseReduction& GetReduction() const;
  const ParseErrorInfo& GetErrorInfo() const;

  int GetLine() const;
  int GetColumn() const;
  std::pair<int, int> GetPosition() const;

 private:
  void ResetState();
  void ReadToken(Token* token);

 private:
  const Grammar& grammar_;
  Lexer lexer_;
  bool trim_reduction_;

  const LALRState* state_;
  std::vector<ParseItem> stack_;
  std::vector<ParseItem> reduction_handles_;
  Token token_;
  bool token_used_;
  ParseReduction reduction_;
  ParseErrorInfo error_info_;

  CPPAUPARSER_UNCOPYABLE(Parser);
};

class CppAuParserDecl ProductionHandler {
 public:
  ProductionHandler(const Grammar& grammar);

  typedef void*(*Handler)(const std::vector<ParseItem>&);
  Handler GetHandler(const char* production_id);
  bool SetHandler(const char* production_id, Handler handler);

  void operator()(ParseResultType::T ret, const Parser& parser);

  void* GetResult() const;

 private:
  const Grammar& grammar_;
  std::vector<Handler> handlers_;
  void* result_;
};

#define PH_ARGS const std::vector<cppauparser::ParseItem>& c

#define PH_ON(ph, p, e) { \
    struct LambdaDummy { \
      static void* h(PH_ARGS) { e } \
    }; \
    (ph).SetHandler((p), &LambdaDummy::h); \
  }

}  // namespace cppauparser

#endif  // _CPPAUPARSER_PARSER_H_
