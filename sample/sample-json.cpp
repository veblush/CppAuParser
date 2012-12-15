#include <cppauparser/all.h>
#include <stdio.h>
#include <list>
#include <map>

bool ParseJson(const cppauparser::Grammar& grammar,
               const PATHCHAR* file_path) {

  // parse json file with a ProductionHandler

  /*
  cppauparser::ProductionHandler ph(grammar);
  PH_ON(ph, "<Json> ::= <Object>",                return c[0].data;);
  PH_ON(ph, "<Json> ::= <Array>",                 return c[0].data;);
  PH_ON(ph, "<Object> ::= { <Members> }",         return c[0].data;);
  PH_ON(ph, "<Object> ::= { }",                   return c[0].data;);
  PH_ON(ph, "<Members> ::= <Members> , <Member>", return c[0].data;);
  PH_ON(ph, "<Members> ::= <Member>",             return c[0].data;);
  PH_ON(ph, "<Member> ::= String : <Value>",      return c[0].data;);
  PH_ON(ph, "<Array> ::= [ <Values> ]",           return c[0].data;);
  PH_ON(ph, "<Array> ::= [ ]",                    return c[0].data;);
  PH_ON(ph, "<Values> ::= <Values> , <Value>",    return c[0].data;);
  PH_ON(ph, "<Values> ::= <Value>",               return c[0].data;);
  PH_ON(ph, "<Value> ::= <Object>",               return c[0].data;);
  PH_ON(ph, "<Value> ::= <Array>",                return c[0].data;);
  PH_ON(ph, "<Value> ::= Integer",                return c[0].data;);
  PH_ON(ph, "<Value> ::= Float",                  return c[0].data;);
  PH_ON(ph, "<Value> ::= String",                 return c[0].data;);
  PH_ON(ph, "<Value> ::= false",                  return c[0].data;);
  PH_ON(ph, "<Value> ::= null",                   return c[0].data;);
  PH_ON(ph, "<Value> ::= true",                   return c[0].data;);

  cppauparser::Parser parser(grammar);
  parser.LoadFile(file_path);
  parser.ParseAll(ph);
  */

  return true;
}

int main(int argc, char* argv[]) {
  // load grammar

  cppauparser::Grammar grammar;
  if (grammar.LoadFile(PATHSTR("data/json.egt")) == false) {
    printf("fail to open a grammar file\n");
    return false;
  }

  grammar.GetProduction("<Object> ::= { <Members> }")->sr_merge_child = true;
  grammar.GetProduction("<Array> ::= [ <Values> ]")->sr_merge_child = true;

  // show tree

  cppauparser::ParseFileToSTree(grammar, PATHSTR("data/json_sample_1.txt")).result->Dump();

  // load json by parsing

  ParseJson(grammar, PATHSTR("data/json_sample_1.txt"));

  return 0;
}
