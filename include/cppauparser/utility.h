// Copyright [year] <Copyright Owner>

#ifndef _CPPAUPARSER_UTILITY_H_
#define _CPPAUPARSER_UTILITY_H_

#include "base.h"
#include "grammar.h"
#include "tree.h"

namespace cppauparser {

struct ParseToTreeResult {
  TreeNode* result;
  ParseErrorInfo error_info;
  std::shared_ptr<LexerBuffer> lexer_buffer;
  std::shared_ptr<TreeNodeAllocator> node_allocator;
};

CppAuParserDecl ParseToTreeResult ParseFileToTree(const Grammar& grammar,
                                                  const PATHCHAR* file_path);

CppAuParserDecl ParseToTreeResult ParseStringToTree(const Grammar& grammar,
                                                    const char* s);

CppAuParserDecl ParseToTreeResult ParseBufferToTree(const Grammar& grammar,
                                                    const byte* buf,
                                                    size_t size);

CppAuParserDecl ParseToTreeResult ParseFileToSTree(const Grammar& grammar,
                                                   const PATHCHAR* file_path);

CppAuParserDecl ParseToTreeResult ParseStringToSTree(const Grammar& grammar,
                                                     const char* s);

CppAuParserDecl ParseToTreeResult ParseBufferToSTree(const Grammar& grammar,
                                                     const byte* buf,
                                                     size_t size);

}  // namespace cppauparser

#endif  // _CPPAUPARSER_UTILITY_H_
