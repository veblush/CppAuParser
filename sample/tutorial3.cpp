// Copyright 2012 Esun Kim

#include <cppauparser/all.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
  // load grammar

  cppauparser::Grammar grammar;
  if (grammar.LoadFile(PATHSTR("data/operator.egt")) == false) {
    printf("fail to open a grammar file\n");
    return 1;
  }

  // parse with building a parse-tree

  cppauparser::Parser parser(grammar);
  parser.LoadString("-2*(3+4)-5");

  cppauparser::TreeBuilder builder;
  if (parser.ParseAll(builder) == cppauparser::ParseResultType::kAccept) {
    builder.result->Dump();
    printf("\n");
  } else {
    printf("Error\t%s\n", parser.GetErrorInfo().GetString().c_str());
    return 1;
  }

  // parse a string with a ProductionHandler

  struct Evaluator {
    static int eval(const cppauparser::TreeNode* node) {
      const cppauparser::TreeNodeNonTerminal* nt = static_cast<const cppauparser::TreeNodeNonTerminal*>(node);
      const cppauparser::TreeNode* const * c = &nt->childs[0];
      switch (node->production->index) {
      case 0: // <E> ::= <E> + <M>
        return eval(c[0]) + eval(c[2]);
      case 1: // <E> ::= <E> - <M>
        return eval(c[0]) - eval(c[2]);
      case 2: // <E> ::= <M>
        return eval(c[0]);
      case 3: // <M> ::= <M> * <N>
        return eval(c[0]) * eval(c[2]);
      case 4: // <M> ::= <M> / <N>
        return eval(c[0]) / eval(c[2]);
      case 5: // <M> ::= <N>
        return eval(c[0]);
      case 6: // <N> ::= - <V>
        return -eval(c[1]);
      case 7: // <N> ::= <V>
        return eval(c[0]);
      case 8: // <V> ::= Num
        return atoi((const char*)static_cast<const cppauparser::TreeNodeTerminal*>(c[0])->token.lexeme.c_str());
      case 9: // <V> ::= ( <E> )
        return eval(c[1]);
        break;
      default:
        return 0;
      }
    }
  };

  int result = Evaluator::eval(builder.result);
  printf("Result = %d\n", result);
  
  return 0;
}
