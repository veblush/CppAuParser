// Copyright 2012 Esun Kim

#include "parser.h"
#include <vector>
#include <utility>

namespace cppauparser {

utf8_string ParseItem::GetString() const {
  if (production) {
    return utf8_format("S=%d, P=%s", state->index,
                                     production->GetID().c_str());
  } else if (token.symbol) {
    return utf8_format("S=%d, T=%s", state->index,
                                     token.symbol->GetID().c_str());
  } else {
    return utf8_format("S=%d", state->index);
  }
}

utf8_string ParseReduction::GetString() const {
  utf8_string handles_str;
  for (auto i = handles->begin(), i_end = handles->end(); i != i_end; ++i) {
    if (i == handles->begin()) {
      handles_str += i->GetString();
    } else {
      handles_str += (byte*)", " + i->GetString();
    }
  }
  return utf8_format("P=%d, H=(%s), Hs=[%s]", production->index,
                                              head->GetString().c_str(),
                                              handles_str.c_str());
}

ParseErrorInfo::ParseErrorInfo()
    : type(ParseErrorType::kNone),
      position(std::make_pair(0, 0)),
      state(nullptr) {
}

ParseErrorInfo::ParseErrorInfo(
    ParseErrorType::T type, std::pair<int, int> position,
    const LALRState* state, Token token)
    : type(type),
      position(position),
      state(state),
      token(token) {
}

utf8_string ParseErrorInfo::GetString() const {
  switch (type) {
  case ParseErrorType::kNone:
    return utf8_format("None");

  case ParseErrorType::kLexicalError:
    return utf8_format("LexicalError(%d:%d) Token='%s'",
        token.position.first, token.position.second,
        token.lexeme.get_string().c_str());

  case ParseErrorType::kSyntaxError: {
      utf8_string e_str;
      for (auto i = expected_symbols.begin(),
                i_end = expected_symbols.end();
           i != i_end; ++i) {
        if (i == expected_symbols.begin()) {
          e_str += (*i)->GetID();
        } else {
          e_str += (byte*)", " + (*i)->GetID();
        }
      }
      return utf8_format("SyntaxError(%d:%d) Token=%s ExpectedTokens=[%s]",
        token.position.first, token.position.second,
        token.GetString().c_str(), e_str.c_str());
    }

  case ParseErrorType::kInternalError:
    return utf8_format("InternalError(%d:%d) State=%d",
        token.position.first, token.position.second,
        state->index);
  }

  return utf8_string();
}

Parser::Parser(const Grammar& grammar)
    : grammar_(grammar)
    , lexer_(grammar)
    , trim_reduction_(false) {
}

Parser::~Parser() {
}

bool Parser::LoadFile(const PATHCHAR* file_path) {
  if (lexer_.LoadFile(file_path)) {
    ResetState();
    return true;
  } else {
    return false;
  }
}

bool Parser::LoadString(const char* buf) {
  if (lexer_.LoadString(buf)) {
    ResetState();
    return true;
  } else {
    return false;
  }
}

bool Parser::LoadBuffer(const byte* buf, size_t size) {
  if (lexer_.LoadBuffer(buf, size)) {
    ResetState();
    return true;
  } else {
    return false;
  }
}

void Parser::ResetCursor() {
  lexer_.ResetCursor();
  ResetState();
}

std::shared_ptr<LexerBuffer> Parser::ReleaseBuffer() {
  return lexer_.ReleaseBuffer();
}

ParseResultType::T Parser::ParseStep() {
  if (token_used_) {
    ReadToken(&token_);
    token_used_ = false;
  }

  if (token_.symbol->type == SymbolType::kError) {
    error_info_ = ParseErrorInfo(ParseErrorType::kLexicalError,
                                 GetPosition(), state_, token_);
    return ParseResultType::kError;
  }

  const LALRAction* fa = state_->jmp_table[token_.symbol->index];
  if (fa == nullptr) {
    error_info_ = ParseErrorInfo(ParseErrorType::kSyntaxError,
                                 GetPosition(), state_, token_);
    for (auto i = state_->actions.begin(),
              i_end = state_->actions.end();
         i != i_end; i++) {
      const Symbol& symbol = grammar_.symbols[i->first];
      if (symbol.type == SymbolType::kTerminal ||
          symbol.type == SymbolType::kEndOfFile ||
          symbol.type == SymbolType::kGroupStart ||
          symbol.type == SymbolType::kGroupEnd) {
        error_info_.expected_symbols.push_back(&symbol);
      }
    }
    return ParseResultType::kError;
  }

  const LALRAction& action = *fa;
  if (action.type == LALRActionType::kShift) {
    // Shift
    state_ = &grammar_.lalr_states[action.target];
    ParseItem item = { state_, nullptr, token_, nullptr };
    stack_.push_back(item);
    token_used_ = true;
    return ParseResultType::kShift;
  } else if (action.type == LALRActionType::kReduce) {
    // Reduce/Production
    const Production& production = grammar_.productions[action.target];
    bool trimmed =
        trim_reduction_ &&
        production.handles.size() == 1 &&
        production.handle_refs[0]->type == SymbolType::kNonTerminal;
    const LALRState* top_state;
    if (trimmed) {
      top_state = stack_[stack_.size() - 2].state;
    } else {
      reduction_handles_.assign(stack_.end() - production.handles.size(),
                                stack_.end());
      stack_.resize(stack_.size() - production.handles.size());
      top_state = stack_.back().state;

      reduction_.production = &production;
      reduction_.handles = &reduction_handles_;
    }
    // Reduce/Goto
    const LALRAction* ga = top_state->jmp_table[production.head];
    if (ga == nullptr) {
      error_info_ = ParseErrorInfo(ParseErrorType::kInternalError,
                                   GetPosition(), state_, token_);
      return ParseResultType::kError;
    }
    const LALRAction& goto_action = *ga;
    if (goto_action.type != LALRActionType::kGoto) {
      error_info_ = ParseErrorInfo(ParseErrorType::kInternalError,
                                   GetPosition(), state_, token_);
      return ParseResultType::kError;
    }
    state_ = &grammar_.lalr_states[goto_action.target];
    if (trimmed) {
      stack_.back().state = state_;
      return ParseResultType::kReduceEliminated;
    } else {
      ParseItem item = { state_, &production, Token(), nullptr };
      stack_.push_back(item);
      reduction_.head = &stack_.back();
      return ParseResultType::kReduce;
    }
  } else if (action.type == LALRActionType::kGoto) {
    // Goto
    error_info_ = ParseErrorInfo(ParseErrorType::kInternalError,
                                 GetPosition(), state_, token_);
    return ParseResultType::kError;
  } else if (action.type == LALRActionType::kAccept) {
    // Accept
    return ParseResultType::kAccept;
  } else {
    // Internal Error
    error_info_ = ParseErrorInfo(ParseErrorType::kInternalError,
                                 GetPosition(), state_, token_);
    return ParseResultType::kError;
  }
}

ParseResultType::T Parser::ParseReduce() {
  while (true) {
    ParseResultType::T ret = ParseStep();
    if (ret == ParseResultType::kAccept ||
        ret == ParseResultType::kReduce ||
        ret == ParseResultType::kError) {
      return ret;
    }
  }
}

ParseResultType::T Parser::ParseAll() {
  while (true) {
    ParseResultType::T ret = ParseStep();
    if (ret == ParseResultType::kAccept ||
        ret == ParseResultType::kError) {
      return ret;
    }
  }
}

void Parser::ResetState() {
  state_ = &grammar_.lalr_states[grammar_.lalr_init];
  token_ = Token();
  token_used_ = true;
  ParseItem item = { state_, nullptr, token_, nullptr };
  stack_.clear();
  stack_.push_back(item);
}

void Parser::ReadToken(Token* token) {
  while (true) {
    lexer_.ReadToken(token);
    if (token->symbol->type != SymbolType::kNoise) {
      return;
    }
  }
}

const LALRState* Parser::GetState() const {
  return state_;
}

const ParseItem& Parser::GetTop() const {
  return stack_.back();
}

const std::vector<ParseItem>& Parser::GetStack() const {
  return stack_;
}

const Token& Parser::GetToken() const {
  return token_;
}

const ParseReduction& Parser::GetReduction() const {
  return reduction_;
}

const ParseErrorInfo& Parser::GetErrorInfo() const {
  return error_info_;
}

int Parser::GetLine() const {
  return lexer_.GetLine();
}

int Parser::GetColumn() const {
  return lexer_.GetColumn();
}

std::pair<int, int> Parser::GetPosition() const {
  return lexer_.GetPosition();
}

ProductionHandler::ProductionHandler(const Grammar& grammar)
  : grammar_(grammar),
    result_(nullptr) {
    handlers_.resize(grammar_.productions.size(), nullptr);
}

ProductionHandler::Handler ProductionHandler::GetHandler(
    const char* production_id) {
  const Production* p = grammar_.GetProduction((const byte*)production_id);
  return (p) ? handlers_[p->index] : nullptr;
}

bool ProductionHandler::SetHandler(const char* production_id,
                                   ProductionHandler::Handler handler) {
  const Production* p = grammar_.GetProduction((const byte*)production_id);
  if (p) {
    handlers_[p->index] = handler;
    return true;
  } else {
    return false;
  }
}

void ProductionHandler::operator()(ParseResultType::T ret,
                                   const Parser& parser) {
  if (ret == cppauparser::ParseResultType::kReduce) {
    const ParseReduction& reduction = parser.GetReduction();
    Handler handler = handlers_[parser.GetReduction().production->index];
    if (handler) {
      reduction.head->data = handler(*reduction.handles);
    } else {
      reduction.head->data = (*reduction.handles)[0].data;
    }
  } else if (ret == cppauparser::ParseResultType::kAccept) {
    result_ = parser.GetTop().data;
  }
}

void* ProductionHandler::GetResult() const {
  return result_;
}

}  // namespace cppauparser
