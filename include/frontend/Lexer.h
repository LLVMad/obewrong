#ifndef OBW_LEXER_H
#define OBW_LEXER_H

#include "SourceLocation.h"

#include <fstream>
#include <memory>
#include <string>
#include <variant>
#ifdef DEBUG
  #include "util/Logger.h"
#endif

enum TokenType {
  TOKEN_EOF,
  TOKEN_IDENTIFIER,  // user-defined: letter { lettter | digit }
  TOKEN_CLASS,       // class
  TOKEN_EXTENDS,     // extends
  TOKEN_VAR_DECL,    // var
  TOKEN_SELFREF,     // this (do not confuse with constructor `this(...)`)
  TOKEN_RETURN,      // return
  TOKEN_MODULE_DECL, // module
  TOKEN_MODULE_IMP,  // import
  TOKEN_IF,          // if
  TOKEN_ELSE,        // else
  TOKEN_THEN,        // then
  TOKEN_WHILE,       // while
  TOKEN_LOOP,        // loop
  TOKEN_METHOD,      // method
  TOKEN_BBEGIN,      // is
  TOKEN_BEND,        // end
  TOKEN_INT_NUMBER,
  TOKEN_REAL_NUMBER,
  TOKEN_COMMENT,
  TOKEN_STRING,
  TOKEN_BOOL_TRUE,  // true
  TOKEN_BOOL_FALSE, // false
  TOKEN_RBRACKET,   // )
  TOKEN_LBRACKET,   // (
  TOKEN_RSBRACKET,  // [
  TOKEN_LSBRACKET,  // ]
  TOKEN_ASSIGNMENT, // :=
  TOKEN_COLON,      // :
  TOKEN_DOT,        // .
  TOKEN_COMMA,      // ,
  TOKEN_ARROW,      // =>,
  TOKEN_TYPE_STRING,
  TOKEN_TYPE_CHAR,
  TOKEN_TYPE_INTEGER,
  TOKEN_TYPE_REAL,
  TOKEN_TYPE_BOOL,
  TOKEN_TYPE_LIST,
  TOKEN_TYPE_ARRAY,
  TOKEN_TYPE_ANYVAL,
  TOKEN_TYPE_ANYREF,
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
 *    | \0-9\ -> continue (it is defienetly an identifier => no need to check if
 * its a keyword just wait for whitespace) | \0-9\ -> | whitespace -> return
 * Integer | \0-9\ -> continue (put new char in str) | . -> continue (it is a
 * Real Number then => we expect \0-9\ next | ':" -> | '=' -> Assignment OP |
 * whitespace -> it is var definition | '=' -> | '>" -> it is Forward routine |
 * '(' -> lbracket | ')' -> rbracket | '.' -> access to method
 *
 *  Then the mealy automata table should look like:
 */
enum StateType {
  STATE_NONE = -1,
  STATE_START,
  STATE_READ_WORD,
  STATE_READ_NUM,
  STATE_READ_DECL,
  STATE_READ_IDENT,
  STATE_READ_REAL,
  STATE_READ_ASSIGN,
  STATE_READ_STRING,
  STATE_READ_ARROW,
  STATE_FAIL = 9,
};

class Token {
public:
  /*
  union Value {
    int intValue;            // for Integer numbers
    double realValue;        // for Real numbers
    const char* stringValue; // for String literals
    const char* identName;   // for Identifiers
  }; */

  TokenType type;
  std::variant<std::monostate, int, double, std::string> value;
  size_t line;
  size_t column;

  // Mostly single-character and/or special symbols
  Token(TokenType type, size_t line, size_t column)
      : type(type), line(line), column(column) {};

  // Int value
  Token(TokenType type, int intValue, size_t line, size_t column)
      : type(type), value(intValue), line(line), column(column) {};

  // Real number
  Token(TokenType type, double realValue, size_t line, size_t column)
      : type(type), value(realValue), line(line), column(column) {};

  // Identifier or string literal
  Token(TokenType type, const std::string &lexem, size_t line, size_t column)
      : type(type), value(lexem), line(line), column(column) {};

};

class Lexer {
public:
  Lexer(std::shared_ptr<SourceBuffer> buffer);
  std::unique_ptr<Token> next();
  static const char* getTokenTypeName(TokenType type);
private:
  std::shared_ptr<SourceBuffer> source_buffer;
  StateType curr_state;
  size_t curr_line;   // @TODO sync with source buffer somehow
  size_t curr_column;
  const char* buffer;

  void advance() { buffer++; }
  void rewind() { buffer--; }
  char peek() { return buffer[0]; };

  inline static unsigned int hash(const char *str, size_t len);
  static std::pair<const char*, TokenType> in_word_set(const char *str, size_t len);
};

#endif
