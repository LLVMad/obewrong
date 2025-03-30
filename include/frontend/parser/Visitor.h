#ifndef VISITOR_H
#define VISITOR_H
#include "Expression.h"

#include <iostream>

/**
 * Interface for Visitors
 */
class Visitor {
public:
  virtual ~Visitor() = default;

  virtual void visit(std::shared_ptr<IntLiteralEXP> node) = 0;
  virtual void visit(std::shared_ptr<RealLiteralEXP> node) = 0;
  virtual void visit(std::shared_ptr<StringLiteralEXP> node) = 0;
  virtual void visit(std::shared_ptr<BoolLiteralEXP> node) = 0;
};

/**
 * Prints AST tree
 *
 * @example \n
 * ModuleSTMT | "simple" \n
 *   ClassDecl | "Simple" \n
 *     FieldDecl | "a Integer" \n
 *
 *     MethodDecl | "add" \n
 *       ParameterDecl | "a" \n
 *         AssignmentSTMT | "b := .." \n
 *           VarRefEXP | "a" \n
 *           MethodCallEXPR | "a.UnaryMinus()" \n
 *         MethodCallEXPR | "a.Plus(b)" \n
 *
 *     ConstrDecl | "..." \n
 *       ParamDecl | "b" \n
 *         AssignemtnEXPR \n
 *           FieldRefEXPR | "a" \n
 *           ...
 */
class PrintVisitor : public Visitor {
public:
  void visit(std::shared_ptr<IntLiteralEXP> node) override {
    std::cout << node->getValue() << std::endl;
  };
};

#endif
