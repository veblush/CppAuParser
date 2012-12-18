===================================================
CppAuParser - GOLD Parser Engine for C++
===================================================

:Version: 0.50
:Author: Esun Kim (veblush+git_at_gmail_com)
:Download: https://github.com/veblush/CppAuParser
:Source: https://github.com/veblush/CppAuParser
:License: The MIT License `LICENSE`_
:Keywords: c++, goldparser, parser, lalr

.. contents::
    :local:

.. _LICENSE: https://github.com/veblush/CppAuParser/blob/master/LICENSE.txt

Overview
========

New C++ engine for GOLD Parser. It supports unicode and new .egt file format.

Make
=====

Building
--------

You can build cppauparser from cmake::

	$ md build
	$ cd build
	$ cmake ..
	$ make install

Alternatively you can use Visual C++ 2010 or newer to build cppauparser using msvc/all.sln.

Compatibility
-------------

You can use following compilers to build cppauparser.

 * Visual C++ 10 or newer
 * GCC 4.4 or newer
 * Clang 2.9 or newer

I want to use full c++11 features like scoped enum, lambda expression, ranged-for, etc.
But to support old compilers which support c++11 partially, only limited c++11 features is used as following:

 * auto keyword
 * std::shared_ptr in <memory>

If you continue to support old compilers, be careful of using c++11 features.

Tutorial
========

Prepare a Grammar
-----------------

First we need a grammar of language for parsing. Pyauparser use a .egt file which is
compiled from .grm file which consists of text-formatted grammar definitions.

You can write a .grm file with GOLD-Parser_ or alternatives with GOLD-Meta-Language_.
And instead of writing you can find lots of grammar files of popular languages at Grammars_.

.. _GOLD-Parser: http://www.goldparser.org
.. _GOLD-Meta-Language: http://goldparser.org/doc/grammars/index.htm
.. _Grammars: http://goldparser.org/grammars/index.htm

Let's start with a simple calculator grammar. It consists of number, +, -, ``*``, /, unary -, parenthesis. ::

	"Start Symbol" = <E>

	{Digi9} = {Digit} - ['0']
	Num     = '0' | {Digi9}{Digit}*

	<E>   ::= <E> '+' <M> 
	       |  <E> '-' <M> 
	       |  <M> 
	
	<M>   ::= <M> '*' <N> 
	       |  <M> '/' <N> 
	       |  <N> 
	
	<N>   ::= '-' <V> 
	       |  <V> 
	
	<V>   ::= Num
	       |  '(' <E> ')'

Compile a .grm file with GOLD Parser and save table data to a .egt file.
We need only .egt file for a further parsing process. (from now .grm file is not necessary.)

Linking Library
---------------

Simply include::

	#include <cppauparser/all.h>

And link your module with this library, you might use proper command to linker.
Probablly you're already familiar with your linker option that make it.

Load a Grammar
--------------

After preparing a .egt grammar file, we can load a grammar file now.
Load a grammar as following::

	cppauparser::Grammar grammar;
	if (grammar.LoadFile(PATHSTR("data/operator.egt")) == false) {
	  printf("fail to open a grammar file\n");
	}

Pyauparser doesn't support old .cgt file format.
But if you have a .grm file, you can make a .egt file with GOLD Parser 5 or newer.

Parse
-----

With a grammar, you can parse a string or a file. There are two way to handle parsing results.
First one is an event-driven way as following::

	struct ParserEvent {
	  void operator()(cppauparser::ParseResultType::T ret,
	                  cppauparser::Parser& parser) const {
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
	  }
	};
	cppauparser::Parser parser(grammar);
	parser.LoadString("-2*(3+4)-5");
	parser.ParseAll(ParserEvent());

You can use lambda expression instead of ParseEvent struct.
Result is following::

	Shift   S=1, T=- '-'
	Shift   S=3, T=Num '2'
	Reduce  P=8, H=(S=8, P=<V> ::= Num), Hs=[(S=3, T=Num '2')]
	Reduce  P=6, H=(S=6, P=<N> ::= - <V>), Hs=[(S=1, T=- '-'), (S=8, P=<V> ::= Num)]
	Reduce  P=5, H=(S=5, P=<M> ::= <N>), Hs=[(S=6, P=<N> ::= - <V>)]
	...

It may look complicated but will be handled in a simple way.
Second one is creating a whole parse tree way as following::

	auto ret = cppauparser::ParseStringToTree(grammar, "-2*(3+4)-5");

