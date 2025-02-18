#include <fstream>
#include <iostream>
#include <sstream>
#include <cstring>
#include <regex>
#include <utility>

#include "Lexer.h"

#define TOTAL_KEYWORDS 27
#define MIN_WORD_LENGTH 1
#define MAX_WORD_LENGTH 8
#define MIN_HASH_VALUE 1
#define MAX_HASH_VALUE 34

/*
 * Hash function for fast check if an input word is a keyword
 */
inline unsigned int Lexer::hash(const char *str, size_t len) {
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

  return len + asso_values[static_cast<unsigned char>(str[len - 1])] +
         asso_values[static_cast<unsigned char>(str[0])];
}

/*
 * Checks if a word is a keyword
 */
const char* Lexer::in_word_set(const char *str, size_t len) {
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

Lexer::Lexer(std::shared_ptr<SourceBuffer> buffer) {
  this->source_buffer = std::move(buffer);
  this->buffer = source_buffer->data.c_str();
  curr_state = STATE_START;

  // std::cout << "Current buffer" << this->buffer << std::endl;
}

std::unique_ptr<Token> Lexer::next() {
  while (true) {
    char c = peek();

    std::cout << "Looking at char: " << c << std::endl;

    switch (curr_state) {
      case STATE_START: {
        if      (std::isalpha(c))   { curr_state = STATE_READ_WORD;   }
        else if (std::isdigit(c))   { curr_state = STATE_READ_NUM;    }
        else if (std::isspace(c) || c == '\n' || c == '\r')   { curr_state = STATE_START;       }
        else if (c == ':')          { curr_state = STATE_READ_DECL;   }
        else if (c == '"')          { curr_state = STATE_READ_STRING; }
        else if (c == '(') {
          curr_state = STATE_START;
          return std::make_unique<Token>(TOKEN_LBRACKET);
        }
        else if (c == ')') {
          curr_state = STATE_START;
          return std::make_unique<Token>(TOKEN_RBRACKET);
        }
        else if (c == ',') {
          curr_state = STATE_START;
          return std::make_unique<Token>(TOKEN_COMMA);
        }
        else if (c == '[') {
          curr_state = STATE_START;
          return std::make_unique<Token>(TOKEN_LSBRACKET);
        }
        else if (c == ']') {
          curr_state = STATE_START;
          return std::make_unique<Token>(TOKEN_RSBRACKET);
        }
        else curr_state = STATE_FAIL;
      } break;
      default: return nullptr;
    }

    // If no token has been returned move to next char, i.e. eat input
    advance(); // this->buffer++;
    std::cout << "New buffer" << this->buffer << std::endl;
  }
}
