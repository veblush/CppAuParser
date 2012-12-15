#include <cppauparser/all.h>
#include <stdio.h>
#include <time.h>

void test_parse(cppauparser::Grammar& grammar, const PATHCHAR* file_path, int icount) {
  cppauparser::Parser parser(grammar);
  parser.LoadFile(file_path);

  clock_t start_tick = clock();
  for (int i=0; i < icount; i++) {
    parser.ResetCursor();
    auto ret = parser.ParseAll();
    if (ret != cppauparser::ParseResultType::kAccept) {
      printf("ERROR: %s\n", parser.GetErrorInfo().GetString().c_str());
      return;
    }
  }
  clock_t end_tick = clock();
  printf("PARSE: %fs\n", double(end_tick - start_tick) / CLOCKS_PER_SEC);
}

void test_tree(cppauparser::Grammar& grammar, const PATHCHAR* file_path, int icount) {
  cppauparser::Parser parser(grammar);
  parser.LoadFile(file_path);

  clock_t start_tick = clock();
  for (int i=0; i < icount; i++) {
    parser.ResetCursor();

    cppauparser::TreeBuilder builder;
    auto ret = parser.ParseAll(builder);
    if (ret != cppauparser::ParseResultType::kAccept) {
      printf("ERROR: %s\n", parser.GetErrorInfo().GetString().c_str());
      return;
    }
  }
  clock_t end_tick = clock();
  printf("TREE:  %fs\n", double(end_tick - start_tick) / CLOCKS_PER_SEC);
}

void test_stree(cppauparser::Grammar& grammar, const PATHCHAR* file_path, int icount) {
  cppauparser::Parser parser(grammar);
  parser.LoadFile(file_path);

  clock_t start_tick = clock();
  for (int i=0; i < icount; i++) {
    parser.ResetCursor();

    cppauparser::SimplifiedTreeBuilder builder;
    auto ret = parser.ParseAll(builder);
    if (ret != cppauparser::ParseResultType::kAccept) {
      printf("ERROR: %s\n", parser.GetErrorInfo().GetString().c_str());
      return;
    }
  }
  clock_t end_tick = clock();
  printf("STREE: %fs\n", double(end_tick - start_tick) / CLOCKS_PER_SEC);
}

int main(int argc, char* argv[]) {
  // load grammar

  cppauparser::Grammar grammar;
  if (grammar.LoadFile(PATHSTR("data/json.egt")) == false) {
    printf("fail to open a grammar file\n");
    return 1;
  }

  grammar.GetProduction("<Object> ::= { <Members> }")->sr_merge_child = true;
  grammar.GetProduction("<Array> ::= [ <Values> ]")->sr_merge_child = true;

  test_parse(grammar, PATHSTR("data/json_sample_3.txt"), 100);
  test_tree(grammar, PATHSTR("data/json_sample_3.txt"), 100);
  test_stree(grammar, PATHSTR("data/json_sample_3.txt"), 100);
  cppauparser::ParseFileToTree(grammar, PATHSTR("data/json_sample_1.txt")).result->Dump();
  cppauparser::ParseFileToSTree(grammar, PATHSTR("data/json_sample_1.txt")).result->Dump();
}
