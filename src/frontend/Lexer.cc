#include <fstream>
#include <iostream>
#include <sstream>

#include "Lexer.h"

#include <cstring>

#define TOTAL_KEYWORDS 27
#define MIN_WORD_LENGTH 1
#define MAX_WORD_LENGTH 8
#define MIN_HASH_VALUE 1
#define MAX_HASH_VALUE 34

// надо бы её в класс а то как то не хорошо
inline static unsigned int hash(const char *str, size_t len) {
  static unsigned char asso_values[] = {
      35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35,
      35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35,
      35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35,
      35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 10, 0,  0,  35, 35, 35, 35,
      35, 0,  35, 35, 13, 35, 35, 35, 35, 35, 15, 10, 0,  35, 35, 35, 35, 35,
      35, 35, 35, 35, 35, 35, 35, 35, 35, 20, 10, 5,  5,  10, 35, 0,  35, 35,
      15, 0,  20, 35, 10, 35, 15, 0,  0,  35, 10, 0,  35, 5,  35, 35, 35, 35,
      35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35,
      35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35,
      35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35,
      35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35,
      35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35,
      35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35,
      35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35,
      35, 35, 35, 35};


  return len + asso_values[(unsigned char)str[len - 1]] +
         asso_values[(unsigned char)str[0]];
}

// и это тоже
const char *in_word_set(const char *str, size_t len) {
  static const char *wordlist[] = {
      "",       "T",       "is",       "",      "this",   "Class",   "import",
      "if",     "",        "true",     "while", "module", "extends", "",
      "else",   "false",   "method",   "List",  "end",    "Char",    "Array",
      "AnyRef", "Integer", "AnyValue", "then",  "class",  "String",  "Boolean",
      "var",    "loop",    "",         "",      "",       "",        "Real"};

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH) {
    unsigned int key = hash(str, len);

    if (key <= MAX_HASH_VALUE) {
      const char *s = wordlist[key];

      if (*str == *s && !strcmp(str + 1, s + 1))
        return s;
    }
  }
  return nullptr;
}

Lexer::Lexer(SourceBuffer buffer) {
  this->current = nullptr;
  this->buffer = buffer.data.c_str();
}

Lexer::~Lexer() {}

std::unique_ptr<Token> Lexer::next() {

  while (true) {
    /*
     * swtich(STATE)
     * case start: ...    :
     * case choto tam...  : return token or change state
     */
  }

  // char ch = source_file.get();
  //
  // printf("%c ", ch);
  //
  return nullptr;
}
