#include <stdio.h>
#include <Windows.h>
#include <cppauparser/all.h>

void test1() {
  cppauparser::Grammar grammar;
  bool ret = grammar.LoadFile("data\\operator.egt");

  cppauparser::Lexer lexer(grammar);
  lexer.LoadFile("data\\operator_sample_1.txt");

  cppauparser::Lexer::Token token;
  while (true) {
    lexer.ReadToken(&token);
    if (token.symbol == grammar.symbol_EOF) {
      printf("<EOF>\n");
      break;
    } else if (token.symbol == grammar.symbol_Error) {
      printf("ERROR\t%s\n", token.lexeme.get_string().c_str());
      break;
    } else {
      printf("TOKEN\t%s\n", token.GetString().c_str());
    }
  }
}

void test2() {
  cppauparser::Grammar grammar;
  bool ret = grammar.LoadFile("data\\operator.egt");

  cppauparser::Parser parser(grammar);
  parser.LoadFile("data\\operator_sample_1.txt");

  while (true) {
    cppauparser::Parser::ParseResultType::T ret = parser.ParseStep();
    
    bool done = false;
    switch (ret) {
    case cppauparser::Parser::ParseResultType::kAccept: {
        printf("Accept\n");
        done = true;
      }
      break;
    case cppauparser::Parser::ParseResultType::kShift: {
        const cppauparser::Lexer::Token& t = parser.GetToken();
        printf("Shift\t%s\n", t.GetString().c_str());
      }
      break;
    case cppauparser::Parser::ParseResultType::kReduce: {
        const cppauparser::Parser::Reduction& r = parser.GetReduction();
        printf("Reduce\t%s\n", r.GetString().c_str());
      }
      break;
    case cppauparser::Parser::ParseResultType::kReduceEliminated: {
        printf("ReduceEliminated\n");
      }
      break;
    case cppauparser::Parser::ParseResultType::kError: {
        const cppauparser::Parser::ParseErrorInfo& e = parser.GetErrorInfo();
        printf("Error\t%s\n", e.GetString().c_str());
        done = true;
      }
      break;
    }
    if (done) {
      break;
    }
  }

  printf("done.\n");
}

struct ParseTreeNode {
  ParseTreeNode() {
    production = nullptr;
    token.symbol = nullptr;
  }
  
  ~ParseTreeNode() {
    for (auto i = childs.begin(), i_end = childs.end(); i != i_end; ++i) {
      delete *i;
    }
  }

  void dump(const cppauparser::Grammar& grammar, int depth=0) {
    for (int i=0; i<depth; i++) {
      printf("  ");
    }
    if (production) {
      printf("%s\n", grammar.symbols[production->head].name.c_str());
    } else {
      printf("%s '%s'\n", 
        token.symbol->name.c_str(),
        token.lexeme.get_string().c_str());
    }
    for (auto i = childs.begin(), i_end = childs.end(); i != i_end; ++i) {
      (*i)->dump(grammar, depth+1);
    }
  }

  const cppauparser::Production* production;
  cppauparser::Lexer::Token token;
  std::vector<ParseTreeNode*> childs;
};

void test3() {
  cppauparser::Grammar grammar;
  bool ret = grammar.LoadFile("data\\operator.egt");

  cppauparser::Parser parser(grammar);
  parser.LoadFile("data\\operator_sample_1.txt");

  cppauparser::TreeBuilder builder;
  if (parser.ParseAll(builder) == cppauparser::Parser::ParseResultType::kAccept) {
    builder.result->Dump();
  } else {
    printf("Error: %s\n", parser.GetErrorInfo().GetString().c_str());
  }
}

void test4() {
  cppauparser::Grammar grammar;
  bool ret = grammar.LoadFile("data\\json.egt");

  cppauparser::Parser parser(grammar);
  parser.LoadFile("data\\sample.json");

  DWORD start_tick = GetTickCount();
  for (int i=0; i < 1000; i++) {
    parser.ResetCursor();
    auto ret = parser.ParseAll();
    if (ret != cppauparser::Parser::ParseResultType::kAccept) {
      printf("ERROR! %d\n", ret);
      return;
    }
  }
  DWORD end_tick = GetTickCount();
  printf("%dms\n", end_tick - start_tick);

}

int main(int argc, char* argv[]) {
  //test1();
  //test2();
  test3();
  //test4();
  return 0;
  //DWORD a = GetTickCount();
  //test();
  //DWORD b = GetTickCount();
  //printf("%d\n", b-a);

  cppauparser::Grammar grammar;
  bool ret = grammar.LoadFile("data\\operator.egt");
  //bool ret = grammar.LoadFile("data\\json.egt");
  printf("%d\n", ret);

  cppauparser::Lexer lexer(grammar);
  //lexer.LoadFile("data\\sample.json");
  //lexer.LoadFile("data\\big1.json");
  lexer.LoadFile("data\\operator_sample_1.txt");

  int x = 0;
  DWORD start_tick = GetTickCount();
  for (int i=0; i<1; i++) {
    x = 0;
    lexer.ResetCursor();
    cppauparser::Lexer::Token token;
    while (true) {
      lexer.ReadToken(&token);
      if (token.symbol == grammar.symbol_EOF) {
        //printf("<EOF>\n");
        break;
      } else if (token.symbol == grammar.symbol_Error) {
        printf("Error! %s\n", token.lexeme.get_string().c_str());
        break;
      } else {
        x += token.symbol->index;
        printf("TOKEN %s '%s'\n", token.symbol->name.c_str(), 
                                  token.lexeme.get_string().c_str());
      }
    }
  }
  DWORD end_tick = GetTickCount();
  printf("%dms %dms (%d)\n", end_tick - start_tick, (end_tick - start_tick) / 1000, x);

  return 0;
}
