// SemanticVisitor.h
#ifndef SEMANTIC_VISITOR_H
#define SEMANTIC_VISITOR_H

#include "frontend/SymbolTable.h"
#include "frontend/TypeTable.h"
#include "frontend/parser/Expression.h"

class SemanticVisitor {
public:
  SemanticVisitor(std::shared_ptr<GlobalSymbolTable> symbolTable,
                  std::shared_ptr<GlobalTypeTable> typeTable)
      : symbolTable(symbolTable), typeTable(typeTable),
        currentModule("Global") {}

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
  std::shared_ptr<GlobalSymbolTable> symbolTable;
  std::shared_ptr<GlobalTypeTable> typeTable;
  std::vector<std::string> scopeStack; // Текущий контекст областей видимости
  std::unordered_map<std::string,
                     std::unordered_map<std::string, std::shared_ptr<Decl>>>
      localScopes;

private:
  std::string currentModule{};
};

#endif