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
std::pair<const char*, TokenType> Lexer::in_word_set(const char *str, size_t len) {
  static const char *wordlist[] = {
      "",       "T",       "is",       "",      "this",   "Class",   "import",
      "if",     "",        "true",     "while", "module", "extends", "",
      "else",   "false",   "method",   "List",  "end",    "Char",    "Array",
      "AnyRef", "Integer", "AnyValue", "then",  "class",  "String",  "Boolean",
      "var",    "loop",    "",         "",      "",       "",        "Real"
  };

  static const TokenType keytokenlist[] = {
    TOKEN_UNKNOWN, TOKEN_UNKNOWN, TOKEN_BBEGIN, TOKEN_UNKNOWN, TOKEN_SELFREF, TOKEN_CLASS, TOKEN_MODULE_IMP,
    TOKEN_IF, TOKEN_UNKNOWN, TOKEN_BOOL_TRUE, TOKEN_WHILE, TOKEN_MODULE_DECL, TOKEN_EXTENDS, TOKEN_UNKNOWN,
    TOKEN_ELSE, TOKEN_BOOL_FALSE, TOKEN_METHOD, TOKEN_TYPE_LIST, TOKEN_BEND, TOKEN_TYPE_CHAR, TOKEN_TYPE_ARRAY,
    TOKEN_TYPE_ANYREF, TOKEN_TYPE_INTEGER, TOKEN_TYPE_ANYVAL, TOKEN_THEN, TOKEN_CLASS, TOKEN_TYPE_STRING, TOKEN_TYPE_BOOL,
    TOKEN_VAR_DECL, TOKEN_LOOP, TOKEN_UNKNOWN, TOKEN_UNKNOWN, TOKEN_UNKNOWN, TOKEN_UNKNOWN, TOKEN_TYPE_REAL
  };

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH) {
    unsigned int key = hash(str, len);
    if (key <= MAX_HASH_VALUE) {
      const char *s = wordlist[key];
      if (*str == *s && !strcmp(str + 1, s + 1)) {
        return std::pair{s, keytokenlist[key]};
      }
    }
  }
  return std::pair{nullptr, TOKEN_UNKNOWN};
}

Lexer::Lexer(std::shared_ptr<SourceBuffer> buffer) {
  this->source_buffer = std::move(buffer);
  this->buffer = source_buffer->data.c_str();
  this->curr_state = STATE_START;
  this->id = 0;

#ifdef DEBUG
  LOG("In Lexer::Lexer() incoming buffer:\n%s\n", this->source_buffer->data.c_str());
#endif
}

/*
 * Return a pointer to the next token/lexem
 *
 * @TODO add fail(StateType from_state) func
 */
