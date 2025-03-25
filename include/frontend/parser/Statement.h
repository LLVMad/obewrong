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
  // std::string name;
};

// Identifier := Expression
class AssignmentSTMT : public Statement {
public:
  AssignmentSTMT(std::unique_ptr<VarRefEXP> lhs, std::unique_ptr<Expression> rhs)
      : Statement(E_Assignment), variable(std::move(lhs)), expression(std::move(rhs)) {}

  // children are
  std::unique_ptr<VarRefEXP> variable;
  std::unique_ptr<Expression> expression;
  // void addVariable(std::unique_ptr<VarRefEXP> variable) {
  //   this->children.push_back(std::move(variable));
  // }
  //
  // void addExpression(std::unique_ptr<Expression> expression) {
  //   this->children.push_back(std::move(expression));
  // }
};

// return [ Expression ]
class ReturnSTMT : public Statement {
public:
  explicit ReturnSTMT(std::unique_ptr<Expression> expr)
      : Statement(E_Return_Statement), expr(std::move(expr)) {}

  // children are
  std::unique_ptr<Expression> expr;
  // void addExpression(std::unique_ptr<Expression> expression) {
  //   this->children.push_back(std::move(expression));
  // }
};

// class ElseSTMT : public Statement {
// public:
// }

// if Expression then Body [ else Body ] end
class IfSTMT : public Statement {
public:
  IfSTMT(std::unique_ptr<Expression> condition, std::unique_ptr<Block> ifTrue,
         std::unique_ptr<Block> ifFalse, bool elsed)
      : Statement(E_If_Statement), condition(std::move(condition)),
        ifTrue(std::move(ifTrue)), ifFalse(std::move(ifFalse)), isElsed(elsed) {
  }

  IfSTMT(std::unique_ptr<Expression> condition, std::unique_ptr<Block> ifTrue)
    : Statement(E_If_Statement), condition(std::move(condition)), ifTrue(std::move(ifTrue)), ifFalse(nullptr), isElsed(false) {}

  IfSTMT(std::unique_ptr<Expression> condition, std::unique_ptr<Block> ifTrue, std::unique_ptr<Block> ifFalse)
    : Statement(E_If_Statement), condition(std::move(condition)), ifTrue(std::move(ifTrue)), ifFalse(std::move(ifFalse)), isElsed(true) {}

  // children are
  std::unique_ptr<Expression> condition;
  std::unique_ptr<Block> ifTrue;
  std::unique_ptr<Block> ifFalse;
  bool isElsed;
  // void addCondition(std::unique_ptr<Expression> condition) {
  //   this->children.push_back(std::move(condition));
  // }
  //
  // void addIfTrue(std::unique_ptr<Block> ifTrue) {
  //   this->children.push_back(std::move(ifTrue));
  // }
  //
  // void addIfFalse(std::unique_ptr<Block> ifFalse) {
  //   isElsed = true;
  //   this->children.push_back(std::move(ifFalse));
  // }
};

class CaseSTMT : public Statement {
public:
  CaseSTMT(std::unique_ptr<Expression> condition, std::unique_ptr<Block> body)
    : Statement(E_Case_Statement), condition_literal(std::move(condition)), body(std::move(body)) {}

  std::unique_ptr<Expression> condition_literal;
  std::unique_ptr<Block> body;
};

class SwitchSTMT : public Statement {
public:
  SwitchSTMT(std::unique_ptr<Expression> condition,
             std::vector<std::unique_ptr<CaseSTMT>> cases)
      : Statement(E_Switch_Statement), condition(std::move(condition)),
        cases(std::move(cases)) {}

  explicit SwitchSTMT(std::unique_ptr<Expression> condition)
    : Statement(E_Switch_Statement), condition(std::move(condition)) {}

  std::unique_ptr<Expression> condition;
  std::vector<std::unique_ptr<CaseSTMT>> cases;

  // void addCase(std::unique_ptr<Expression> condition,
  //              std::unique_ptr<Block> body) {
  //   cases.push_back(
  //       std::make_unique<CaseSTMT>(std::move(condition), std::move(body)));
  // }

  void addCase(std::unique_ptr<CaseSTMT> cas) {
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
  explicit WhileSTMT(std::unique_ptr<Expression> condition,
                     std::unique_ptr<Block> body)
      : Statement(E_While_Loop), condition(std::move(condition)),
        body(std::move(body)) {}

  std::unique_ptr<Expression> condition;
  std::unique_ptr<Block> body;
};

#endif