// Copyright 2012 Esun Kim

#include <cppauparser/all.h>
#include <stdio.h>
#include <string.h>

#ifndef _WIN32
# define _tmain main
# define _tcscmp strcmp
# define _ttoi atoi
#endif

int c_show(int argc, PATHCHAR* argv[]) {
  if (argc < 2) {
    return 1;
  }

  // load grammar

  cppauparser::Grammar grammar;
  if (grammar.LoadFile(argv[1]) == false) {
    printf("fail to open a grammar file\n");
    return 1;
  }

  // print symbols or productions as a specified format

  if (_tcscmp(argv[0], PATHSTR("-s")) == 0) {
    printf("* symbols\n");
    for (auto i = grammar.symbols.begin(),
              i_end = grammar.symbols.end();
         i != i_end; ++i) {
      printf("\t%d\t\"%s\"\n", i->index, i->GetID().c_str());
    }
  } else if (_tcscmp(argv[0], PATHSTR("-p")) == 0) {
    printf("* productions\n");
    for (auto i = grammar.productions.begin(),
              i_end = grammar.productions.end();
         i != i_end; ++i) {
      printf("\t%d\t\"%s\"\n", i->index, i->GetID().c_str());
    }
  } else if (_tcscmp(argv[0], PATHSTR("-P")) == 0) {
    for (auto i = grammar.productions.begin(),
              i_end = grammar.productions.end();
         i != i_end; ++i) {
      printf("PH_ON(ph, \"%s\", return 0;);\n", i->GetID().c_str());
    }
  } else {
    return 1;
  }

  return 0;
}

int c_embed(int argc, PATHCHAR* argv[]) {
  if (argc < 1) {
    return 1;
  }

  // load options

  const PATHCHAR* grammar_path = PATHSTR("");
  int width = 80;
  
  for (int i = 0; i < argc; i++) {
    if (_tcscmp(argv[i], PATHSTR("-w")) == 0) {
      width = _ttoi(argv[i+1]);
      i += 1;
    } else {
      grammar_path = argv[i];
    }
  }

  // load grammar as binary

  FILE* fp = PATHOPEN(grammar_path, PATHSTR("rb"));
  if (fp == NULL) {
    printf("Cannot open grammar file.\n");
    return 1;
  }

  fseek(fp, 0, SEEK_END);
  size_t size = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  char* buf = new char[size];
  fread(buf, size, 1, fp);
  fclose(fp);

  // hexify binary to string

  printf("\"");
  bool prev_escaped = false;
  int cur_width = 0;
  char s[16];
  int s_len;
  for (size_t i = 0; i < size; i++) {
    unsigned char c = buf[i];
    if ((c >= 32 && c <= 126 && c != 34 && c != 92) &&
        (!prev_escaped || !((c >= 48 && c <= 57) ||
                            (c >= 65 && c <= 70) ||
                            (c >= 97 && c <= 102)))) {
      s_len = sprintf(s, "%c", c);
      prev_escaped = false;
    } else if (c == 0) {
      s_len = sprintf(s, "\\0");
      prev_escaped = true;
    } else {
      s_len = sprintf(s, "\\x%02x", c);
      prev_escaped = true;
    }

    if (cur_width + s_len > width - 2) {
      printf("\"\n\"");
      cur_width = 0;
    }
    printf("%s", s);
    cur_width += s_len;
  }
  printf("\"\n");

  delete [] buf;
  return 0;
}

void usage() {
  printf("auparser command ...\n");
  printf("  h[elp]     : show help\n");
  printf("\n");
  printf("  s[how]     : show information from a grammar file\n");
  printf("    [options] egt\n");
  printf("    -s show a symbol list\n");
  printf("    -p show a production rule list\n");
  printf("    -P show a production rule list as PH-on macros\n");
  printf("\n");
  printf("  e[mbed]   : create a string embedding a grammar file\n");
  printf("    [options] egt\n");
  printf("    -w width : specify max width of line. (default: 80)\n");
}

int _tmain(int argc, PATHCHAR* argv[]) {
  if (argc < 2) {
    usage();
    return 1;
  }

  if (_tcscmp(argv[1], PATHSTR("h")) == 0 ||
      _tcscmp(argv[1], PATHSTR("help")) == 0) {
    usage();
    return 0;
  } else if (_tcscmp(argv[1], PATHSTR("s")) == 0 ||
             _tcscmp(argv[1], PATHSTR("show")) == 0) {
      return c_show(argc-2, argv+2);
  } else if (_tcscmp(argv[1], PATHSTR("e")) == 0 ||
             _tcscmp(argv[1], PATHSTR("embed")) == 0) {
      return c_embed(argc-2, argv+2);
  } else {
    printf("Invalid command\n");
    return 1;
  }
}
