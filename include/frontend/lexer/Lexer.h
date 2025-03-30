#ifndef OBW_LEXER_H
#define OBW_LEXER_H

#include "frontend/SourceLocation.h"

#include <fstream>
#include <memory>
#include <string>
#include <variant>
#ifdef DEBUG
#include "util/Logger.h"
#endif

// '*' means we added this syntax ourselves,
// and it wasn't mentioned in the reference manual
// maybe not all of this will be implemented, but
// its useful to add it before i think
enum TokenKind {
  TOKEN_EOF,
  TOKEN_IDENTIFIER,  // user-defined: letter { lettter | digit }
  TOKEN_CLASS,       // class
  TOKEN_EXTENDS,     // extends
  TOKEN_VAR_DECL,    // var
  TOKEN_SELFREF,     // this (do not confuse with constructor `this(...)`)
  TOKEN_RETURN,      // return
  TOKEN_MODULE_DECL, // * module
  TOKEN_MODULE_IMP,  // * import
  TOKEN_IF,          // if
  TOKEN_ELSE,        // else
  TOKEN_THEN,        // then
  TOKEN_SWITCH,      // * switch
  TOKEN_CASE,        // * case
  TOKEN_DEFAULT,     // * default
  TOKEN_WHILE,       // while
  TOKEN_LOOP,        // loop
  TOKEN_METHOD,      // method
  TOKEN_FUNC,        // * func
  TOKEN_FOR,         // * for
  TOKEN_STATIC,      // * static
  TOKEN_BBEGIN,      // is
  TOKEN_BEND,        // end
  TOKEN_INT_NUMBER,
  TOKEN_REAL_NUMBER,
  TOKEN_COMMENT,
  TOKEN_STRING,
  TOKEN_BOOL_TRUE,       // true
  TOKEN_BOOL_FALSE,      // false
  TOKEN_RBRACKET,        // )
  TOKEN_LBRACKET,        // (
  TOKEN_RSBRACKET,       // [
  TOKEN_LSBRACKET,       // ]
  TOKEN_ASSIGNMENT,      // :=
  TOKEN_COLON,           // :
  TOKEN_DOUBLE_COLON,    // * ::
  TOKEN_DOT,             // .
  TOKEN_COMMA,           // ,
  TOKEN_ARROW,           // =>,
  TOKEN_EQUAL,           // * ==
  TOKEN_NOT_EQUAL,       // * !=
  TOKEN_WRONG_ASSIGN,    // =
  TOKEN_MORE,            // >, illigel again
  TOKEN_LESS,            // * <
  TOKEN_MORE_EQUAL,      // * >=
  TOKEN_LESS_EQUAL,      // * <=
  TOKEN_BIT_AND,         // * &
  TOKEN_BIT_OR,          // * |
  TOKEN_BIT_XOR,         // * ^
  TOKEN_BIT_INV,         // * ~
  TOKEN_LOGIC_NOT,       // * !
  TOKEN_LOGIC_AND,       // * &&
  TOKEN_LOGIC_OR,        // * ||
  TOKEN_BIT_SHIFT_LEFT,  // * <<
  TOKEN_BIT_SHIFT_RIGHT, // * >>
  TOKEN_PLUS,            // * +
  TOKEN_MINUS,           // * -
  TOKEN_STAR,            // * *
  TOKEN_SLASH,           // * /
  TOKEN_PERCENT,         // * %
  TOKEN_PRINT,           // * printl
  TOKEN_TYPE_STRING,     // String, string
  TOKEN_TYPE_INT32,      // Integer, int, i32
  TOKEN_TYPE_INT64,      // i64
  TOKEN_TYPE_INT16,      // i16
  TOKEN_TYPE_U32,        // u32
  TOKEN_TYPE_U16,        // u16
  TOKEN_TYPE_U64,        // u64
  TOKEN_TYPE_REAL,       // Real, real, f32
  TOKEN_TYPE_F64,        // f64
  TOKEN_TYPE_BOOL,       // Boolean, bool
  TOKEN_TYPE_LIST,       // List, list
  TOKEN_TYPE_ARRAY,      // Array, array
  TOKEN_TYPE_ANYVAL,
  TOKEN_TYPE_ANYREF,
  TOKEN_TYPE_TYPE, // * for generics ?
  TOKEN_NEW,
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
 *      | is keyword ? -> return keyword ( + kind )
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

  TokenKind kind;
  std::variant<std::monostate, int, double, std::string> value;
  size_t line;
  size_t column;

  // Mostly single-character and/or special symbols
  Token(TokenKind kind, size_t line, size_t column)
      : kind(kind), line(line), column(column) {};

  // Int value
  Token(TokenKind kind, int intValue, size_t line, size_t column)
      : kind(kind), value(intValue), line(line), column(column) {};

  // Real number
  Token(TokenKind kind, double realValue, size_t line, size_t column)
      : kind(kind), value(realValue), line(line), column(column) {};

  // Identifier or string literal
  Token(TokenKind kind, const std::string &lexem, size_t line, size_t column)
      : kind(kind), value(lexem), line(line), column(column) {};
};

/*
 * Lexical analysis
 *
 * @TODO maybe take a look at the Brzozowski derivatives method
 *       interesting to implement
 */
class Lexer {
public:
  Lexer(std::shared_ptr<SourceBuffer> buffer);
  std::unique_ptr<Token> next();
  std::vector<std::unique_ptr<Token>> lex();
  static const char *getTokenTypeName(TokenKind kind);

private:
  std::shared_ptr<SourceBuffer> source_buffer;
  StateType curr_state;
  size_t curr_line; // @TODO sync with source buffer somehow
  size_t curr_column;
  const char *buffer;

  static bool isSpecial(char c);

  void advance() { buffer++; }
  void rewind() { buffer--; }
  char peek() { return buffer[0]; };

  inline static unsigned int hash(const char *str, size_t len);
  static std::pair<const char *, TokenKind> in_word_set(const char *str,
                                                        size_t len);
};

#endif
