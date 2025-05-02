#ifndef OBW_VISITOR_H
#define OBW_VISITOR_H
#include "Expression.h"
#include "frontend/types/Decl.h"

#include <iostream>

/**
 * Interface for Visitors
 */
class Visitor {
public:
  virtual ~Visitor() = default;

  //=============== EXPRESSIONS ===============
  virtual void visit(std::shared_ptr<IntLiteralEXP> node) = 0;
  virtual void visit(std::shared_ptr<RealLiteralEXP> node) = 0;
  virtual void visit(std::shared_ptr<StringLiteralEXP> node) = 0;
  virtual void visit(std::shared_ptr<BoolLiteralEXP> node) = 0;
  virtual void visit(std::shared_ptr<ArrayLiteralExpr> node) = 0;
  virtual void visit(std::shared_ptr<VarRefEXP> node) = 0;
  virtual void visit(std::shared_ptr<FieldRefEXP> node) = 0;
  virtual void visit(std::shared_ptr<MethodCallEXP> node) = 0;
  virtual void visit(std::shared_ptr<FuncCallEXP> node) = 0;
  virtual void visit(std::shared_ptr<ClassNameEXP> node) = 0;
  virtual void visit(std::shared_ptr<ConstructorCallEXP> node) = 0;
  // virtual void visit(std::shared_ptr<FieldAccessEXP> node) = 0; ??
  virtual void visit(std::shared_ptr<CompoundEXP> node) = 0;
  virtual void visit(std::shared_ptr<ThisEXP> node) = 0;
  virtual void visit(std::shared_ptr<BinaryOpEXP> node) = 0;
  //=============================================

  //=============== DECLARATIONS ===============
  virtual void visit(std::shared_ptr<FieldDecl> node) = 0;
  virtual void visit(std::shared_ptr<VarDecl> node) = 0;
  virtual void visit(std::shared_ptr<ParameterDecl> node) = 0;
  virtual void visit(std::shared_ptr<MethodDecl> node) = 0;
  virtual void visit(std::shared_ptr<ConstrDecl> node) = 0;
  virtual void visit(std::shared_ptr<FuncDecl> node) = 0;
  virtual void visit(std::shared_ptr<ClassDecl> node) = 0;
  virtual void visit(std::shared_ptr<ArrayDecl> node) = 0;
  virtual void visit(std::shared_ptr<ListDecl> node) = 0;
  virtual void visit(std::shared_ptr<Block> node) = 0;
  virtual void visit(std::shared_ptr<Decl> node) = 0;
  //=============================================

  //=============== STATEMENTS ===============
  virtual void visit(std::shared_ptr<AssignmentSTMT> node) = 0;
  virtual void visit(std::shared_ptr<ReturnSTMT> node) = 0;
  virtual void visit(std::shared_ptr<IfSTMT> node) = 0;
  virtual void visit(std::shared_ptr<CaseSTMT> node) = 0;
  virtual void visit(std::shared_ptr<SwitchSTMT> node) = 0;
  virtual void visit(std::shared_ptr<WhileSTMT> node) = 0;
  virtual void visit(std::shared_ptr<ForSTMT> node) = 0;
  //=============================================
};

#endif
