// Copyright 2012 Esun Kim

#include "tree.h"
#include <string.h>
#include <vector>
#include <algorithm>

namespace cppauparser {

TreeNode::TreeNode(const Production* production)
    : production(production) {
}

bool TreeNode::IsTerminal() const {
  return production == NULL_PTR;
}

bool TreeNode::IsNonTerminal() const {
  return production != NULL_PTR;
}

void TreeNode::Dump(int depth) const {
  for (int i = 0; i < depth; i++) {
    printf("  ");
  }
  if (IsTerminal()) {
    const TreeNodeTerminal* n = static_cast<const TreeNodeTerminal*>(this);
    printf("%s\n", n->token.GetString().c_str());
  } else {
    const TreeNodeNonTerminal* n = static_cast<const TreeNodeNonTerminal*>(this);
    printf("%s\n", n->production->GetID().c_str());
    for (int i = 0; i < n->child_count; i++) {
      n->childs[i]->Dump(depth + 1);
    }
  }
}

TreeNodeTerminal::TreeNodeTerminal(const Token& token)
    : TreeNode(NULL_PTR),
      token(token) {
}

TreeNodeNonTerminal::TreeNodeNonTerminal(const Production* production,
                                         int child_count)
    : TreeNode(production),
      child_count(child_count) {
}

size_t TreeNodeNonTerminal::CalculateObjectSize(int child_count) {
  return sizeof(TreeNodeNonTerminal)
      - sizeof(TreeNode*)
      + (child_count * sizeof(TreeNode*));
}

TreeNodeAllocator::TreeNodeAllocator()
    : block_size_(4096),
      cur_(NULL_PTR),
      cur_left_(0) {
}

TreeNodeAllocator::~TreeNodeAllocator() {
  Clear();
}

TreeNode* TreeNodeAllocator::Alloc(size_t size) {
  if (size > cur_left_) {
    cur_ = malloc(block_size_);
    cur_left_ = block_size_;
    blocks_.push_back(cur_);
  }

  void* ret = cur_;
  cur_ = reinterpret_cast<char*>(cur_) + size;
  cur_left_ -= size;
  return reinterpret_cast<TreeNode*>(ret);
}


TreeNodeTerminal* TreeNodeAllocator::Create(const Token& token) {
  TreeNodeTerminal* n = static_cast<TreeNodeTerminal*>(
      Alloc(sizeof(TreeNodeTerminal)));
  new (n) TreeNodeTerminal(token);
  return n;
}

TreeNodeNonTerminal* TreeNodeAllocator::Create(const Production* production,
                                               int child_count) {
  TreeNodeNonTerminal* n = static_cast<TreeNodeNonTerminal*>(
      Alloc(TreeNodeNonTerminal::CalculateObjectSize(child_count)));
  new (n) TreeNodeNonTerminal(production, child_count);
  return n;
}

void TreeNodeAllocator::Clear() {
  for (auto i = blocks_.begin(), i_end = blocks_.end(); i != i_end; ++i)
    free(*i);
  cur_ = NULL_PTR;
  cur_left_ = 0;
}

void TreeNodeAllocator::Swap(TreeNodeAllocator& a) {
  std::swap(block_size_, a.block_size_);
  std::swap(blocks_, a.blocks_);
  std::swap(cur_, a.cur_);
  std::swap(cur_left_, a.cur_left_);
}

TreeBuilder::TreeBuilder()
    : result(NULL_PTR) {
}

void TreeBuilder::operator()(ParseResultType::T ret,
                             const Parser& parser) {
  if (ret == ParseResultType::kShift) {
    TreeNodeTerminal* node = allocator.Create(parser.GetToken());
    parser.GetTop().data = node;
  } else if (ret == ParseResultType::kReduce) {
    const ParseReduction& reduction = parser.GetReduction();
    int child_count = static_cast<int>(reduction.handles->size());
    TreeNodeNonTerminal* node = allocator.Create(reduction.production,
                                                 child_count);
    for (int i = 0; i < child_count; i++) {
      node->childs[i] = reinterpret_cast<TreeNode*>((*reduction.handles)[i].data);
    }
    reduction.head->data = node;
  } else if (ret == ParseResultType::kAccept) {
    result = reinterpret_cast<TreeNode*>(parser.GetTop().data);
  }
}

SimplifiedTreeBuilder::SimplifiedTreeBuilder()
    : result(NULL_PTR),
      ln_cn(NULL_PTR) {
}

void SimplifiedTreeBuilder::operator()(ParseResultType::T ret,
                                       const Parser& parser) {
  if (ret == ParseResultType::kReduce) {
    const ParseReduction& r = parser.GetReduction();
    const Production* p = r.production;
    const std::vector<ParseItem>& hs = *r.handles;

    // make all handles into a list of child candidate.
    // in making lists, create terminal nodes if exist
    // because nothing is done in a shift event.
    if (p->sr_remove_single_lexeme) {
      // remove symbols which consist of only a single lexeme.
      int j = 0;
      ccs.resize(hs.size());
      for (size_t i = 0, i_end = hs.size(); i < i_end; i++) {
        if (hs[i].production ||
            hs[i].token.symbol->single_lexeme == false) {
          ccs[j].item = &hs[i];
          ccs[j].node = (hs[i].data)
              ? reinterpret_cast<TreeNode*>(hs[i].data)
              : allocator.Create(hs[i].token);
          j += 1;
        }
      }
      ccs.resize(j);
    } else {
      ccs.resize(hs.size());
      for (size_t i = 0, i_end = hs.size(); i < i_end; i++) {
        ccs[i].item = &hs[i];
        ccs[i].node = (hs[i].data)
            ? reinterpret_cast<TreeNode*>(hs[i].data)
            : allocator.Create(hs[i].token);
      }
    }

    // forward a child node and drop me
    if (p->sr_forward_child && hs.size() == 1) {
      if (ccs[0].node == ln_cn) {
        r.head->data = PopListNodeAndMove();
      } else {
        r.head->data = ccs[0].node;
      }
      return;
    }

    // change a recursive child node to a flat list
    if (p->sr_listify_recursion) {
      int fi = -1;
      for (int i = 0; i < int(ccs.size()); i++) {
        if (ccs[i].item->production &&
            ccs[i].item->production->head == p->head) {
          fi = i;
          break;
        }
      }
      if (fi != -1) {
        if (ccs[fi].item->production->index == p->index) {
          ListNode& ln = lns.back();
          int ccs_len = int(ccs.size());

          // calculate new child count and expand buffer if not sufficient
          int new_child_count = ln.node->child_count + ccs_len - 1;
          if (new_child_count > ln.max_childs) {
            ln.max_childs = std::max(new_child_count, ln.max_childs * 2);
            ln.buf = reinterpret_cast<byte*>(realloc(ln.buf,
              TreeNodeNonTerminal::CalculateObjectSize(ln.max_childs)));
            ln_cn = ln.node = reinterpret_cast<TreeNodeNonTerminal*>(ln.buf);
          }

          // move childs [0, n) -> [fi, fi+n)
          if (fi != 0) {
            for (int i = ln.node->child_count - 1; i >= 0; i--) {
              ln.node->childs[i+fi] = ln.node->childs[i];
            }
          }
          // [, fi)
          for (int i = 0; i < fi; i++) {
            ln.node->childs[i] = ccs[i].node;
          }
          // [fi+n, )
          for (int i = fi+1; i < ccs_len; i++) {
            ln.node->childs[i+ln_cn->child_count-1] = ccs[i].node;
          }

          // make merge done and return
          ln.node->child_count = new_child_count;
          r.head->data = ln_cn;
          return;
        } else if (ccs[fi].item->production->handles.empty()) {
          // remove an empty node used for a list termination
          ccs.erase(ccs.begin() + fi);
        }
      }
      // push new item on stack
      ListNode ln;
      ln.max_childs = std::max(16, int(ccs.size()));
      ln.buf = (byte*)malloc(TreeNodeNonTerminal::CalculateObjectSize(ln.max_childs));
      ln.node = new (ln.buf) TreeNodeNonTerminal(r.production, int(ccs.size()));
      for (int i = 0; i < int(ccs.size()); i++)
        ln.node->childs[i] = ccs[i].node;
      lns.push_back(ln);
      ln_cn = ln.node;
      r.head->data = ln_cn;
    } else if (p->sr_merge_child) {
      // get children of a child and drop a child
      int child_count = 0;
      for (auto i = ccs.begin(), i_end = ccs.end(); i != i_end; ++i) {
        if (i->item && i->item->production &&
            i->node && i->node->IsNonTerminal() &&
            i->item->production->index == i->node->production->index) {
          child_count += static_cast<TreeNodeNonTerminal*>(i->node)->child_count;
        } else {
          child_count += 1;
        }
      }
      // create a merged non-terminal node
      TreeNodeNonTerminal* node = allocator.Create(r.production, child_count);
      int j = 0;
      for (auto i = ccs.begin(), i_end = ccs.end(); i != i_end; ++i) {
        if (i->item && i->item->production &&
            i->node && i->node->IsNonTerminal() &&
            i->item->production->index == i->node->production->index) {
          TreeNodeNonTerminal* cnode = static_cast<TreeNodeNonTerminal*>(i->node);
          for (int k = 0; k < cnode->child_count; k++) {
            node->childs[j] = cnode->childs[k];
            j += 1;
          }
          if (cnode == ln_cn) {
            PopListNode();
          }
        } else {
          node->childs[j] = (i->node == ln_cn)
              ? PopListNodeAndMove()
              : i->node;
          j += 1;
        }
      }
      r.head->data = node;
    } else {
      // create a non-terminal node
      TreeNodeNonTerminal* node = allocator.Create(r.production, ccs.size());
      for (size_t i = 0, i_end = ccs.size(); i < i_end; i++) {
        node->childs[i] = (ccs[i].node == ln_cn)
            ? PopListNodeAndMove()
            : ccs[i].node;
      }
      r.head->data = node;
    }
  } else if (ret == ParseResultType::kAccept) {
    result = reinterpret_cast<TreeNode*>(parser.GetTop().data);
    if (result == ln_cn) {
      result = PopListNodeAndMove();
    }
  } else if (ret == ParseResultType::kAccept) {
    while (lns.empty() == false) {
      PopListNode();
    }
  }
}

void SimplifiedTreeBuilder::PopListNode() {
  free(lns.back().buf);
  lns.pop_back();
  ln_cn = lns.empty() ? NULL_PTR : lns.back().node;
}

TreeNodeNonTerminal* SimplifiedTreeBuilder::PopListNodeAndMove() {
  TreeNodeNonTerminal* n = allocator.Create(ln_cn->production, ln_cn->child_count);
  memcpy(n->childs, ln_cn->childs, n->child_count * sizeof(TreeNode*));
  PopListNode();
  return n;
}

}  // namespace cppauparser
