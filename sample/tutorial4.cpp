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

  // set following production as a formatting one,
  // it makes that specified productions will be removed when
  // building a simplified tree if possible

  grammar.GetProduction("<V> ::= ( <E> )")->sr_forward_child = true;

  // parse with building a parse-tree

  auto ret = cppauparser::ParseStringToSTree(grammar, "-2*(1+2+4)-2-2-1");
  if (ret.result) {
    ret.result->Dump();
    printf("\n");
  } else {
    printf("Error\t%s\n", ret.error_info.GetString().c_str());
    return 1;
  }

  // evaluate a simplfiied parse tree by traversing nodes

  struct Evaluator {
    static int eval(const cppauparser::TreeNode* node) {
      if (node->IsNonTerminal()) {
        const cppauparser::TreeNodeNonTerminal* nt = static_cast<const cppauparser::TreeNodeNonTerminal*>(node);
        const cppauparser::TreeNode* const * c = &nt->childs[0];
        int ret = eval(c[0]);
        switch (node->production->index) {
        case 0: // <E> ::= <E> + <M>
          for (int i = 1; i < nt->child_count; i++) {
            ret += eval(c[i]);
          }
          break;
        case 1: // <E> ::= <E> - <M>
          for (int i = 1; i < nt->child_count; i++) {
            ret -= eval(c[i]);
          }
          break;
        case 3: // <M> ::= <M> * <N>
          for (int i = 1; i < nt->child_count; i++) {
            ret *= eval(c[i]);
          }
          break;
        case 4: // <M> ::= <M> / <N>
          for (int i = 1; i < nt->child_count; i++) {
            ret /= eval(c[i]);
          }
          break;
        case 6: // <N> ::= - <V>
          ret = -ret;
          break;
        }
        return ret;
      } else {
        const cppauparser::TreeNodeTerminal* t = static_cast<const cppauparser::TreeNodeTerminal*>(node);
        return atoi((const char*)t->token.lexeme.c_str());
      }
    }
  };

  int result = Evaluator::eval(ret.result);
  printf("Result = %d\n", result);
  return 0;
}
