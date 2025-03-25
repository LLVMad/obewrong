#ifndef OBW_PARSER_H
#define OBW_PARSER_H

#include <vector>

#include "Entity.h"
#include "frontend/lexer/Lexer.h"
#include "frontend/parser/SymbolTable.h"
#include "frontend/parser/TypeTable.h"
#include "frontend/types/Decl.h"
#include <stdexcept>

class Parser {
  SymbolTable symbolTable;
  TypeTable typeTable;

  void initBuiltinTypes() {
    typeTable.addType("Integer", std::make_unique<TypeInt>());
    typeTable.addType("Real", std::make_unique<TypeReal>());
    typeTable.addType("Bool", std::make_unique<TypeBool>());
    typeTable.addType("String", std::make_unique<TypeString>());
  }

public:
  Parser(std::vector<std::unique_ptr<Token>> tokens)
      : tokens(std::move(tokens)), tokenPos(-1) {
    initBuiltinTypes();
    symbolTable.enterScope();
  }

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

  std::unique_ptr<Entity> parseIfStatement();

  std::unique_ptr<Entity> parseReturnStatement();

  std::unique_ptr<Entity> parseSwitch();

  std::unique_ptr<Entity> parseCase();

  std::unique_ptr<Entity> parseVarDecl();

  std::unique_ptr<Entity> parseFunctionDecl();

  std::unique_ptr<Entity> parseClassDecl();

  std::unique_ptr<Entity> parseFieldDecl();

  std::unique_ptr<Entity> parseWhileStatement();

  std::unique_ptr<Entity> parseForStatement();

  std::unique_ptr<Entity> parseAssignment();

  /**
   * @note Block \n
   *  : "is" ... "end" \n
   *  this creature of a function should
   *  parse WHATEVER construct is between is and end
   *  could be ANYTHING!!!
   */
  std::unique_ptr<Entity> parseBlock();

  /**
   * @note Expression \n
   * : Primary { . Identifier [ Arguments ] }
   *
   * @note also parses functioncalls and compund expressions
   */
  std::unique_ptr<Entity> parseExpression();

  void parseParameters(const std::unique_ptr<FuncDecl>& funcDecl);

  void parseParameters(const std::unique_ptr<MethodDecl>& funcDecl);

  /**
   * @brief Parses arguments in a method call
   * @param method_name name of a method, that we read arguments for, that will
   * be called
   */
  void parseArguments(const std::unique_ptr<MethodCallEXP> &method_name);

  /**
   * @brief Parses arguments in a function call
   * or a constructor and adds them as a children of method_name
   * @param function_name name of a func, that we read arguments for, that will
   * be called
   */
  void parseArguments(const std::unique_ptr<FuncCallEXP> &function_name);

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
    | Identifier \n
   */
  std::unique_ptr<Entity> parsePrimary();
};

#endif
