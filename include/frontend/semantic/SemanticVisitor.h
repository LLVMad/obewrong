// SemanticVisitor.h
#ifndef SEMANTIC_VISITOR_H
#define SEMANTIC_VISITOR_H

#include "frontend/SymbolTable.h"
#include "frontend/TypeTable.h"
#include "frontend/parser/Expression.h"

class SemanticVisitor {
public:
  SemanticVisitor(
      std::shared_ptr<SymbolTable> symbolTable, // Изменено на SymbolTable
      std::shared_ptr<GlobalTypeTable> typeTable)
      : symbolTable(symbolTable), typeTable(typeTable) {}

  virtual ~SemanticVisitor() = default;

  virtual void visit(std::shared_ptr<VarDecl> node);
  virtual void visit(std::shared_ptr<MethodDecl> node);
  virtual void visit(std::shared_ptr<MethodCallEXP> node);
  // нужно больше войд визит но ето потом

  // utils
  std::shared_ptr<Type> getExpressionType(std::shared_ptr<Expression> expr);
  bool areTypesCompatible(std::shared_ptr<Type> argType,
                          std::shared_ptr<Type> paramType);
  std::shared_ptr<Decl> lookupInScopes(const std::string &name);

protected:
  std::shared_ptr<SymbolTable> symbolTable; // Новый тип
  std::shared_ptr<GlobalTypeTable> typeTable;

private:
  std::string currentModule{};
};

#endif