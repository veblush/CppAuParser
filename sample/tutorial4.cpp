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

  cppauparser::Parser parser(grammar);
  parser.LoadString("-2*(1+2+4)-2-2-1");

  cppauparser::SimplifiedTreeBuilder builder;
  if (parser.ParseAll(builder) == cppauparser::ParseResultType::kAccept) {
    builder.result->Dump();
    printf("\n");
  } else {
    printf("Error\t%s\n", parser.GetErrorInfo().GetString().c_str());
    return 1;
  }

  return 0;
}