Parser create a parse tree from string and return it.
You can traverse a tree in a way you want and evaluate it freely.
Tree can be dumped using Dump() method of tree::

	ret.result->Dump()

Result is following::

	<E> ::= <E> - <M>
	  <E> ::= <M>
	    <M> ::= <M> * <N>
	      <M> ::= <N>
	       <N> ::= - <V>
	         - '-'
	          <V> ::= Num
	            Num '2'
	      * '*'
	      <N> ::= <V>
	...

Link: https://github.com/veblush/CppAuParser/blob/master/sample/tutorial1.cpp

Evaluate with parsing events
----------------------------

Because LALR is a bottom-up parser, every parsing event occurs in a bottom up way.
And if there is a way to evaluate a parsed string from bottom-up, we can use an event-driven
eveluation process as following::

	cppauparser::ProductionHandler ph(grammar);
	PH_ON(ph, "<E> ::= <E> + <M>", return (void*)((int)c[0].data + (int)c[2].data););
	PH_ON(ph, "<E> ::= <E> - <M>", return (void*)((int)c[0].data - (int)c[2].data););
	PH_ON(ph, "<E> ::= <M>",       return c[0].data;);
	PH_ON(ph, "<M> ::= <M> * <N>", return (void*)((int)c[0].data * (int)c[2].data););
	PH_ON(ph, "<M> ::= <M> / <N>", return (void*)((int)c[0].data / (int)c[2].data););
	PH_ON(ph, "<M> ::= <N>",       return c[0].data;);
	PH_ON(ph, "<N> ::= - <V>",     return (void*)-(int)c[1].data; );
	PH_ON(ph, "<N> ::= <V>",       return c[0].data;);
	PH_ON(ph, "<V> ::= Num",       return (void*)atoi((char*)c[0].token.lexeme.c_str()););
	PH_ON(ph, "<V> ::= ( <E> )",   return c[1].data;);

	cppauparser::Parser parser(grammar);
	parser.LoadString("-2*(3+4)-5");
	parser.ParseAll(ph);
	printf("result=%d\n", (int)ph.GetResult());

Result is following::

	Result = -19

<TODO>

Link: https://github.com/veblush/CppAuParser/blob/master/sample/tutorial2.cpp

Evaluate with a syntax tree
---------------------------

Sometimes we need a whole parse tree. Because it is easy to traverse and manipulate.
If you need a value of sibling nodes or parents while evaluating a tree, this is what you're finding::

	struct Evaluator {
	  static int eval(const cppauparser::TreeNode* node) {
	    const cppauparser::TreeNodeNonTerminal* nt = static_cast<const cppauparser::TreeNodeNonTerminal*>(node);
	    const cppauparser::TreeNode* const * c = &nt->childs[0];
	    switch (node->production->index) {
	    case 0: // <E> ::= <E> + <M>
	      return eval(c[0]) + eval(c[2]);
	    case 1: // <E> ::= <E> - <M>
	      return eval(c[0]) - eval(c[2]);
	    case 2: // <E> ::= <M>
	      return eval(c[0]);
	    case 3: // <M> ::= <M> * <N>
	      return eval(c[0]) * eval(c[2]);
	    case 4: // <M> ::= <M> / <N>
	      return eval(c[0]) / eval(c[2]);
	    case 5: // <M> ::= <N>
	      return eval(c[0]);
	    case 6: // <N> ::= - <V>
	      return -eval(c[1]);
	    case 7: // <N> ::= <V>
	      return eval(c[0]);
	    case 8: // <V> ::= Num
	      return atoi((const char*)static_cast<const cppauparser::TreeNodeTerminal*>(c[0])->token.lexeme.c_str());
	    case 9: // <V> ::= ( <E> )
	      return eval(c[1]);
	      break;
	    default:
	      return 0;
	    }
	  }
	};

	int result = Evaluator::eval(ret.result);
	printf("Result = %d\n", result);

Result is following::

	Result = -19

Link: https://github.com/veblush/CppAuParser/blob/master/sample/tutorial3.cpp

Simplified Tree
---------------

<TODO>

Link: https://github.com/veblush/CppAuParser/blob/master/sample/tutorial4.cpp

Embedding a Grammar
-------------------

<TODO>

Link: https://github.com/veblush/CppAuParser/blob/master/sample/tutorial5.cpp

Changelog
=========

* 0.5

  * First release
