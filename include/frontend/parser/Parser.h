#ifndef OBW_PARSER_H
#define OBW_PARSER_H

#include <utility>
#include <vector>

#include "../SymbolTable.h"
#include "../TypeTable.h"
#include "Entity.h"
#include "frontend/lexer/Lexer.h"
#include "frontend/types/Decl.h"

#include <stack>
#include <stdexcept>

class Parser {
  // SymbolTable symbolTable;
  std::shared_ptr<GlobalSymbolTable> globalSymbolTable;
  std::shared_ptr<GlobalTypeTable> globalTypeTable;

public:
  Parser(std::vector<std::unique_ptr<Token>> tokens,
         const std::shared_ptr<GlobalSymbolTable> &globalSymbolTable,
         const std::shared_ptr<GlobalTypeTable> &globalTypeTable)
      : globalSymbolTable(globalSymbolTable), globalTypeTable(globalTypeTable), tokens(std::move(tokens)), tokenPos(-1) {
    lastDeclaredScopeParent.emplace("Global");
    globalTypeTable->initBuiltinTypes();
    globalSymbolTable->initBuiltinFuncs(globalTypeTable);
    // symbolTable.enterScope();
  }

  ~Parser() {
    // globalSymbolTable.moduleSymbolTables.clear();
    // globalTypeTable.types.clear();
  }

  /**
   * @brief Main function for parsing a program
   * @return pointer to a root of an AAST tree
   */
  std::shared_ptr<Entity> parseProgram();

private:
  std::vector<std::unique_ptr<Token>> tokens;

  std::shared_ptr<Entity> current_scope;

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

  std::shared_ptr<Entity> parseIfStatement();

  std::shared_ptr<Entity> parseReturnStatement();

  std::shared_ptr<Entity> parseSwitch();

  std::shared_ptr<Entity> parseCase();

  std::shared_ptr<Entity> parseVarDecl();

  std::shared_ptr<Entity> parseConstructorDecl();

  std::shared_ptr<Entity> parseMethodDecl();

  std::shared_ptr<Entity> parseFunctionDecl();

  std::shared_ptr<Entity> parseClassDecl();

  std::shared_ptr<Entity> parseFieldDecl();

  /**
   * @note WhileLoop ::= \n
   * "while" Expression "loop" Body "end"
   *
   */
  std::shared_ptr<Entity> parseWhileStatement();

  /**
    * @note ForLoop ::=\n
      "for" Assignment ";" Expression ";" Expression ";" "is"\n
      Body\n
      "end"
   */
  std::shared_ptr<Entity> parseForStatement();

  std::shared_ptr<Entity> parseAssignment();

  /**
   * @note Block \n
   *  : "is" ... "end" \n
   *  this creature of a function should
   *  parse WHATEVER construct is between is and end
   *  could be ANYTHING!!!
   */
  std::shared_ptr<Entity> parseBlock(BlockKind blockKind);

  /**
   * @note Expression \n
   * : Primary { . Identifier [ Arguments ] }
   *
   * @note also parses functioncalls and compund expressions
   */
  std::shared_ptr<Entity> parseExpression();

  /**
   *
   */
  std::shared_ptr<ParameterDecl> parseParameterDecl();

  /**
   *
   */
  void parseParameters(const std::shared_ptr<FuncDecl>& funcDecl);
  void parseParameters(const std::shared_ptr<ConstrDecl>& constrDecl);
  void parseParameters(const std::shared_ptr<MethodDecl>& funcDecl);

  /**
   * @brief Parses arguments in a method call
   * @param method_name name of a method, that we read arguments for, that will
   * be called
   */
  void parseArguments(const std::shared_ptr<MethodCallEXP> &method_name);

  void parseArguments(const std::shared_ptr<ConstructorCallEXP> &constr_name);

  /**
   * @brief Parses arguments in a function call
   * or a constructor and adds them as a children of method_name
   * @param function_name name of a func, that we read arguments for, that will
   * be called
   */
  void parseArguments(const std::shared_ptr<FuncCallEXP> &function_name);

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
  std::shared_ptr<Entity> parsePrimary();

  // variables to keep track
  // of declared important constructs
  // and their scope
  std::string lastDeclaredClass;
  std::string lastDeclaredMethod;
  std::string lastDeclaredFunction;

  std::stack<std::string> lastDeclaredScopeParent;

  std::string moduleName;
};

#endif
