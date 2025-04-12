#include <cstring>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <utility>

#include "frontend/lexer/Lexer.h"

#define TOTAL_KEYWORDS 50
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 8
#define MIN_HASH_VALUE 2
#define MAX_HASH_VALUE 88
/* maximum key range = 87, duplicates = 0 */

/*
 * Hash function for fast check if an input word is a keyword
 */
inline unsigned int Lexer::hash(const char *str, size_t len) {
  static unsigned char asso_values[] = {
      89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89,
      89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89,
      89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 15, 89, 10, 89,
      60, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 45, 0,  89, 89, 89, 89, 89,
      89, 10, 89, 89, 60, 89, 89, 89, 89, 89, 20, 5,  89, 89, 89, 89, 89, 89,
      89, 89, 89, 89, 89, 89, 89, 0,  15, 10, 0,  5,  15, 25, 5,  0,  89, 89,
      35, 15, 30, 55, 10, 89, 5,  0,  0,  25, 40, 20, 89, 0,  89, 89, 89, 89,
      89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89,
      89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89,
      89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89,
      89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89,
      89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89,
      89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89,
      89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89,
      89, 89, 89, 89};
  return len + asso_values[(unsigned char)str[len - 1]] +
         asso_values[(unsigned char)str[0]];
}

/*
 * Checks if a word is a keyword
 */
