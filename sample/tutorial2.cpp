// Copyright 2012 Esun Kim

#include <cppauparser/all.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
  // load grammar

  cppauparser::Grammar grammar;
  if (grammar.LoadFile(PATHSTR("data\\operator.egt")) == false) {
    printf("fail to open a grammar file\n");
    return 1;
  }

  // parse a string with a ProductionHandler

  cppauparser::ProductionHandler ph(grammar);
  PH_ON(ph, "<E> ::= <E> + <M>", return (void*)((int)c[0].data + (int)c[2].data););
  PH_ON(ph, "<E> ::= <E> - <M>", return (void*)((int)c[0].data - (int)c[2].data););
  PH_ON(ph, "<E> ::= <M>",       return c[0].data;);
  PH_ON(ph, "<M> ::= <M> * <N>", return (void*)((int)c[0].data * (int)c[2].data););
  PH_ON(ph, "<M> ::= <M> / <N>", return (void*)((int)c[0].data / (int)c[2].data););
  PH_ON(ph, "<M> ::= <N>",       return c[0].data;);
  PH_ON(ph, "<N> ::= - <V>",     return (void*)-(int)c[1].data; );
  PH_ON(ph, "<N> ::= <V>",       return c[0].data;);
  PH_ON(ph, "<V> ::= Num",       return (void*)atoi((char*)c[0].token.lexeme.c_str()););
  PH_ON(ph, "<V> ::= ( <E> )",   return c[1].data;);

  cppauparser::Parser parser(grammar);
  parser.LoadString("-2*(3+4)-5");
  parser.ParseAll(ph);
  printf("result=%d\n", (int)ph.GetResult());

  return 0;
}
