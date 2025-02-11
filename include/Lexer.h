#ifndef OBW_LEXER_H
#define OBW_LEXER_H

#include "SourceLocation.h"

#include <fstream>
#include <memory>
#include <string>

enum TokenType {
  TOKEN_EOF,
  TOKEN_IDENTIFIER,   // user-defined: letter { lettter | digit }
  TOKEN_CLASS,        // class
  TOKEN_EXTENDS,      // extends
  TOKEN_VAR_DECL,     // var
  TOKEN_SELFREF,      // this (do not confuse with constructor `this(...)`)
  TOKEN_RETURN,       // return
  TOKEN_MODULE_DECL,  // module
  TOKEN_MODULE_IMP,   // import
  TOKEN_IF,           // if
  TOKEN_ELSE,         // else
  TOKEN_THEN,         // then
  TOKEN_WHILE,        // while
  TOKEN_LOOP,         // loop
  TOKEN_METHOD,       // method
  TOKEN_BBEGIN,       // is
  TOKEN_BEND,         // end
  TOKEN_INT_NUMBER,
  TOKEN_REAL_NUMBER,
  TOKEN_COMMENT,
  TOKEN_STRING,
  TOKEN_BOOL_TRUE,    // true
  TOKEN_BOOL_FALSE,   // false
  TOKEN_RBRACKET,     // )
  TOKEN_LBRACKET,     // (
  TOKEN_ASSIGNMENT,   // :=
  TOKEN_COLON,        // :
  TOKEN_DOT,          // .
  TOKEN_ARROW,        // =>
  TOKEN_UNKNOWN
};

class Token {
public:
  TokenType type;
  std::string lexeme;
  Token(TokenType type, const std::string &lexeme);
};

class Lexer {
public:
  Lexer(SourceBuffer buffer);
  ~Lexer();
  std::unique_ptr<Token> next();
private:
  std::unique_ptr<Token> current;
  const char* buffer;

  void advance();
};

#endif
