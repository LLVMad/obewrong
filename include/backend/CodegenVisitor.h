#ifndef OBW_CODEGENVISITOR_H
#define OBW_CODEGENVISITOR_H

#include "frontend/SymbolTable.h"
#include "frontend/parser/Visitor.h"

class CodeGenVisitor {
public:
  CodeGenVisitor(const std::shared_ptr<Scope> &globalScope,
                 const std::shared_ptr<TypeTable> &typeTable,
                 const std::shared_ptr<Entity> &root)
      : globalScope(globalScope), typeTable(typeTable), root(root) {}

  void visit(std::shared_ptr<IntLiteralEXP> node);
  void visit(std::shared_ptr<RealLiteralEXP> node);
  void visit(std::shared_ptr<StringLiteralEXP> node);
  void visit(std::shared_ptr<BoolLiteralEXP> node);
  void visit(std::shared_ptr<ArrayLiteralExpr> node);
  void visit(std::shared_ptr<VarRefEXP> node);
  void visit(std::shared_ptr<FieldRefEXP> node);
  void visit(std::shared_ptr<MethodCallEXP> node);
  void visit(std::shared_ptr<FuncCallEXP> node);
  void visit(std::shared_ptr<ClassNameEXP> node);
  void visit(std::shared_ptr<ConstructorCallEXP> node);
  void visit(std::shared_ptr<CompoundEXP> node);
  void visit(std::shared_ptr<ThisEXP> node);
  void visit(std::shared_ptr<BinaryOpEXP> node);
  void visit(std::shared_ptr<FieldDecl> node);
  void visit(std::shared_ptr<VarDecl> node);
  void visit(std::shared_ptr<ParameterDecl> node);
  void visit(std::shared_ptr<MethodDecl> node);
  void visit(std::shared_ptr<ConstrDecl> node);
  void visit(std::shared_ptr<FuncDecl> node);
  void visit(std::shared_ptr<ClassDecl> node);
  void visit(std::shared_ptr<ArrayDecl> node);
  void visit(std::shared_ptr<ListDecl> node);
  void visit(std::shared_ptr<AssignmentSTMT> node);
  void visit(std::shared_ptr<ReturnSTMT> node);
  void visit(std::shared_ptr<IfSTMT> node);
  void visit(std::shared_ptr<CaseSTMT> node);
  void visit(std::shared_ptr<SwitchSTMT> node);
  void visit(std::shared_ptr<WhileSTMT> node);
  void visit(std::shared_ptr<ForSTMT> node);

private:
  // entry for a symboltable (actually a tree of scopes)
  std::shared_ptr<Scope> globalScope;
  // global type table
  std::shared_ptr<TypeTable> typeTable;
  // root node of an AST
  std::shared_ptr<Entity> root;
};

#endif