std::unique_ptr<Token> Lexer::next() {
  std::string token;
  while (true) {
    char c = peek();

    // EOF / end of buffer
    if (strlen(buffer) <= 0) {
      return std::make_unique<Token>(TOKEN_EOF);
    }

    switch (curr_state) {
      case STATE_START: {
        if      (std::isalpha(c))   { curr_state = STATE_READ_WORD;   }
        else if (std::isdigit(c))   { curr_state = STATE_READ_NUM;    }
        else if (std::isspace(c) || c == '\n' || c == '\r') {
          curr_state = STATE_START;
        }
        else if (c == ':')          { curr_state = STATE_READ_DECL;   }
        else if (c == '"')          { curr_state = STATE_READ_STRING; }
        else if (c == '(') {
          curr_state = STATE_START;
          advance();
          return std::make_unique<Token>(TOKEN_LBRACKET);
        }
        else if (c == ')') {
          curr_state = STATE_START;
          advance();
          return std::make_unique<Token>(TOKEN_RBRACKET);
        }
        else if (c == ',') {
          curr_state = STATE_START;
          advance();
          return std::make_unique<Token>(TOKEN_COMMA);
        }
        else if (c == '[') {
          curr_state = STATE_START;
          advance();
          return std::make_unique<Token>(TOKEN_LSBRACKET);
        }
        else if (c == ']') {
          curr_state = STATE_START;
          advance();
          return std::make_unique<Token>(TOKEN_RSBRACKET);
        }
        else curr_state = STATE_FAIL;
      } break;
      case STATE_READ_WORD: {
        if (isalpha(c))             { curr_state = STATE_READ_WORD;   }
        else if (std::isdigit(c))   { curr_state = STATE_READ_IDENT;  }
        else if (std::isspace(c)) {
          curr_state = STATE_START;
          auto [word, wtype] = in_word_set(token.c_str(), strlen(token.c_str()));
          if (word) {
            return std::make_unique<Token>(wtype);
          }
          return std::make_unique<Token>(TOKEN_IDENTIFIER, id++, token);
          // @TODO add to symbol table ?
        }
        else if (c == ':') {
          auto [word, wtype] = in_word_set(token.c_str(), strlen(token.c_str()));
          if (word) {
            // declaration cant follow a keyword, i.e.: "var while : Integer"
            curr_state = STATE_FAIL;
          }

          curr_state = STATE_READ_DECL;
          return std::make_unique<Token>(TOKEN_IDENTIFIER, id++, token);
          // @TODO add to symbol table ?
        }
        else if (c == '(' || c == ')' || c == ',' || c == '.' || c == '[' || c == ']') {
          curr_state = STATE_START;

          auto [word, wtype] = in_word_set(token.c_str(), strlen(token.c_str()));
          if (word) {
            return std::make_unique<Token>(wtype);
          }
          return std::make_unique<Token>(TOKEN_IDENTIFIER, id++, token);
          // @TODO add to symbol table ?
        }
        else if (c == '"')          { curr_state = STATE_READ_STRING; }
        else curr_state = STATE_FAIL;
      } break;
      case STATE_READ_NUM: {
        if (std::isspace(c) || c == ',') {
          curr_state = STATE_START;
          return std::make_unique<Token>(TOKEN_INT_NUMBER, std::stoi(token));
        }
        else if (std::isdigit(c))   { curr_state = STATE_READ_NUM;    }
        else if (c == '.')          { curr_state = STATE_READ_REAL;   }
        else curr_state = STATE_FAIL;
      } break;
      case STATE_READ_DECL: {
        if (std::isalpha(c)) {
          curr_state = STATE_READ_WORD;
          return std::make_unique<Token>(TOKEN_VAR_DECL);
        }
        else if (std::isdigit(c)) {
          curr_state = STATE_READ_NUM;
          return std::make_unique<Token>(TOKEN_VAR_DECL);
        }
        else if (std::isspace(c)) {
          curr_state = STATE_START;
          return std::make_unique<Token>(TOKEN_VAR_DECL);
        }
        else if (c == '"') {
          curr_state = STATE_READ_STRING;
          return std::make_unique<Token>(TOKEN_VAR_DECL);
        }
        else if (c == '=')          { curr_state = STATE_READ_ASSIGN;}
        else curr_state = STATE_FAIL;
      } break;
      case STATE_READ_IDENT: {
        if (std::isalpha(c) || std::isdigit(c)) {
          curr_state = STATE_READ_IDENT;
        }
        else if (std::isspace(c)) {
          curr_state = STATE_START;
          return std::make_unique<Token>(TOKEN_IDENTIFIER, id++, token);
          // @TODO add to symbol table ?
        }
        else if (c == ':') {
          curr_state = STATE_READ_DECL;
          return std::make_unique<Token>(TOKEN_IDENTIFIER, id++, token);
        }
        else if (c == '(' || c == ')' || c == ',' || c == '.' || c == '[') {
          curr_state = STATE_START;
          return std::make_unique<Token>(TOKEN_IDENTIFIER, id++, token);
        }
        else curr_state = STATE_FAIL;
      } break;
      case STATE_READ_REAL: {
        if (std::isdigit(c))        { curr_state = STATE_READ_REAL;  }
        else if (std::isspace(c)) {
          curr_state = STATE_START;
          return std::make_unique<Token>(TOKEN_REAL_NUMBER, std::stod(token));
        }
        else if (c == ',') {
          curr_state = STATE_START;
          return std::make_unique<Token>(TOKEN_REAL_NUMBER, std::stod(token));
        }
        else curr_state = STATE_FAIL;
      } break;
      case STATE_READ_ASSIGN: {
        if (std::isalpha(c)) {
          curr_state = STATE_READ_WORD;
          return std::make_unique<Token>(TOKEN_ASSIGNMENT);
        }
        else if (std::isdigit(c)) {
          curr_state = STATE_READ_NUM;
          return std::make_unique<Token>(TOKEN_ASSIGNMENT);
        }
        else if (std::isspace(c)) {
          curr_state = STATE_START;
          return std::make_unique<Token>(TOKEN_ASSIGNMENT);
        }
        else if (c == '"') {
          curr_state = STATE_READ_STRING;
          return std::make_unique<Token>(TOKEN_ASSIGNMENT);
        }
        else curr_state = STATE_FAIL;
      } break;
      case STATE_READ_STRING: {
        if (c == '"') {
          curr_state = STATE_START;
          token += c;
          advance();
          return std::make_unique<Token>(TOKEN_STRING, token);
        }
        curr_state = STATE_READ_STRING;
      } break;
      case STATE_READ_ARROW: {
        if (c == '>') {
          curr_state = STATE_START;
          return std::make_unique<Token>(TOKEN_ARROW);
        }
        curr_state = STATE_FAIL;
      } break;
      default: return nullptr;
    }

    if (!std::isspace(c) && c != '\n' && c != '\r')
      token += c;

    // If no token has been returned move to next char, i.e. eat input
    advance();
#ifdef DEBUG
    LOG("Current buffer token %s\n", token.c_str());
#endif
  }
}