std::pair<const char *, TokenKind> Lexer::in_word_set(const char *str,
                                                      size_t len) {
  static const char *wordlist[] = {
      "",        "",        "is",       "int",    "this",     "array",
      "import",  "default", "end",      "true",   "",         "switch",
      "extends", "i64",     "else",     "class",  "static",   "if",
      "i32",     "case",    "",         "method", "Integer",  "for",
      "enum",    "false",   "module",   "",       "f64",      "func",
      "while",   "string",  "",         "f32",    "then",     "",
      "String",  "Boolean", "u64",      "list",   "",         "return",
      "",        "u32",     "real",     "",       "",         "",
      "var",     "loop",    "Array",    "printl", "",         "new",
      "bool",    "",        "",         "",       "AnyValue", "Real",
      "",        "",        "",         "i16",    "List",     "",
      "AnyRef",  "",        "override", "",       "",         "",
      "",        "",        "",         "",       "",         "",
      "",        "",        "",         "",       "virtual",  "",
      "",        "",        "",         "",       "u16"};

  static const TokenKind keytokenlist[] = {
      TOKEN_UNKNOWN,     // 0: ""
      TOKEN_UNKNOWN,     // 1: ""
      TOKEN_BBEGIN,      // 2: "is"
      TOKEN_TYPE_INT32,  // 3: "int"
      TOKEN_SELFREF,     // 4: "this"
      TOKEN_TYPE_ARRAY,  // 5: "array"
      TOKEN_MODULE_IMP,  // 6: "import"
      TOKEN_DEFAULT,     // 7: "default"
      TOKEN_BEND,        // 8: "end"
      TOKEN_BOOL_TRUE,   // 9: "true"
      TOKEN_UNKNOWN,     // 10: ""
      TOKEN_SWITCH,      // 11: "switch"
      TOKEN_EXTENDS,     // 12: "extends"
      TOKEN_TYPE_INT64,  // 13: "i64"
      TOKEN_ELSE,        // 14: "else"
      TOKEN_CLASS,       // 15: "class"
      TOKEN_STATIC,      // 16: "static"
      TOKEN_IF,          // 17: "if"
      TOKEN_TYPE_INT32,  // 18: "i32"
      TOKEN_CASE,        // 19: "case"
      TOKEN_UNKNOWN,     // 20: ""
      TOKEN_METHOD,      // 21: "method"
      TOKEN_TYPE_INT32,  // 22: "Integer"
      TOKEN_FOR,         // 23: "for"
      TOKEN_ENUM,        // 24: "enum"
      TOKEN_BOOL_FALSE,  // 25: "false"
      TOKEN_MODULE_DECL, // 26: "module"
      TOKEN_UNKNOWN,     // 27: ""
      TOKEN_TYPE_F64,    // 28: "f64"
      TOKEN_FUNC,        // 29: "func"
      TOKEN_WHILE,       // 30: "while"
      TOKEN_TYPE_STRING, // 31: "string"
      TOKEN_UNKNOWN,     // 32: ""
      TOKEN_TYPE_REAL,   // 33: "f32"
      TOKEN_THEN,        // 34: "then"
      TOKEN_UNKNOWN,     // 35: ""
      TOKEN_TYPE_STRING, // 36: "String"
      TOKEN_TYPE_BOOL,   // 37: "Boolean"
      TOKEN_TYPE_U64,    // 38: "u64"
      TOKEN_TYPE_LIST,   // 39: "list"
      TOKEN_UNKNOWN,     // 40: ""
      TOKEN_RETURN,      // 41: "return"
      TOKEN_UNKNOWN,     // 42: ""
      TOKEN_TYPE_U32,    // 43: "u32"
      TOKEN_TYPE_REAL,   // 44: "real"
      TOKEN_UNKNOWN,     // 45: ""
      TOKEN_UNKNOWN,     // 46: ""
      TOKEN_UNKNOWN,     // 47: ""
      TOKEN_VAR_DECL,    // 48: "var"
      TOKEN_LOOP,        // 49: "loop"
      TOKEN_TYPE_ARRAY,  // 50: "Array"
      TOKEN_PRINT,       // 51: "printl"
      TOKEN_UNKNOWN,     // 52: ""
      TOKEN_NEW,         // 53: "new"
      TOKEN_TYPE_BOOL,   // 54: "bool"
      TOKEN_UNKNOWN,     // 55: ""
      TOKEN_UNKNOWN,     // 56: ""
      TOKEN_UNKNOWN,     // 57: ""
      TOKEN_TYPE_ANYVAL, // 58: "AnyValue"
      TOKEN_TYPE_REAL,   // 59: "Real"
      TOKEN_UNKNOWN,     // 60: ""
      TOKEN_UNKNOWN,     // 61: ""
      TOKEN_UNKNOWN,     // 62: ""
      TOKEN_TYPE_INT16,  // 63: "i16"
      TOKEN_TYPE_LIST,   // 64: "List"
      TOKEN_UNKNOWN,     // 65: ""
      TOKEN_TYPE_ANYREF, // 66: "AnyRef"
      TOKEN_UNKNOWN,     // 67: ""
      TOKEN_OVERRIDE,    // 68: "override"
      TOKEN_UNKNOWN,     // 69: ""
      TOKEN_UNKNOWN,     // 70: ""
      TOKEN_UNKNOWN,     // 71: ""
      TOKEN_UNKNOWN,     // 72: ""
      TOKEN_UNKNOWN,     // 73: ""
      TOKEN_UNKNOWN,     // 74: ""
      TOKEN_UNKNOWN,     // 75: ""
      TOKEN_UNKNOWN,     // 76: ""
      TOKEN_UNKNOWN,     // 77: ""
      TOKEN_UNKNOWN,     // 78: ""
      TOKEN_UNKNOWN,     // 79: ""
      TOKEN_VIRTUAL,     // 80: "virtual"
      TOKEN_UNKNOWN,     // 81: ""
      TOKEN_UNKNOWN,     // 82: ""
      TOKEN_UNKNOWN,     // 83: ""
      TOKEN_UNKNOWN,     // 84: ""
      TOKEN_UNKNOWN,     // 85: ""
      TOKEN_TYPE_U16     // 86: "u16"
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
         c == '=' || c == '>' || c == '<' || c == '!' || c == '&' || c == '~' ||
         c == '/' || c == '%' || c == '^' || c == '{' || c == '|' || c == '}' ||
         c == ':' || std::isspace(c);
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

      advance();
      c = peek();
    }

    switch (curr_state) {
    case STATE_START: {
      if (std::isalpha(c)) {
        curr_state = STATE_READ_WORD;
      } else if (std::isdigit(c) /*|| c == '-'*/) {
        curr_state = STATE_READ_NUM;
      } else if (std::isspace(c)) {
        curr_state = STATE_START;
      } else if (c == '\n' || c == '\r') {
        curr_state = STATE_START;
      } else if (c == ':') {
        curr_state = STATE_START;
        advance();
        if (peek() == '=') {
          advance();
          return std::make_unique<Token>(TOKEN_ASSIGNMENT, curr_line,
                                         curr_column);
        }

        if (peek() == ':') {
          advance();
          return std::make_unique<Token>(TOKEN_DOUBLE_COLON, curr_line,
                                         curr_column);
        }

        return std::make_unique<Token>(TOKEN_COLON, curr_line, curr_column);
      } else if (c == '=') {
        curr_state = STATE_START;
        advance();
        if (peek() == '>') {
          advance();
          return std::make_unique<Token>(TOKEN_ARROW, curr_line, curr_column);
        }
        if (peek() == '=') {
          advance();
          return std::make_unique<Token>(TOKEN_EQUAL, curr_line, curr_column);
        }

        return std::make_unique<Token>(TOKEN_WRONG_ASSIGN, curr_line,
                                       curr_column);
      } else if (c == '>') {
        curr_state = STATE_START;
        advance();

        if (peek() == '>') {
          advance();
          return std::make_unique<Token>(TOKEN_BIT_SHIFT_RIGHT, curr_line,
                                         curr_column);
        }
        if (peek() == '=') {
          advance();
          return std::make_unique<Token>(TOKEN_MORE_EQUAL, curr_line,
                                         curr_column);
        }

        return std::make_unique<Token>(TOKEN_MORE, curr_line, curr_column);
      } else if (c == '<') {
        curr_state = STATE_START;
        advance();

        if (peek() == '<') {
          advance();
          return std::make_unique<Token>(TOKEN_BIT_SHIFT_LEFT, curr_line,
                                         curr_column);
        }
        if (peek() == '=') {
          advance();
          return std::make_unique<Token>(TOKEN_LESS_EQUAL, curr_line,
                                         curr_column);
        }

        return std::make_unique<Token>(TOKEN_LESS, curr_line, curr_column);
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
      } else if (c == '&') {
        curr_state = STATE_START;
        advance();
        if (peek() == '&') {
          advance();
          return std::make_unique<Token>(TOKEN_LOGIC_AND, curr_line,
                                         curr_column);
        }

        return std::make_unique<Token>(TOKEN_BIT_AND, curr_line, curr_column);
      } else if (c == '|') {
        curr_state = STATE_START;
        advance();

        if (peek() == '|') {
          advance();
          return std::make_unique<Token>(TOKEN_LOGIC_OR, curr_line,
                                         curr_column);
        }

        return std::make_unique<Token>(TOKEN_BIT_OR, curr_line, curr_column);
      } else if (c == '^') {
        curr_state = STATE_START;
        advance();
        return std::make_unique<Token>(TOKEN_BIT_XOR, curr_line, curr_column);
      } else if (c == '!') {
        curr_state = STATE_START;
        advance();

        if (peek() == '=') {
          advance();
          return std::make_unique<Token>(TOKEN_NOT_EQUAL, curr_line,
                                         curr_column);
        }

        return std::make_unique<Token>(TOKEN_LOGIC_NOT, curr_line, curr_column);
      } else if (c == '~') {
        curr_state = STATE_START;
        advance();
        return std::make_unique<Token>(TOKEN_BIT_INV, curr_line, curr_column);
      } else if (c == '+') {
        curr_state = STATE_START;
        advance();
        return std::make_unique<Token>(TOKEN_PLUS, curr_line, curr_column);
      } else if (c == '-') {
        curr_state = STATE_START;
        advance();
        return std::make_unique<Token>(TOKEN_MINUS, curr_line, curr_column);
      } else if (c == '*') {
        curr_state = STATE_START;
        advance();
        return std::make_unique<Token>(TOKEN_STAR, curr_line, curr_column);
      } else if (c == '/') {
        curr_state = STATE_START;
        advance();
        return std::make_unique<Token>(TOKEN_SLASH, curr_line, curr_column);
      } else if (c == '%') {
        curr_state = STATE_START;
        advance();
        return std::make_unique<Token>(TOKEN_PERCENT, curr_line, curr_column);
      } else
        curr_state = STATE_FAIL;
    } break;
    case STATE_READ_WORD: {
      if (isalpha(c) || c == '_' || std::isdigit(c)) {
        curr_state = STATE_READ_WORD;
        // } else if (std::isdigit(c)) {
        //   curr_state = STATE_READ_IDENT;
      } else if (isSpecial(c)) {
        curr_state = STATE_START;
        auto [word, wtype] = in_word_set(token.c_str(), strlen(token.c_str()));
        if (word) {
          return std::make_unique<Token>(
              wtype, word, curr_line, curr_column /* - token.length() + 1 */);
        }
        return std::make_unique<Token>(TOKEN_IDENTIFIER, token, curr_line,
                                       curr_column /* - token.length() + 1 */);
        // @TODO add to symbol table
      } else
        curr_state = STATE_FAIL;
    } break;
    case STATE_READ_NUM: {
      if (!std::isdigit(c) && c != '.') {
        curr_state = STATE_START;
        return std::make_unique<Token>(TOKEN_INT_NUMBER, std::stoi(token),
                                       curr_line,
                                       curr_column /* - token.length() + 1 */);
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
                                       curr_column /* - token.length() + 1 */);
      }
      // else curr_state = STATE_FAIL;
    } break;
    case STATE_READ_STRING: {
      if (c == '"') {
        curr_state = STATE_START;
        token += c;
        advance();
        return std::make_unique<Token>(TOKEN_STRING, token, curr_line,
                                       curr_column /* - token.length() + 1 */);
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
      throw std::runtime_error("Lexer: Unrecognized token type");
      return nullptr;
    } break;
    }

    if (!std::isspace(c) && c != '\n' && c != '\r')
      token += c;

    // If no token has been returned move to next char, i.e. eat input
    advance();
#ifdef DEBUG
    LOG("Current buffer token: %s | state: %d | %lu:%lu\n", token.c_str(),
        curr_state, this->curr_line, this->curr_column);
#endif
  }
}

