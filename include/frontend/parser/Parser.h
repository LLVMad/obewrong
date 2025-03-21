#ifndef OBW_PARSER_H
#define OBW_PARSER_H

#include <vector>

#include "Entity.h"
#include "frontend/lexer/Lexer.h"
#include "frontend/types/Decl.h"
#include "util/Logger.h"
#include "Statement.h"

class Parser {
public:
  Parser(std::vector<std::unique_ptr<Token>> tokens)
    : tokens(std::move(tokens)), tokenPos(-1) {}

  /**
   * @brief Main function for parsing a program
   * @return pointer to a root of an AAST tree
   */
  std::unique_ptr<Entity> parseProgram();

private:
  std::vector<std::unique_ptr<Token>> tokens;

  std::unique_ptr<Entity> current_scope;

  /**
   * @brief eats current token
   * @return next token from `tokens`
   */
  std::unique_ptr<Token> next();

  /**
   * @brief peek to next token
   * doesnt it the token
   * @return next token
   */
  std::unique_ptr<Token> peek();

  int tokenPos;

  // void parseProgramDecls();
  // void parseClassDeclaration();
  // void parseMemberDeclaration();
  // void parseVariableDeclaration();
  // void parseMethodDeclaration();
  // void parseMethodHeader();
  // void parseMethodBody();
  // void parseParameters();
  // void parseBody();
  // void parseStatement();
  // void parseAssignmentStatement();
  // void parseIfStatement();
  // void parseWhileStatement();
  // void parseReturnStatement();
  // void parseExpression();
  // void parseMethodCall();
  // void parseArguments();
  // void parseConstructorDeclaration();

  /**
   * @note Expression \n
   * : Primary { . Identifier [ Arguments ] }
   */
  std::unique_ptr<Entity> parseExpression();

  /**
   * @brief Parses arguments in a method call
   * or a constructor and adds them as a children of method_name
   * @param method_name name of a method, that we read arguments for, that will be called
   */
  void parseArguments(const std::unique_ptr<MethodCallEXP> &method_name);

  /**
   * @brief this is the last point
   * in a chain of top-down parsing
   *
   * @note Primary ::= \n
    | IntegerLiteral \n
    | RealLiteral \n
    | BooleanLiteral \n
    | StringLiteral \n
    | this \n
    | ClassName \n
   */
  std::unique_ptr<Entity> parsePrimary();
};

#endif
