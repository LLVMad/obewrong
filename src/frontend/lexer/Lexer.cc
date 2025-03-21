#include <cstring>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <utility>

#include "frontend/lexer/Lexer.h"

#define TOTAL_KEYWORDS 47
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 8
#define MIN_HASH_VALUE 2
#define MAX_HASH_VALUE 78

/*
 * Hash function for fast check if an input word is a keyword
 */
inline unsigned int Lexer::hash(const char *str, size_t len) {
  static unsigned char asso_values[] = {
      79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79,
      79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79,
      79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 10, 79, 5,  79,
      50, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 5,  0,  0,  79, 79, 79, 79,
      79, 0,  79, 79, 45, 79, 79, 79, 79, 79, 20, 15, 79, 79, 79, 79, 79, 79,
      79, 79, 79, 79, 79, 79, 79, 15, 15, 10, 35, 5,  15, 25, 15, 0,  79, 79,
      20, 0,  55, 79, 10, 79, 30, 0,  0,  25, 30, 0,  79, 10, 79, 79, 79, 79,
      79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79,
      79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79,
      79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79,
      79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79,
      79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79,
      79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79,
      79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79,
      79, 79, 79, 79};

  return len + asso_values[(unsigned char)str[len - 1]] +
         asso_values[(unsigned char)str[0]];
}

/*
 * Checks if a word is a keyword
 */
std::pair<const char *, TokenKind> Lexer::in_word_set(const char *str,
                                                      size_t len) {
  static const char *wordlist[] = {
      "",         "",        "is",      "int",    "this",   "Class",
      "import",   "",        "i64",     "true",   "while",  "module",
      "extends",  "i32",     "else",    "class",  "static", "if",
      "AnyValue", "case",    "Array",   "switch", "",       "f64",
      "list",     "false",   "AnyRef",  "",       "f32",    "func",
      "array",    "string",  "",        "u64",    "loop",   "",
      "printl",   "Integer", "u32",     "bool",   "",       "method",
      "default",  "end",     "Real",    "",       "String", "",
      "for",      "List",    "",        "",       "",       "i16",
      "real",     "",        "",        "",       "new",    "then",
      "",         "",        "Boolean", "var",    "",       "",
      "",         "",        "",        "",       "",       "",
      "",         "",        "",        "",       "",       "",
      "u16"};

  static const TokenKind keytokenlist[] = {
      TOKEN_UNKNOWN,    TOKEN_UNKNOWN,    TOKEN_BBEGIN,      TOKEN_TYPE_INT32,
      TOKEN_SELFREF,    TOKEN_CLASS,      TOKEN_MODULE_IMP,  TOKEN_UNKNOWN,
      TOKEN_TYPE_INT64, TOKEN_BOOL_TRUE,  TOKEN_WHILE,       TOKEN_MODULE_DECL,
      TOKEN_EXTENDS,    TOKEN_TYPE_INT32, TOKEN_ELSE,        TOKEN_CLASS,
      TOKEN_STATIC,     TOKEN_IF,         TOKEN_TYPE_ANYVAL, TOKEN_CASE,
      TOKEN_TYPE_ARRAY, TOKEN_SWITCH,     TOKEN_UNKNOWN,     TOKEN_TYPE_F64,
      TOKEN_TYPE_LIST,  TOKEN_BOOL_FALSE, TOKEN_TYPE_ANYREF, TOKEN_UNKNOWN,
      TOKEN_TYPE_REAL,  TOKEN_FUNC,       TOKEN_TYPE_ARRAY,  TOKEN_TYPE_STRING,
      TOKEN_UNKNOWN,    TOKEN_TYPE_U64,   TOKEN_LOOP,        TOKEN_UNKNOWN,
      TOKEN_PRINT,      TOKEN_TYPE_INT32, TOKEN_TYPE_U32,    TOKEN_TYPE_BOOL,
      TOKEN_UNKNOWN,    TOKEN_METHOD,     TOKEN_DEFAULT,     TOKEN_BEND,
      TOKEN_TYPE_REAL,  TOKEN_UNKNOWN,    TOKEN_TYPE_STRING, TOKEN_UNKNOWN,
      TOKEN_FOR,        TOKEN_TYPE_LIST,  TOKEN_UNKNOWN,     TOKEN_UNKNOWN,
      TOKEN_UNKNOWN,    TOKEN_TYPE_INT16, TOKEN_TYPE_REAL,   TOKEN_UNKNOWN,
      TOKEN_UNKNOWN,    TOKEN_UNKNOWN,    TOKEN_NEW,         TOKEN_THEN,
      TOKEN_UNKNOWN,    TOKEN_UNKNOWN,    TOKEN_TYPE_BOOL,   TOKEN_VAR_DECL,
      TOKEN_UNKNOWN,    TOKEN_UNKNOWN,    TOKEN_UNKNOWN,     TOKEN_UNKNOWN,
      TOKEN_UNKNOWN,    TOKEN_UNKNOWN,    TOKEN_UNKNOWN,     TOKEN_UNKNOWN,
      TOKEN_UNKNOWN,    TOKEN_UNKNOWN,    TOKEN_TYPE_U16};

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
         c == ':' ||std::isspace(c);
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
          return std::make_unique<Token>(TOKEN_ARROW, curr_line, curr_column);
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
          return std::make_unique<Token>(TOKEN_NOT_EQUAL, curr_line, curr_column);
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
      throw std::runtime_error("Lexer: Unrecognized token type");
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

std::vector<std::unique_ptr<Token>> Lexer::lex() {
  std::vector<std::unique_ptr<Token>> tokens;

  std::unique_ptr<Token> token = next();
  while (token->kind != TOKEN_EOF) {
#ifdef DEBUG
    LOG("next token is %s\n", getTokenTypeName(token->kind));
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
                                     "TOKEN_MINUS",
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
                                     "TOKEN_UNKNOWN"};

  if (type >= TOKEN_EOF && type <= TOKEN_UNKNOWN) {
    return tokenNames[type];
  }
  return "TOKEN_UNKNOWN";
}
