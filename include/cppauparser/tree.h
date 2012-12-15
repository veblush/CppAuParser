// Copyright 2012 Esun Kim

#ifndef _CPPAUPARSER_TREE_H_
#define _CPPAUPARSER_TREE_H_

#include "base.h"
#include "grammar.h"
#include "lexer.h"
#include "parser.h"
#include <memory>
#include <vector>

namespace cppauparser {

struct CppAuParserDecl TreeNode {
  const Production* production;

 public:
  explicit TreeNode(const Production* production);

  bool IsTerminal() const;
  bool IsNonTerminal() const;

  void Dump(int depth = 0) const;
};

struct CppAuParserDecl TreeNodeTerminal : public TreeNode {
  Token token;

 public:
  explicit TreeNodeTerminal(const Token& token);
};

struct CppAuParserDecl TreeNodeNonTerminal : public TreeNode {
  int child_count;
  TreeNode* childs[1];

 public:
  explicit TreeNodeNonTerminal(const Production* production, int child_count);
  static size_t CalculateObjectSize(int child_count);
};

class CppAuParserDecl TreeNodeAllocator {
 public:
  TreeNodeAllocator();
  ~TreeNodeAllocator();

  TreeNode* Alloc(size_t size);
  TreeNodeTerminal* Create(const Token& token);
  TreeNodeNonTerminal* Create(const Production* production, int child_count);

  void Clear();
  void Swap(TreeNodeAllocator& a);

 private:
  size_t block_size_;
  std::vector<void*> blocks_;
  void* cur_;
  size_t cur_left_;

  CPPAUPARSER_UNCOPYABLE(TreeNodeAllocator);
};

class CppAuParserDecl TreeBuilder {
 public:
  TreeBuilder();
  void operator()(ParseResultType::T ret, const Parser& parser);

 public:
  TreeNode* result;
  TreeNodeAllocator allocator;
};

class CppAuParserDecl SimplifiedTreeBuilder {
 public:
  SimplifiedTreeBuilder();
  void operator()(ParseResultType::T ret, const Parser& parser);

 public:
  TreeNode* result;
  TreeNodeAllocator allocator;

 private:
  void PopListNode();
  TreeNodeNonTerminal* PopListNodeAndMove();

  struct ChildCandidate {
    const ParseItem* item;
    TreeNode* node;
  };
  std::vector<ChildCandidate> ccs;

  struct ListNode {
    TreeNodeNonTerminal* node;
    byte* buf;
    int max_childs;
  };
  std::vector<ListNode> lns;
  TreeNodeNonTerminal* ln_cn;
};

}  // namespace cppauparser

#endif  // _CPPAUPARSER_TREE_H_