std::vector<std::unique_ptr<Token>> Lexer::lex() {
  std::vector<std::unique_ptr<Token>> tokens;

  std::unique_ptr<Token> token = next();
  while (token->kind != TOKEN_EOF) {
#ifdef DEBUG
    LOG("next token is %s | %lu:%lu\n", getTokenTypeName(token->kind), curr_line, curr_column);
#endif
    tokens.push_back(std::move(token));
    token = next();
  }

  tokens.push_back(std::move(token));

  return tokens;
}

const char *Lexer::getTokenTypeName(TokenKind type) {
  static const char *tokenNames[] = {"TOKEN_EOF",
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
                                     "TOKEN_SWITCH",
                                     "TOKEN_CASE",
                                     "TOKEN_DEFAULT",
                                     "TOKEN_WHILE",
                                     "TOKEN_LOOP",
                                     "TOKEN_METHOD",
                                     "TOKEN_FUNC",
                                     "TOKEN_FOR",
                                     "TOKEN_STATIC",
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
                                     "TOKEN_DOUBLE_COLON",
                                     "TOKEN_DOT",
                                     "TOKEN_COMMA",
                                     "TOKEN_ARROW",
                                     "TOKEN_EQUAL",
                                     "TOKEN_NOT_EQUAL",
                                     "TOKEN_WRONG_ASSIGN",
                                     "TOKEN_MORE",
                                     "TOKEN_LESS",
                                     "TOKEN_MORE_EQUAL",
                                     "TOKEN_LESS_EQUAL",
                                     "TOKEN_BIT_AND",
                                     "TOKEN_BIT_OR",
                                     "TOKEN_BIT_XOR",
                                     "TOKEN_BIT_INV",
                                     "TOKEN_LOGIC_NOT",
                                     "TOKEN_LOGIC_AND",
                                     "TOKEN_LOGIC_OR",
                                     "TOKEN_BIT_SHIFT_LEFT",
                                     "TOKEN_BIT_SHIFT_RIGHT",
                                     "TOKEN_PLUS",
                                     "TOKEN_INCREMENT"
                                     "TOKEN_MINUS",
                                     "TOKEN_DECREMENT",
                                     "TOKEN_STAR",
                                     "TOKEN_SLASH",
                                     "TOKEN_PERCENT",
                                     "TOKEN_PRINT",
                                     "TOKEN_TYPE_STRING",
                                     "TOKEN_TYPE_INT32",
                                     "TOKEN_TYPE_INT64",
                                     "TOKEN_TYPE_INT16",
                                     "TOKEN_TYPE_U32",
                                     "TOKEN_TYPE_U16",
                                     "TOKEN_TYPE_U64",
                                     "TOKEN_TYPE_REAL",
                                     "TOKEN_TYPE_F64",
                                     "TOKEN_TYPE_BOOL",
                                     "TOKEN_TYPE_LIST",
                                     "TOKEN_TYPE_ARRAY",
                                     "TOKEN_TYPE_ANYVAL",
                                     "TOKEN_TYPE_ANYREF",
                                     "TOKEN_TYPE_TYPE",
                                     "TOKEN_NEW",
                                     "TOKEN_OVERRIDE",
                                     "TOKEN_VIRTUAL",
                                     "TOKEN_ENUM",
                                     "TOKEN_UNKNOWN"};

  return tokenNames[type];
}
