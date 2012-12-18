// Copyright 2012 Esun Kim

#include "utility.h"
#include "parser.h"

namespace cppauparser {

template<typename T>
ParseToTreeResult DoParseToTree(const Grammar& grammar, Parser& parser) {
  ParseToTreeResult ret;

  T builder;
  if (parser.ParseAll(builder) == cppauparser::ParseResultType::kAccept) {
    ret.result = builder.result;
    ret.lexer_buffer = parser.ReleaseBuffer();
    ret.node_allocator = std::make_shared<TreeNodeAllocator>();
    ret.node_allocator->Swap(builder.allocator);
  } else {
    ret.result = NULL_PTR;
    ret.error_info = parser.GetErrorInfo();
  }

  return ret;
}

ParseToTreeResult ParseFileToTree(const Grammar& grammar,
                                  const PATHCHAR* file_path) {
  Parser parser(grammar);
  if (parser.LoadFile(file_path) == false) {
    return ParseToTreeResult();
  }
  return DoParseToTree<TreeBuilder>(grammar, parser);
}

ParseToTreeResult ParseStringToTree(const Grammar& grammar, const char* s) {
  Parser parser(grammar);
  if (parser.LoadString(s) == false) {
    return ParseToTreeResult();
  }
  return DoParseToTree<TreeBuilder>(grammar, parser);
}

ParseToTreeResult ParseBufferToTree(const Grammar& grammar,
                                    const byte* buf,
                                    size_t size) {
  Parser parser(grammar);
  if (parser.LoadBuffer(buf, size) == false) {
    return ParseToTreeResult();
  }
  return DoParseToTree<TreeBuilder>(grammar, parser);
}

ParseToTreeResult ParseFileToSTree(const Grammar& grammar,
                                   const PATHCHAR* file_path) {
  Parser parser(grammar);
  if (parser.LoadFile(file_path) == false) {
    return ParseToTreeResult();
  }
  return DoParseToTree<SimplifiedTreeBuilder>(grammar, parser);
}

ParseToTreeResult ParseStringToSTree(const Grammar& grammar, const char* s) {
  Parser parser(grammar);
  if (parser.LoadString(s) == false) {
    return ParseToTreeResult();
  }
  return DoParseToTree<SimplifiedTreeBuilder>(grammar, parser);
}

ParseToTreeResult ParseBufferToSTree(const Grammar& grammar,
                                     const byte* buf,
                                     size_t size) {
  Parser parser(grammar);
  if (parser.LoadBuffer(buf, size) == false) {
    return ParseToTreeResult();
  }
  return DoParseToTree<SimplifiedTreeBuilder>(grammar, parser);
}

}  // namespace cppauparser
