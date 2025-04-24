#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include "frontend/SymbolTable.h"
#include "frontend/TypeTable.h"
#include "frontend/parser/Entity.h"
#include <string>
#include <vector>

class SemanticAnalyzer {
public:
  SemanticAnalyzer(std::shared_ptr<GlobalTypeTable> globalTypeTable,
                   std::shared_ptr<SymbolTable> symbolTable)
      : globalTypeTable(globalTypeTable), symbolTable(symbolTable) {
    globalTypeTable->initBuiltinTypes();
    symbolTable->initBuiltinFunctions(globalTypeTable);
  }

  bool analyze(std::shared_ptr<Entity> root);
  const std::vector<std::string> &getErrors() const { return errors; }

private:
  std::shared_ptr<GlobalTypeTable> globalTypeTable;
  std::shared_ptr<SymbolTable> symbolTable;
  std::vector<std::string> errors;
  std::shared_ptr<Scope> currentScope;

  // Declarations
  void checkModuleDecl(const std::shared_ptr<ModuleDecl> &moduleDecl);
  void checkClassDecl(const std::shared_ptr<ClassDecl> &classDecl);
  void checkConstrDecl(const std::shared_ptr<ConstrDecl> &constrDecl);
  void checkVarDecl(const std::shared_ptr<VarDecl> &varDecl);
  void checkFuncDecl(const std::shared_ptr<FuncDecl> &funcDecl);
  void checkFieldDecl(const std::shared_ptr<FieldDecl> &fieldDecl);
  void checkParameterDecl(const std::shared_ptr<ParameterDecl> &paramDecl);
  void checkEnumDecl(const std::shared_ptr<EnumDecl> &enumDecl);
  void checkArrayDecl(const std::shared_ptr<ArrayDecl> &arrayDecl);
  void checkMethodDecl(const std::shared_ptr<MethodDecl> &methodDecl);

  void checkEntity(const std::shared_ptr<Entity> &entity);
  void checkBlock(const std::shared_ptr<Block> &block);

  // Literals
  void checkIntLiteralEXP(const std::shared_ptr<IntLiteralEXP> &lit);
  void checkRealLiteralEXP(const std::shared_ptr<RealLiteralEXP> &lit);
  void checkStringLiteralEXP(const std::shared_ptr<StringLiteralEXP> &lit);
  void checkBoolLiteralEXP(const std::shared_ptr<BoolLiteralEXP> &lit);
  void checkClassNameEXP(const std::shared_ptr<ClassNameEXP> &classRef);

  // Expressions
  void checkVarRefEXP(const std::shared_ptr<VarRefEXP> &varRef);
  void checkFieldRefEXP(const std::shared_ptr<FieldRefEXP> &fieldRef);
  void checkFuncCallEXP(const std::shared_ptr<FuncCallEXP> &funcCall);
  void checkConstructorCallEXP(
      const std::shared_ptr<ConstructorCallEXP> &constrCall);
  void checkArrayLiteralExpr(const std::shared_ptr<ArrayLiteralExpr> &arrayLit);
  void checkBinaryOpEXP(const std::shared_ptr<BinaryOpEXP> &binOp);
  void checkUnaryOpEXP(const std::shared_ptr<UnaryOpEXP> &unaryOp);
  void checkEnumRefEXP(const std::shared_ptr<EnumRefEXP> &enumRef);
  void checkMethodCallEXP(const std::shared_ptr<MethodCallEXP> &methodCall);

  // Statements
  void checkAssignmentSTMT(const std::shared_ptr<AssignmentSTMT> &assignment);
  void checkForSTMT(const std::shared_ptr<ForSTMT> &forStmt);
  void checkWhileSTMT(const std::shared_ptr<WhileSTMT> &whileStmt);
  void checkIfSTMT(const std::shared_ptr<IfSTMT> &ifStmt);
  void checkSwitchSTMT(const std::shared_ptr<SwitchSTMT> &switchStmt);
  void checkCaseSTMT(const std::shared_ptr<CaseSTMT> &caseStmt);
  void checkReturnSTMT(const std::shared_ptr<ReturnSTMT> &returnStmt);
  void checkThisEXP(const std::shared_ptr<ThisEXP> &thisExp);

  void resolveType(const std::shared_ptr<Expression> &expr);
  void reportError(const std::string &message, const Loc &loc);
};

#endif