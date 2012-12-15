#include <cppauparser/all.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
  // load grammar

  cppauparser::Grammar grammar;
  if (grammar.LoadFile(PATHSTR("data/operator.egt")) == false) {
    printf("fail to open a grammar file\n");
    return 1;
  }

  // parse with printing events

  cppauparser::Parser parser(grammar);
  parser.LoadString("-2*(3+4)-5");
  parser.ParseAll([&](cppauparser::ParseResultType::T ret,
                      cppauparser::Parser& parser) {
    switch (ret) {
    case cppauparser::ParseResultType::kAccept:
      printf("Accept\t\n");
      break;
    case cppauparser::ParseResultType::kShift:
      printf("Shift\t%s\n", parser.GetTop().GetString().c_str());
      break;
    case cppauparser::ParseResultType::kReduce:
      printf("Reduce\t%s\n", parser.GetReduction().GetString().c_str());
      break;
    case cppauparser::ParseResultType::kReduceEliminated:
      printf("ReduceEliminated\t\n");
      break;
    case cppauparser::ParseResultType::kError:
      printf("Error\t%s\n", parser.GetErrorInfo().GetString().c_str());
      break;
    }
  });

  // parse with building a parse-tree

  cppauparser::TreeBuilder builder;
  parser.LoadString("-2*(3+4)-5");
  if (parser.ParseAll(builder) == cppauparser::ParseResultType::kAccept) {
    builder.result->Dump();
  } else {
    printf("Error\t%s\n", parser.GetErrorInfo().GetString().c_str());
  }

  return 0;
}
