#include <cstring>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <utility>

#include "frontend/Lexer.h"

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
std::pair<const char *, TokenType> Lexer::in_word_set(const char *str,
                                                      size_t len) {
  static const char *wordlist[] = {
      "",       "T",       "is",       "",      "this",   "Class",   "import",
      "if",     "",        "true",     "while", "module", "extends", "",
      "else",   "false",   "method",   "List",  "end",    "Char",    "Array",
      "AnyRef", "Integer", "AnyValue", "then",  "class",  "String",  "Boolean",
      "var",    "loop",    "",         "",      "",       "",        "Real"};

  static const TokenType keytokenlist[] = {
      TOKEN_UNKNOWN,     TOKEN_UNKNOWN,      TOKEN_BBEGIN,
      TOKEN_UNKNOWN,     TOKEN_SELFREF,      TOKEN_CLASS,
      TOKEN_MODULE_IMP,  TOKEN_IF,           TOKEN_UNKNOWN,
      TOKEN_BOOL_TRUE,   TOKEN_WHILE,        TOKEN_MODULE_DECL,
      TOKEN_EXTENDS,     TOKEN_UNKNOWN,      TOKEN_ELSE,
      TOKEN_BOOL_FALSE,  TOKEN_METHOD,       TOKEN_TYPE_LIST,
      TOKEN_BEND,        TOKEN_TYPE_CHAR,    TOKEN_TYPE_ARRAY,
      TOKEN_TYPE_ANYREF, TOKEN_TYPE_INTEGER, TOKEN_TYPE_ANYVAL,
      TOKEN_THEN,        TOKEN_CLASS,        TOKEN_TYPE_STRING,
      TOKEN_TYPE_BOOL,   TOKEN_VAR_DECL,     TOKEN_LOOP,
      TOKEN_UNKNOWN,     TOKEN_UNKNOWN,      TOKEN_UNKNOWN,
      TOKEN_UNKNOWN,     TOKEN_TYPE_REAL};

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
  this->curr_line = 0;
  this->curr_column = 0;
#ifdef DEBUG
  LOG("In Lexer::Lexer() incoming buffer:\n%s\n",
      this->source_buffer->data.c_str());
#endif
}

bool Lexer::isSpecial(char c) {
  return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '.' ||
         c == '"' || c == '(' || c == ')' || c == ',' || c == '[' || c == ']' ||
         c == '=' || c == '>' || std::isspace(c);
}

/*
 * Return a pointer to the next token/lexem
 */
