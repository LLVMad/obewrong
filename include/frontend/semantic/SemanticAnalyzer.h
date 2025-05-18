#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include "frontend/SymbolTable.h"
#include "frontend/TypeTable.h"
#include "frontend/parser/Entity.h"
#include <string>
#include <vector>

/*
class SemanticAnalyzer {
public:
  SemanticAnalyzer(std::shared_ptr<GlobalTypeTable> globalTypeTable,
                   std::shared_ptr<SymbolTable> symbolTable)
      : globalTypeTable(globalTypeTable), symbolTable(symbolTable) {}

  bool analyze(std::shared_ptr<Entity> root);
  const std::vector<std::string> &getErrors() const { return errors; }

private:
  std::shared_ptr<GlobalTypeTable> globalTypeTable;
  std::shared_ptr<SymbolTable> symbolTable;
  std::vector<std::string> errors;
  std::shared_ptr<Scope> currentScope;

  void checkEntity(const std::shared_ptr<Entity> &entity);
  void checkBlock(const std::shared_ptr<Block> &block);
  void checkMethodCall(const std::shared_ptr<MethodCallEXP> &methodCall);
  // void checkAssignment(const std::shared_ptr<AssignmentSTMT>& assignment);
  // void checkVariableDecl(const std::shared_ptr<VarDecl>& varDecl);
  void checkFunctionDecl(const std::shared_ptr<FuncDecl> &funcDecl);
  void resolveType(const std::shared_ptr<Expression> &expr);
  void reportError(const std::string &message, const Loc &loc);
}; */

#endif