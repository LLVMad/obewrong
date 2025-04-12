#ifndef OBW_PARSEERROR_H
#define OBW_PARSEERROR_H

#include "frontend/lexer/Lexer.h"
#include <exception>
#include <string>

/**
 * Wrong next token in a token stream
 * while expecting a different token
 */
class TokenNotExpected : public std::exception {
public:
  TokenNotExpected(TokenKind expected, TokenKind found, size_t line,
                   size_t column, const std::string &filename)
      : expectedKind(expected), foundKind(found), line(line), column(column),
        filename(filename) {}

  const char *what() const noexcept override {
    std::string msg =
        filename + ":" + std::to_string(line) + ":" + std::to_string(column) +
        " "
        " Parse error: expected token '" +
        std::string(Lexer::getTokenTypeName(expectedKind)) + "' but found '" +
        std::string(Lexer::getTokenTypeName(foundKind)) + "\n";
    return msg.c_str();
  }

private:
  TokenKind expectedKind;
  TokenKind foundKind;
  size_t line;
  size_t column;
  std::string filename;
};

#endif