std::unique_ptr<Token> Lexer::next() {
  std::string token;
  while (true) {
    char c = peek();

#ifdef DEBUG
    LOG("In Lexer::next() incoming buffer: %c with state %u\n", c, curr_state);
#endif

    // EOF / end of buffer
    if (strlen(buffer) <= 0) {
      return std::make_unique<Token>(TOKEN_EOF, curr_line, curr_column);
    }

    // Skip comments
    // @TODO test, this is SUS
    if (c == '/') {
      while (c != '\n') {
        advance();
        c = peek();
      }

      ++this->curr_line;
      this->curr_column = 0;
    }

    switch (curr_state) {
    case STATE_START: {
      if (std::isalpha(c)) {
        curr_state = STATE_READ_WORD;
      } else if (std::isdigit(c) /*|| c == '-'*/) {
        curr_state = STATE_READ_NUM;
      } else if (std::isspace(c) || c == '\n' || c == '\r') {
        curr_state = STATE_START;
      } else if (c == ':') {
        curr_state = STATE_START;
        advance();
        if (peek() == '=') {
          advance();
          return std::make_unique<Token>(TOKEN_ASSIGNMENT, curr_line,
                                         curr_column);
        }

        return std::make_unique<Token>(TOKEN_VAR_DECL, curr_line, curr_column);
      } else if (c == '=') {
        curr_state = STATE_START;
        advance();
        if (peek() == '>') {
          advance();
          return std::make_unique<Token>(TOKEN_ARROW, curr_line, curr_column);
        }

        return std::make_unique<Token>(TOKEN_EQUAL, curr_line, curr_column);
      } else if (c == '>') {
        curr_state = STATE_START;
        advance();
        return std::make_unique<Token>(TOKEN_MORE, curr_line, curr_column);
      } else if (c == '"') {
        curr_state = STATE_READ_STRING;
      } else if (c == '(') {
        curr_state = STATE_START;
        advance();
        return std::make_unique<Token>(TOKEN_LBRACKET, curr_line, curr_column);
      } else if (c == ')') {
        curr_state = STATE_START;
        advance();
        return std::make_unique<Token>(TOKEN_RBRACKET, curr_line, curr_column);
      } else if (c == ',') {
        curr_state = STATE_START;
        advance();
        return std::make_unique<Token>(TOKEN_COMMA, curr_line, curr_column);
      } else if (c == '[') {
        curr_state = STATE_START;
        advance();
        return std::make_unique<Token>(TOKEN_LSBRACKET, curr_line, curr_column);
      } else if (c == ']') {
        curr_state = STATE_START;
        advance();
        return std::make_unique<Token>(TOKEN_RSBRACKET, curr_line, curr_column);
      } else if (c == '.') {
        curr_state = STATE_START;
        advance();
        return std::make_unique<Token>(TOKEN_DOT, curr_line, curr_column);
      } else
        curr_state = STATE_FAIL;
    } break;
    case STATE_READ_WORD: {
      if (isalpha(c)) {
        curr_state = STATE_READ_WORD;
      } else if (std::isdigit(c)) {
        curr_state = STATE_READ_IDENT;
      } else if (isSpecial(c)) {
        curr_state = STATE_START;
        auto [word, wtype] = in_word_set(token.c_str(), strlen(token.c_str()));
        if (word) {
          return std::make_unique<Token>(wtype, curr_line,
                                         curr_column - token.length() + 1);
        }
        return std::make_unique<Token>(TOKEN_IDENTIFIER, token, curr_line,
                                       curr_column - token.length() + 1);
        // @TODO add to symbol table
      } else
        curr_state = STATE_FAIL;
    } break;
    case STATE_READ_NUM: {
      if (!std::isdigit(c) && c != '.') {
        curr_state = STATE_START;
        return std::make_unique<Token>(TOKEN_INT_NUMBER, std::stoi(token),
                                       curr_line,
                                       curr_column - token.length() + 1);
      }
      if (c == '.') {
        curr_state = STATE_READ_REAL;
      } else {
        curr_state = STATE_READ_NUM;
      }
    } break;
    case STATE_READ_IDENT: {
      if (std::isalpha(c) || std::isdigit(c)) {
        curr_state = STATE_READ_IDENT;
      } else if (isSpecial(c)) {
        curr_state = STATE_START;
        return std::make_unique<Token>(TOKEN_IDENTIFIER, token, curr_line,
                                       curr_column);
        // @TODO add to symbol table ?
      } else
        curr_state = STATE_FAIL;
    } break;
    case STATE_READ_REAL: {
      if (std::isdigit(c)) {
        curr_state = STATE_READ_REAL;
      } else {
        curr_state = STATE_START;
        return std::make_unique<Token>(TOKEN_REAL_NUMBER, std::stod(token),
                                       curr_line,
                                       curr_column - token.length() + 1);
      }
      // else curr_state = STATE_FAIL;
    } break;
    case STATE_READ_STRING: {
      if (c == '"') {
        curr_state = STATE_START;
        token += c;
        advance();
        return std::make_unique<Token>(TOKEN_STRING, token, curr_line,
                                       curr_column - token.length() + 1);
      }
      curr_state = STATE_READ_STRING;
    } break;
    case STATE_READ_ARROW: {
      if (c == '>') {
        curr_state = STATE_START;
        return std::make_unique<Token>(TOKEN_ARROW, curr_line, curr_column);
      }
      curr_state = STATE_FAIL;
    } break;
    case STATE_FAIL:
    default: {
      return nullptr;
    } break;
    }

    if (!std::isspace(c) && c != '\n' && c != '\r')
      token += c;

    // If no token has been returned move to next char, i.e. eat input
    advance();

    // Move current position
    this->curr_column++;
    if (c == '\n') {
      ++this->curr_line;
      this->curr_column = 0;
    }
#ifdef DEBUG
    LOG("Current buffer token: %s | state: %d | column %lu\n", token.c_str(),
        curr_state, this->curr_column);
#endif
  }
}

const char *Lexer::getTokenTypeName(TokenType type) {
  static const char *tokenNames[] = {
      "TOKEN_EOF",          "TOKEN_IDENTIFIER",  "TOKEN_CLASS",
      "TOKEN_EXTENDS",      "TOKEN_VAR_DECL",    "TOKEN_SELFREF",
      "TOKEN_RETURN",       "TOKEN_MODULE_DECL", "TOKEN_MODULE_IMP",
      "TOKEN_IF",           "TOKEN_ELSE",        "TOKEN_THEN",
      "TOKEN_WHILE",        "TOKEN_LOOP",        "TOKEN_METHOD",
      "TOKEN_BBEGIN",       "TOKEN_BEND",        "TOKEN_INT_NUMBER",
      "TOKEN_REAL_NUMBER",  "TOKEN_COMMENT",     "TOKEN_STRING",
      "TOKEN_BOOL_TRUE",    "TOKEN_BOOL_FALSE",  "TOKEN_RBRACKET",
      "TOKEN_LBRACKET",     "TOKEN_RSBRACKET",   "TOKEN_LSBRACKET",
      "TOKEN_ASSIGNMENT",   "TOKEN_COLON",       "TOKEN_DOT",
      "TOKEN_COMMA",        "TOKEN_ARROW",       "TOKEN_EQUAL",
      "TOKEN_MORE",         "TOKEN_TYPE_STRING", "TOKEN_TYPE_CHAR",
      "TOKEN_TYPE_INTEGER", "TOKEN_TYPE_REAL",   "TOKEN_TYPE_BOOL",
      "TOKEN_TYPE_LIST",    "TOKEN_TYPE_ARRAY",  "TOKEN_TYPE_ANYVAL",
      "TOKEN_TYPE_ANYREF",  "TOKEN_UNKNOWN"};

  if (type >= TOKEN_EOF && type <= TOKEN_UNKNOWN) {
    return tokenNames[type];
  }
  return "TOKEN_UNKNOWN";
};