const char* Lexer::getTokenTypeName(TokenType type) {
  static const char* tokenNames[] = {
    "TOKEN_EOF",
    "TOKEN_IDENTIFIER",
    "TOKEN_CLASS",
    "TOKEN_EXTENDS",
    "TOKEN_VAR_DECL",
    "TOKEN_SELFREF",
    "TOKEN_RETURN",
    "TOKEN_MODULE_DECL",
    "TOKEN_MODULE_IMP",
    "TOKEN_IF",
    "TOKEN_ELSE",
    "TOKEN_THEN",
    "TOKEN_WHILE",
    "TOKEN_LOOP",
    "TOKEN_METHOD",
    "TOKEN_BBEGIN",
    "TOKEN_BEND",
    "TOKEN_INT_NUMBER",
    "TOKEN_REAL_NUMBER",
    "TOKEN_COMMENT",
    "TOKEN_STRING",
    "TOKEN_BOOL_TRUE",
    "TOKEN_BOOL_FALSE",
    "TOKEN_RBRACKET",
    "TOKEN_LBRACKET",
    "TOKEN_RSBRACKET",
    "TOKEN_LSBRACKET",
    "TOKEN_ASSIGNMENT",
    "TOKEN_COLON",
    "TOKEN_DOT",
    "TOKEN_COMMA",
    "TOKEN_ARROW",
    "TOKEN_TYPE_STRING",
    "TOKEN_TYPE_CHAR",
    "TOKEN_TYPE_INTEGER",
    "TOKEN_TYPE_REAL",
    "TOKEN_TYPE_BOOL",
    "TOKEN_TYPE_LIST",
    "TOKEN_TYPE_ARRAY",
    "TOKEN_TYPE_ANYVAL",
    "TOKEN_TYPE_ANYREF",
    "TOKEN_UNKNOWN"
  };

  if (type >= TOKEN_EOF && type <= TOKEN_UNKNOWN) {
    return tokenNames[type];
  }
  return "TOKEN_UNKNOWN";
};

