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

/*
 * Lets define a FiniteStateMachine,
 * first look at the process, what the reaction
 * of automata to a different inputs (chars)
 *
 * start ->
 *  | \A-z\ ->
 *    | whitespace ->
 *      | is keyword ? -> return keyword ( + type )
 *      | else -> return identifier
 *    | \A-z\ -> continue (put new char in str)
 *    | \0-9\ -> continue (it is defienetly an identifier => no need to check if its a keyword just wait for whitespace)
 *  | \0-9\ ->
 *    | whitespace -> return Integer
 *    | \0-9\ -> continue (put new char in str)
 *    | . -> continue (it is a Real Number then => we expect \0-9\ next
 *  | ':" ->
 *    | '=' -> Assignment OP
 *    | whitespace -> it is var definition
 *  | '=' ->
 *    | '>" -> it is Forward routine
 *  | '(' -> lbracket
 *  | ')' -> rbracket
 *  | '.' -> access to method
 *
 *  Then the mealy automata table should look like:
 */
enum StateType {
  STATE_NONE,
  STATE_START,
  STATE_READING_NUM,
};

class Token {
public:
  TokenType type;
  std::string lexeme;
  Token(TokenType type, const std::string &lexeme);

  int intValue;             // for Integer numbers
  double realValue;         // for Real numbers
  uint64_t id;              // for identifiers
};

class Lexer {
public:
  Lexer(SourceBuffer buffer);
  ~Lexer();
  std::unique_ptr<Token> next();
private:
  std::unique_ptr<Token> current;
  const char* buffer;

  void advance() { buffer++; };
  void rewind() { buffer--; }
};

#endif
