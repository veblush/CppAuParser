#include <cppauparser/all.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
  // load grammar

  cppauparser::Grammar grammar;
  if (grammar.LoadFile(PATHSTR("data/list.egt")) == false) {
    printf("fail to open a grammar file\n");
    return 1;
  }

  grammar.GetProduction("<List> ::= [ <List1> ]")->sr_merge_child = true;
  grammar.GetProduction("<List> ::= { <List2> }")->sr_merge_child = true;

  printf("********** TreeBuilder [a,b,c] **********\n");
  cppauparser::ParseStringToTree(grammar, "[a,b,c]").result->Dump();
  printf("\n");

  printf("********** SimplifiedTreeBuilder [a,b,c] **********\n");
  cppauparser::ParseStringToSTree(grammar, "[a,b,c]").result->Dump();
  printf("\n");

  printf("********** TreeBuilder {a;b;c;} **********\n");
  cppauparser::ParseStringToTree(grammar, "{a;b;c;}").result->Dump();
  printf("\n");

  printf("********** SimplifiedTreeBuilder {a;b;c;} **********\n");
  cppauparser::ParseStringToSTree(grammar, "{a;b;c;}").result->Dump();
  printf("\n");
}
