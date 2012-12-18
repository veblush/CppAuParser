// Copyright 2012 Esun Kim

#include "grammar.h"
#include <stdio.h>
#include <string.h>
#include <vector>
#include <algorithm>

namespace cppauparser {

utf8_string Symbol::GetID() const {
  if (type == SymbolType::kNonTerminal) {
    return utf8_format("<%s>", name.c_str());
  } else if (type == SymbolType::kTerminal) {
    return utf8_format("%s", name.c_str());
  } else {
    return utf8_format("(%s)", name.c_str());
  }
}

utf8_string Production::GetID() const {
  utf8_string ret = head_ref->GetID() + (const byte*)" ::= ";
  for (auto i = handle_refs.cbegin(),
            i_end = handle_refs.cend();
       i != i_end; ++i) {
    if (i == handle_refs.begin()) {
      ret += (*i)->GetID();
    } else {
      ret += (const byte*)" " + (*i)->GetID();
    }
  }
  return ret;
}

Grammar::Grammar() {
}

bool Grammar::LoadFile(const PATHCHAR* file_path) {
  FILE* fp = PATHOPEN(file_path, PATHSTR("rb"));
  if (fp == NULL) {
    return false;
  }

  fseek(fp, 0, SEEK_END);
  size_t size = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  char* buf = new char[size];
  fread(buf, size, 1, fp);
  fclose(fp);

  bool ret = LoadBuffer(buf, size);
  delete [] buf;

  return ret;
}

inline byte read_byte(const char** buf) {
  byte v = **buf;
  *buf += 1;
  return v;
}

inline bool read_bool(const char** buf) {
  bool v = (**buf == 1);
  *buf += 1;
  return v;
}

inline int16_t read_integer(const char** buf) {
  int16_t v = *reinterpret_cast<const int16_t*>(*buf);
  *buf += 2;
  return v;
}

inline utf8_string read_string(const char** buf) {
  utf8_string ret;
  const uint16_t* ra = convert_utf16_to_utf8_string(
      reinterpret_cast<const uint16_t*>(*buf),
      &ret);
  *buf = reinterpret_cast<const char*>(ra);
  return ret;
}

struct EntryValue {
  char type;
  bool v_bool;
  byte v_byte;
  int16_t v_int;
  utf8_string v_str;
};

inline bool read_value(const char** buf, EntryValue* value) {
  switch (**buf) {
  case 'E':
    value->type = *((*buf)++);
    break;
  case 'b':
    value->type = *((*buf)++);
    value->v_byte = read_byte(buf);
    break;
  case 'B':
    value->type = *((*buf)++);
    value->v_bool = read_bool(buf);
    break;
  case 'I':
    value->type = *((*buf)++);
    value->v_int = read_integer(buf);
    break;
  case 'S':
    value->type = *((*buf)++);
    value->v_str = read_string(buf);
    break;
  default:
    value->type = 0;
    return false;
  }

  return true;
}

bool Grammar::LoadBuffer(const char* buf, size_t len) {
  const char* buf_end = buf + len;

  utf8_string header = read_string(&buf);
  if (strcmp((const char*)header.c_str(), "GOLD Parser Tables/v5.0") != 0) {
    return false;
  }

  std::vector<EntryValue> v;
  while (buf < buf_end && read_byte(&buf) == 'M') {
    // read entries
    int16_t value_count = read_integer(&buf);
    if (v.size() < (size_t)value_count) {
      v.resize((size_t)value_count);
    }
    for (int i = 0; i < value_count; i++) {
      if (read_value(&buf, &v[i]) == false) {
        return false;
      }
    }

    // set by entries
    switch (v[0].v_byte) {
    case 'p': {
        Property o;
        o.index = v[1].v_int;
        o.name = v[2].v_str;
        o.value = v[3].v_str;
        properties.push_back(o);
      }
      break;
    case 't': {
        symbols.resize(v[1].v_int);
        charsets.resize(v[2].v_int);
        productions.resize(v[3].v_int);
        dfa_states.resize(v[4].v_int);
        lalr_states.resize(v[5].v_int);
        symbol_groups.resize(v[6].v_int);
      }
      break;
    case 'c': {
        CharacterSet& o = charsets[v[1].v_int];
        o.index = v[1].v_int;
        o.uniplane = v[2].v_int;
        for (int16_t i = 0; i < v[3].v_int; i++) {
          o.ranges.push_back(std::make_pair(
            (uint16_t)v[5+i*2].v_int,
            (uint16_t)v[6+i*2].v_int));
        }
      }
      break;
    case 'S': {
        Symbol& o = symbols[v[1].v_int];
        o.index = v[1].v_int;
        o.name = v[2].v_str;
        o.type = (SymbolType::T)v[3].v_int;
      }
      break;
    case 'g': {
        SymbolGroup& o = symbol_groups[v[1].v_int];
        o.index = v[1].v_int;
        o.name = v[2].v_str;
        o.container = v[3].v_int;
        o.start = v[4].v_int;
        o.end = v[5].v_int;
        o.advance_mode = (AdvanceModeType::T)v[6].v_int;
        o.ending_mode = (EndingModeType::T)v[7].v_int;
        for (int16_t i = 0; i < v[9].v_int; i++) {
          o.nesting_groups.push_back(v[10+i].v_int);
        }
      }
      break;
    case 'R': {
        Production& o = productions[v[1].v_int];
        o.index = v[1].v_int;
        o.head = v[2].v_int;
        for (int16_t i = 4; i < value_count; i++) {
          o.handles.push_back(v[i].v_int);
        }
      }
      break;
    case 'I': {
        dfa_init = v[1].v_int;
        lalr_init = v[2].v_int;
      }
      break;
    case 'D': {
        DFAState& o = dfa_states[v[1].v_int];
        o.index = v[1].v_int;
        o.accept_symbol = v[2].v_bool ? v[3].v_int : -1;
        for (int16_t i = 0; i < (value_count - 5) / 3; i++) {
          DFAEdge edge;
          edge.charset = v[i*3+5].v_int;
          edge.target = v[i*3+6].v_int;
          o.edges.push_back(edge);
        }
      }
      break;
    case 'L': {
        LALRState& o = lalr_states[v[1].v_int];
        o.index = v[1].v_int;
        for (int16_t i = 0; i < (value_count - 3) / 4; i++) {
          LALRAction& action = o.actions[v[i*4+3].v_int];
          action.symbol = v[i*4+3].v_int;
          action.type = (LALRActionType::T)v[i*4+4].v_int;
          action.target = v[i*4+5].v_int;
        }
      }
      break;
    default:
      return false;
    }
  }

  ProcessAfterLoad();
  return true;
}

void Grammar::ProcessAfterLoad() {
  LinkReference();
  BuildDFALookup();
  BuildLALRLookup();
  SetSingleLexemeSymbol();
  SetSimplicationRule();
}

void Grammar::LinkReference() {
  for (auto i = symbols.begin(), i_end = symbols.end(); i != i_end; ++i) {
    if (i->type == SymbolType::kEndOfFile) {
      symbol_EOF = &symbols[i->index];
    } else if (i->type == SymbolType::kError) {
      symbol_Error = &symbols[i->index];
    }
  }

  for (auto i = productions.begin(),
            i_end = productions.end();
       i != i_end; ++i) {
    i->head_ref = &symbols[i->head];
    i->handle_refs.reserve(i->handles.size());
    for (auto j = i->handles.begin(),
              j_end = i->handles.end();
         j != j_end; ++j) {
      i->handle_refs.push_back(&symbols[*j]);
    }
  }

  for (auto i = symbols.begin(),
            i_end = symbols.end();
       i != i_end; ++i) {
    symbol_pname_lookup_[i->GetID()] = &*i;
  }

  for (auto i = productions.begin(),
            i_end = productions.end();
       i != i_end; ++i) {
    production_pname_lookup_[i->GetID()] = &*i;
  }
}

void Grammar::BuildDFALookup() {
  for (auto i = dfa_states.begin(), i_end = dfa_states.end(); i != i_end; ++i) {
    DFAState& s = *i;
    memset(s.jmp_table, 0xFF, sizeof(s.jmp_table));
    for (auto j = s.edges.begin(), j_end = s.edges.end(); j != j_end; ++j) {
      const DFAEdge& e = *j;
      const CharacterSet& cset = charsets[e.charset];
      for (auto k = cset.ranges.begin(),
                k_end = cset.ranges.end();
           k != k_end; ++k) {
        int16_t target = e.target;
        if (e.target == s.index) {
          target = (s.accept_symbol != -1) ? -2 : -3;
        }
        if (k->first < 0x80) {
          for (int x = k->first; x <= std::min<uint16_t>(0x7F, k->second); ++x) {
            s.jmp_table[x] = target;
          }
        }
        if (k->second >= 0x80) {
          DFAState::JmpRange jr;
          jr.range_from = std::max<uint16_t>(0x80, k->first);
          jr.range_to = k->second;
          jr.target = target;
          s.jmp_ranges.push_back(jr);
        }
      }
    }

    struct JmpRangeLess {
      bool operator()(const DFAState::JmpRange& a,
                      const DFAState::JmpRange& b) {
        return a.range_from < b.range_from;
      }
    };
    std::sort(s.jmp_ranges.begin(), s.jmp_ranges.end(), JmpRangeLess());
  }
}

void Grammar::BuildLALRLookup() {
  for (auto i = lalr_states.begin(), i_end = lalr_states.end(); i != i_end; ++i) {
    LALRState& s = *i;
    s.jmp_table.resize(symbols.size(), NULL_PTR);
    for (auto j = s.actions.begin(), j_end = s.actions.end(); j != j_end; ++j) {
      s.jmp_table[j->first] = &j->second;
    }
  }
}

void Grammar::SetSingleLexemeSymbol() {
  // find terminals having only single lexeme.
  // (by finding dfa-state nodes has one-acyclic path from an initial state)
  std::vector<int> path_counts(dfa_states.size(), 0);
  path_counts[dfa_init] = 1;
  std::vector<int> left_nodes;
  left_nodes.push_back(dfa_init);
  while (left_nodes.empty() == false) {
    int di = left_nodes.back();
    left_nodes.pop_back();
    const DFAState* s = &dfa_states[di];
    for (auto i = s->edges.begin(), i_end = s->edges.end(); i != i_end; ++i) {
      if (path_counts[i->target] == 1) {
        // multiple path found.
        // all nodes from this have multiple paths
        std::vector<int> left_m_nodes;
        left_m_nodes.push_back(i->target);
        while (left_m_nodes.empty() == false) {
          int mi = left_m_nodes.back();
          left_m_nodes.pop_back();
          path_counts[mi] = 2;
          const DFAState* m = &dfa_states[mi];
          for (auto j = m->edges.begin(), j_end = m->edges.end(); j != j_end; ++j) {
            if (path_counts[j->target] != 2) {
              left_m_nodes.push_back(j->target);
            }
          }
        }
      } else if (path_counts[i->target] == 0) {
        // new node found. keep going on.
        path_counts[i->target] = 1;
        left_nodes.push_back(i->target);
      }
    }
  }

  std::vector<int> symbol_path_counts(symbols.size(), 0);
  for (auto i = dfa_states.begin(), i_end = dfa_states.end(); i != i_end; ++i) {
    if (path_counts[i->index] == 1 && i->accept_symbol >= 0) {
      symbol_path_counts[i->accept_symbol] += 1;
    }
  }
  for (auto i = symbols.begin(), i_end = symbols.end(); i != i_end; ++i) {
    i->single_lexeme = (symbol_path_counts[i->index] == 1);
  }
}

void Grammar::SetSimplicationRule() {
  for (auto i = productions.begin(), i_end = productions.end(); i != i_end; ++i) {
    // prepare terminals, nonterminals. effective terminals
    std::vector<const Symbol*> ts, nts, its;
    for (auto j = i->handle_refs.begin(), j_end = i->handle_refs.end(); j != j_end; ++j) {
      if ((*j)->type == SymbolType::kTerminal) {
        ts.push_back(*j);
        if ((*j)->single_lexeme == false) {
          its.push_back(*j);
        }
      } else if ((*j)->type == SymbolType::kNonTerminal) {
        nts.push_back(*j);
      }
    }

    i->sr_forward_child = ((nts.size() == 1 && ts.size() == 0) ||
                           (nts.size() == 0 && its.size() == 1) ||
                           (nts.size() == 0 && ts.size() == 1));

    i->sr_merge_child = false;

    i->sr_listify_recursion = false;
    for (auto j = nts.begin(), j_end = nts.end(); j != j_end; ++j) {
      if (i->head == (*j)->index) {
        i->sr_listify_recursion = true;
        break;
      }
    }

    i->sr_remove_single_lexeme = ((nts.size() > 0 || its.size() > 0) &&
                                  (ts.size() > its.size()));
  }
}

Symbol* Grammar::GetSymbol(const utf8_string& pname) const {
  auto i = symbol_pname_lookup_.find(pname);
  return i != symbol_pname_lookup_.end()
      ? const_cast<Symbol*>(i->second)
      : NULL_PTR;
}

Symbol* Grammar::GetSymbol(const char* id) const {
  return GetSymbol(utf8_string(reinterpret_cast<const byte*>(id)));
}

Production* Grammar::GetProduction(const utf8_string& pname) const {
  auto i = production_pname_lookup_.find(pname);
  return i != production_pname_lookup_.end()
      ? const_cast<Production*>(i->second)
      : NULL_PTR;
}

Production* Grammar::GetProduction(const char* id) const {
  return GetProduction(utf8_string(reinterpret_cast<const byte*>(id)));
}

}
