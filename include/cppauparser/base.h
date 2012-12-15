// Copyright 2012 Esun Kim

#ifndef _CPPAUPARSER_BASE_H_
#define _CPPAUPARSER_BASE_H_

#ifdef _CPPAUPARSER_DLL_BUILD
# define CppAuParserDecl __declspec(dllexport)
#else
# ifdef _CPPAUPARSER_DLL_USE
#   define CppAuParserDecl __declspec(dllimport)
# else
#   define CppAuParserDecl
# endif
#endif

#ifdef _MSC_VER
# pragma warning(disable: 4251)
#endif

#ifdef _WIN32
# define PATHCHAR TCHAR
# define PATHSTR _T
# define PATHOPEN _tfopen
# include "tchar.h"
#else
# define PATHCHAR char
# define PATHSTR
# define PATHOPEN fopen
#endif

#define CPPAUPARSER_UNCOPYABLE(T) \
  private: \
    T(const T&); \
    const T& operator=(const T&);

#endif  // _CPPAUPARSER_BASE_H_
