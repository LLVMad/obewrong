#ifndef STATEMENT_H
#define STATEMENT_H

/*
 * Entities that control
 * the control-flow of program
 */
#include <memory>
#include <utility>
#include <vector>

#include "Entity.h"
#include "Expression.h"
#include "frontend/types/Decl.h"
#include "frontend/types/Types.h"

class Statement : public Entity {
public:
  explicit Statement(Ekind kind) : Entity(kind) {}

  std::shared_ptr<Type> resolveType(TypeTable typeTable) override;

  bool validate() override;
  // std::string name;
  ~Statement() override;
};

// Identifier := Expression
class AssignmentSTMT : public Statement {
public:
  AssignmentSTMT(std::shared_ptr<VarRefEXP> lhs,
                 std::shared_ptr<Expression> rhs)
      : Statement(E_Assignment), variable(std::move(lhs)), field(nullptr),
        expression(std::move(rhs)) {}

  AssignmentSTMT(std::shared_ptr<FieldRefEXP> lhs,
                 std::shared_ptr<Expression> rhs)
      : Statement(E_Assignment), variable(nullptr), field(std::move(lhs)),
        expression(std::move(rhs)) {}

  // children are
  std::shared_ptr<VarRefEXP> variable;
  std::shared_ptr<FieldRefEXP> field;
  std::shared_ptr<Expression> expression;

  ~AssignmentSTMT() override = default;
  // void addVariable(std::shared_ptr<VarRefEXP> variable) {
  //   this->children.push_back(std::move(variable));
  // }
  //
  // void addExpression(std::shared_ptr<Expression> expression) {
  //   this->children.push_back(std::move(expression));
  // }
};

// return [ Expression ]
class ReturnSTMT : public Statement {
public:
  explicit ReturnSTMT(std::shared_ptr<Expression> expr)
      : Statement(E_Return_Statement), expr(std::move(expr)) {}

  // children are
  std::shared_ptr<Expression> expr;
  ~ReturnSTMT() override = default;
  // void addExpression(std::shared_ptr<Expression> expression) {
  //   this->children.push_back(std::move(expression));
  // }
};

// class ElseSTMT : public Statement {
// public:
// }

// if Expression then Body [ else Body ] end
class IfSTMT : public Statement {
public:
  IfSTMT(std::shared_ptr<Expression> condition, std::shared_ptr<Block> ifTrue,
         std::shared_ptr<Block> ifFalse, bool elsed)
      : Statement(E_If_Statement), condition(std::move(condition)),
        ifTrue(std::move(ifTrue)), ifFalse(std::move(ifFalse)), isElsed(elsed) {
  }

  IfSTMT(std::shared_ptr<Expression> condition, std::shared_ptr<Block> ifTrue)
      : Statement(E_If_Statement), condition(std::move(condition)),
        ifTrue(std::move(ifTrue)), ifFalse(nullptr), isElsed(false) {}

  IfSTMT(std::shared_ptr<Expression> condition, std::shared_ptr<Block> ifTrue,
         std::shared_ptr<Entity> ifFalse)
      : Statement(E_If_Statement), condition(std::move(condition)),
        ifTrue(std::move(ifTrue)), ifFalse(std::move(ifFalse)), isElsed(true) {}

  // children are
  std::shared_ptr<Expression> condition;
  std::shared_ptr<Block> ifTrue;
  std::shared_ptr<Entity> ifFalse;
  bool isElsed;

  ~IfSTMT() override = default;
};

class CaseSTMT : public Statement {
public:
  CaseSTMT(std::shared_ptr<Expression> condition, std::shared_ptr<Block> body)
      : Statement(E_Case_Statement), condition_literal(std::move(condition)),
        body(std::move(body)), isDefault(false) {}

  CaseSTMT(std::shared_ptr<Block> body)
      : Statement(E_Case_Statement), body(std::move(body)), isDefault(true) {}

  std::shared_ptr<Expression> condition_literal;
  std::shared_ptr<Block> body;
  bool isDefault;

  ~CaseSTMT() override = default;
};

class SwitchSTMT : public Statement {
public:
  SwitchSTMT(std::shared_ptr<Expression> condition,
             std::vector<std::shared_ptr<CaseSTMT>> cases)
      : Statement(E_Switch_Statement), condition(std::move(condition)),
        cases(std::move(cases)) {}

  explicit SwitchSTMT(std::shared_ptr<Expression> condition)
      : Statement(E_Switch_Statement), condition(std::move(condition)) {}

  std::shared_ptr<Expression> condition;
  std::vector<std::shared_ptr<CaseSTMT>> cases;

  ~SwitchSTMT() override = default;

  // void addCase(std::shared_ptr<Expression> condition,
  //              std::shared_ptr<Block> body) {
  //   cases.push_back(
  //       std::make_unique<CaseSTMT>(std::move(condition), std::move(body)));
  // }

  void addCase(std::shared_ptr<CaseSTMT> cas) {
    cases.push_back(std::move(cas));
  }
};

/**
 * A while loop begins by evaluating
 * the boolean loop conditional operand.
 * If the loop conditional operand evaluates to true,
 * the loop body block executes,
 * then control returns to the loop conditional operand.
 *
 * while a.Less(0) then
 *   a.Minus(1)
 * end
 */
class WhileSTMT : public Statement {
public:
  explicit WhileSTMT(std::shared_ptr<Expression> condition,
                     std::shared_ptr<Block> body)
      : Statement(E_While_Loop), condition(std::move(condition)),
        body(std::move(body)) {}

  std::shared_ptr<Expression> condition;
  std::shared_ptr<Block> body;

  ~WhileSTMT() override = default;
};

class ForSTMT : public Statement {
public:
  explicit ForSTMT(std::shared_ptr<AssignmentSTMT> varWithAss,
                   std::shared_ptr<Expression> condition,
                   std::shared_ptr<AssignmentSTMT> post,
                   std::shared_ptr<Block> body)
      : Statement(E_While_Loop), varWithAss(std::move(varWithAss)),
        condition(std::move(condition)), post(std::move(post)),
        body(std::move(body)) {}

  std::shared_ptr<AssignmentSTMT> varWithAss;
  std::shared_ptr<Expression> condition;
  std::shared_ptr<AssignmentSTMT> post;
  std::shared_ptr<Block> body;

  ~ForSTMT() override = default;
};

#endif