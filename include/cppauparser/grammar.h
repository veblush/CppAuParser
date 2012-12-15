// Copyright 2012 Esun Kim

#ifndef _CPPAUPARSER_GRAMMAR_H_
#define _CPPAUPARSER_GRAMMAR_H_

#include "base.h"
#include "strs.h"
#include <stdint.h>
#include <vector>
#include <map>
#include <utility>

namespace cppauparser {

class CppAuParserDecl Property {
 public:
  int index;
  utf8_string name;
  utf8_string value;
};

class CppAuParserDecl CharacterSet {
 public:
  int index;
  int uniplane;
  std::vector<std::pair<uint16_t, uint16_t>> ranges;
};

namespace SymbolType {
enum T {
  kNonTerminal = 0,
  kTerminal = 1,
  kNoise = 2,
  kEndOfFile = 3,
  kGroupStart = 4,
  kGroupEnd = 5,
  kDecremented = 6,
  kError = 7
};
}

class CppAuParserDecl Symbol {
 public:
  int index;
  utf8_string name;
  SymbolType::T type;

 public:
  bool single_lexeme;

 public:
  utf8_string GetID() const;
};

namespace AdvanceModeType {
enum T {
  kToken = 0,
  kCharacter = 1
};
}

namespace EndingModeType {
enum T {
  kOpen = 0,
  kClosed = 1
};
}

class CppAuParserDecl SymbolGroup {
 public:
  int index;
  utf8_string name;
  int container;
  int start;
  int end;
  AdvanceModeType::T advance_mode;
  EndingModeType::T ending_mode;
  std::vector<int> nesting_groups;
};

class CppAuParserDecl Production {
 public:
  int index;
  int head;
  std::vector<int> handles;

 public:
  const Symbol* head_ref;
  std::vector<const Symbol*> handle_refs;

 public:
  bool sr_forward_child;
  bool sr_merge_child;
  bool sr_listify_recursion;
  bool sr_remove_single_lexeme;

 public:
  utf8_string GetID() const;
};

class CppAuParserDecl DFAEdge {
 public:
  int charset;
  int target;
};

class CppAuParserDecl DFAState {
 public:
  int index;
  int accept_symbol;
  std::vector<DFAEdge> edges;

 public:
  int16_t jmp_table[0x80];
  struct JmpRange {
    uint16_t range_from;
    uint16_t range_to;
    int16_t target;
  };
  std::vector<JmpRange> jmp_ranges;
};

namespace LALRActionType {
enum T {
  kShift = 1,
  kReduce = 2,
  kGoto = 3,
  kAccept = 4
};
}

class CppAuParserDecl LALRAction {
 public:
  int symbol;
  LALRActionType::T type;
  int target;
};

class CppAuParserDecl LALRState {
 public:
  int index;
  std::map<int, LALRAction> actions;

 public:
  std::vector<LALRAction*> jmp_table;
};

class CppAuParserDecl Grammar {
 public:
  Grammar();

  bool LoadFile(const PATHCHAR* file_path);
  bool LoadBuffer(const char* buf, size_t len);

 private:
  void ProcessAfterLoad();
  void LinkReference();
  void BuildDFALookup();
  void BuildLALRLookup();
  void SetSingleLexemeSymbol();
  void SetSimplicationRule();

 public:
  Symbol* GetSymbol(const utf8_string& id) const;
  Symbol* GetSymbol(const char* id) const;
  Production* GetProduction(const utf8_string& id) const;
  Production* GetProduction(const char* id) const;

 public:
  std::vector<Property> properties;
  std::vector<CharacterSet> charsets;
  std::vector<Symbol> symbols;
  std::vector<SymbolGroup> symbol_groups;
  std::vector<Production> productions;
  int dfa_init;
  std::vector<DFAState> dfa_states;
  int lalr_init;
  std::vector<LALRState> lalr_states;
  const Symbol* symbol_EOF;
  const Symbol* symbol_Error;

 private:
  std::map<utf8_string, const Symbol*> symbol_pname_lookup_;
  std::map<utf8_string, const Production*> production_pname_lookup_;

  CPPAUPARSER_UNCOPYABLE(Grammar);
};

}

#endif  // _CPPAUPARSER_LANGUAGE_H_
