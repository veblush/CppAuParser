#include <cppauparser/all.h>
#include <stdio.h>

// You can use resource system for windows platform or use adding
// extra section for linux  to embed grammar .egt file. 
// But this time to deal with compatibility the simplest way is used.
// Just convert .egt file to a c-style hexified buffer and include.
// You can make your own string by following command:
//   hexify.py -w 79 data/operator.egt > tutorial5_grammar.str

const char operator_grammar_buf[] =
  #include "tutorial5_grammar.str"
  ;

int main(int argc, char* argv[]) {
  // load grammar

  cppauparser::Grammar grammar;
  if (grammar.LoadBuffer(operator_grammar_buf,
                         sizeof(operator_grammar_buf) - 1) == false) {
    printf("fail to open a grammar file\n");
    return 1;
  }

  // parse folling string with a loaded grammar

  cppauparser::Parser parser(grammar);
  parser.LoadString("-2*(3+4)-5");

  cppauparser::TreeBuilder builder;
  parser.LoadString("-2*(3+4)-5");
  if (parser.ParseAll(builder) == cppauparser::ParseResultType::kAccept) {
    builder.result->Dump();
  } else {
    printf("Error\t%s\n", parser.GetErrorInfo().GetString().c_str());
  }

  return 0;
}
