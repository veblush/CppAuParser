// Copyright 2012 Esun Kim

#include "lexer.h"
#include <utility>
#include <algorithm>

namespace cppauparser {

Token::Token()
    : symbol(nullptr)
    , position(std::make_pair(0, 0)) {
}

Token::Token(const Symbol* symbol,
                    utf8_substring lexeme,
                    std::pair<int, int> position)
    : symbol(symbol)
    , lexeme(lexeme)
    , position(position) {
}

utf8_string Token::GetString() const {
  return utf8_format("%s '%s'", symbol->GetID().c_str(),
                                lexeme.get_string().c_str());
}

LexerBuffer::LexerBuffer()
    : buf_(nullptr),
      buf_size_(0),
      buf_sharable_(false) {
}

LexerBuffer::~LexerBuffer() {
  Clear();
}

byte* LexerBuffer::GetBuffer() {
  return buf_;
}

size_t LexerBuffer::GetBufferSize() {
  return buf_size_;
}

void LexerBuffer::SetBuffer(byte* buf, size_t size, bool sharable) {
  Clear();
  buf_ = buf;
  buf_size_ = size;
  buf_sharable_ = sharable;
}

void LexerBuffer::Clear() {
  if (buf_) {
    if (buf_sharable_ == false) {
      free(buf_);
    }
    buf_ = nullptr;
  }
  buf_size_ = 0;
  buf_sharable_ = false;
}

void LexerBuffer::Swap(LexerBuffer& b) {
  std::swap(buf_, b.buf_);
  std::swap(buf_size_, b.buf_size_);
  std::swap(buf_sharable_, b.buf_sharable_);
}

Lexer::Lexer(const Grammar& grammar)
    : grammar_(grammar)
    , buf_(nullptr)
    , buf_cur_(nullptr)
    , buf_end_(nullptr)
    , buf_peek_(nullptr)
    , line_(0)
    , column_(0) {
}

Lexer::~Lexer() {
  Unload();
}

bool Lexer::LoadFile(const PATHCHAR* file_path) {
  Unload();

  FILE* fp = PATHOPEN(file_path, PATHSTR("rb"));
  fseek(fp, 0, SEEK_END);
  size_t size = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  buf_ = reinterpret_cast<byte*>(malloc(size));
  fread(buf_, size, 1, fp);
  fclose(fp);

  allocator_.SetBuffer(buf_, size, false);

  buf_cur_ = buf_;
  buf_end_ = buf_ + size;
  line_ = 1;
  column_ = 1;
  return true;
}

bool Lexer::LoadString(const char* buf) {
  return LoadBuffer(reinterpret_cast<const byte*>(buf), strlen(buf));
}

bool Lexer::LoadBuffer(const byte* buf, size_t size) {
  Unload();

  allocator_.SetBuffer(const_cast<byte*>(buf), size, true);

  buf_cur_ = buf_ = const_cast<byte*>(buf);
  buf_end_ = buf_ + size;
  line_ = 1;
  column_ = 1;
  return true;
}

void Lexer::Unload() {
  if (buf_) {
    allocator_.Clear();
    buf_ = nullptr;
    buf_cur_ = nullptr;
    buf_end_ = nullptr;
  }
}

void Lexer::ResetCursor() {
  buf_cur_ = buf_;
  line_ = 1;
  column_ = 1;
}

std::shared_ptr<LexerBuffer> Lexer::ReleaseBuffer() {
  std::shared_ptr<LexerBuffer> b = std::make_shared<LexerBuffer>();
  b->Swap(allocator_);
  return b;
}

void Lexer::PeekToken(Token* token) {
  const DFAState* state = &grammar_.dfa_states[grammar_.dfa_init];
  byte* cur = buf_cur_;
  int hit_symbol = -1;
  byte* hit_cur = nullptr;
  while (cur < buf_end_) {
    int32_t c = *cur;
    if (c < 0x80) {
      cur += 1;
      int target = state->jmp_table[c];
      if (target == -3) {
        continue;
      } else if (target == -2) {
        hit_cur = cur;
        continue;
      } else if (target == -1) {
        break;
      } else {
        state = &grammar_.dfa_states[target];
        if (state->accept_symbol != -1) {
          hit_symbol = state->accept_symbol;
          hit_cur = cur;
        }
      }
    } else {
      if (c < 0xE0) {
        c = ((c & 0x1F) << 6) | (cur[1] & 0x3F);
        cur += 2;
      } else if (c < 0xF0) {
        c = ((c & 0x0F) << 12) | ((cur[1] & 0x3F) << 6) |
            (cur[2] & 0x3F);
        cur += 3;
      } else {
        c = ((c & 0x07) << 18) | ((cur[1] & 0x3F) << 12) |
            ((cur[2] & 0x3F) << 6) | (cur[3] & 0x3F);
        c = 0xFFFF;
        cur += 4;
      }
      int target = -1;
      for (auto j = state->jmp_ranges.begin(),
                j_end = state->jmp_ranges.end();
           j != j_end; ++j) {
        if (c >= j->range_from && c <= j->range_to) {
          target = j->target;
          break;
        }
      }
      if (target == -3) {
        continue;
      } else if (target == -2) {
        continue;
      } else if (target == -1) {
        break;
      } else {
        state = &grammar_.dfa_states[target];
        if (state->accept_symbol != -1) {
          hit_symbol = state->accept_symbol;
          hit_cur = cur;
        }
      }
    }
  }

  if (hit_symbol != -1) {
    buf_peek_ = hit_cur;
    token->symbol = &grammar_.symbols[hit_symbol];
    token->lexeme = utf8_substring(buf_cur_, (hit_cur - buf_cur_));
    token->position = GetPosition();
  } else {
    buf_peek_ = cur;
    if (cur == buf_cur_) {
      token->symbol = grammar_.symbol_EOF;
      token->lexeme = utf8_substring();
      token->position = GetPosition();
    } else {
      token->symbol = grammar_.symbol_Error;
      token->lexeme = utf8_substring(buf_cur_, (cur - buf_cur_));
      token->position = GetPosition();
    }
  }
}

void Lexer::AdvancePeekBuffer() {
  AdvanceBuffer(buf_peek_ - buf_cur_);
}

void Lexer::AdvanceBuffer(size_t n) {
  byte* buf_next = buf_cur_ + n;
  for (; buf_cur_ < buf_next; ++buf_cur_) {
    if (*buf_cur_ == 0x13) {
      line_ += 1;
      column_ = 1;
    } else {
      column_ += 1;
    }
  }
}

void Lexer::ReadToken(Token* token) {
  while (true) {
    PeekToken(token);

    const Symbol* symbol = token->symbol;
    SymbolType::T symbol_type = symbol->type;

    bool nest_group;
    const SymbolGroup* symbol_group;
    if (symbol_type == SymbolType::kGroupStart) {
      for (auto i = grammar_.symbol_groups.begin(),
                i_end = grammar_.symbol_groups.end();
           i != i_end; ++i) {
        if (i->start == symbol->index) {
          symbol_group = &*i;
          break;
        }
      }
      if (group_stack_.empty()) {
        nest_group = true;
      } else {
        nest_group = false;
        for (auto i = group_stack_.back().symbol_group->nesting_groups.begin(),
                  i_end = group_stack_.back().symbol_group->nesting_groups.end();
             i != i_end; ++i) {
          if (*i == symbol_group->index) {
            nest_group = true;
            break;
          }
        }
      }
    } else {
      nest_group = false;
    }

    if (nest_group) {
      // into nested
      AdvancePeekBuffer();
      Group g = { symbol_group, token->lexeme };
      group_stack_.push_back(g);
    } else if (group_stack_.empty()) {
      // token in plain
      AdvancePeekBuffer();
      return;
    } else if (group_stack_.back().symbol_group->end == symbol->index) {
      // out of nested
      Group pop = group_stack_.back();
      group_stack_.pop_back();
      if (pop.symbol_group->ending_mode == EndingModeType::kClosed) {
        pop.text = utf8_substring(
            pop.text.c_str(),
            token->lexeme.c_str() - pop.text.c_str() + token->lexeme.size());
        AdvancePeekBuffer();
      } else {
        pop.text = utf8_substring(
            pop.text.c_str(),
            token->lexeme.c_str() - pop.text.c_str());
      }

      if (group_stack_.empty()) {
        token->symbol = &grammar_.symbols[pop.symbol_group->container];
        token->lexeme = pop.text;
      }
      return;
    } else if (symbol_type == SymbolType::kEndOfFile) {
      // EOF in nested
      return;
    } else {
      // token in nested
      if (group_stack_.back().symbol_group->advance_mode == AdvanceModeType::kToken) {
        // delay adding a token to lexeme until "out of nested"
        AdvancePeekBuffer();
      } else {
        // delay adding a char to lexeme until "out of nested"
        AdvanceBuffer(1);
      }
    }
  }
}

int Lexer::GetLine() const {
  return line_;
}

int Lexer::GetColumn() const {
  return column_;
}

std::pair<int, int> Lexer::GetPosition() const {
  return std::make_pair(line_, column_);
}

}
