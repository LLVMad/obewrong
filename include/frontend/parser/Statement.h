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
#include "util/Logger.h"
#define DEBUG 1 

class Statement : public Entity {
public:
  explicit Statement(Ekind kind) : Entity(kind) {}
  // std::string name;
};

/**
* A compound statement (also called a block, or block statement)
* is a group of zero or more statements that is treated
* by the compiler as if it were a single statement.
*
* .. is|then
*  ...
* end
*/
class Block : public Statement {
public:
    explicit Block(std::vector<std::unique_ptr<Expression>> body)
        : Statement(E_Block), body(std::move(body)) {}

    std::vector<std::unique_ptr<Expression>> body;

    bool validate() override {
        // Проверяем все выражения в блоке
        for (auto& expr : body) {
            if (!expr->validate()) {
                LOG_ERR_NOARGS("Can't validate block");
                return false;
            }
        }
        LOG_NOARGS("Validate block is OK");
        return true;
    }
};

// Identifier := Expression
class AssignmentSTMT : public Statement {
public:
  AssignmentSTMT(std::unique_ptr<VarDecl> lhs, std::unique_ptr<Expression> rhs)
    : Statement(E_Assignment), variable(std::move(lhs)), expression(std::move(rhs)) {}

  std::unique_ptr<VarDecl> variable;
  std::unique_ptr<Expression> expression;
};

// return [ Expression ]
class ReturnSTMT : public Statement {
public:
  explicit ReturnSTMT(
    std::unique_ptr<Expression> expr)
  : Statement(E_Return_Statement), expr(std::move(expr)) {}

  std::unique_ptr<Expression> expr;
};

// class ElseSTMT : public Statement {
// public:
// }

// if Expression then Body [ else Body ] end
class IfSTMT : public Statement {
public:
  IfSTMT(std::unique_ptr<Expression> condition, std::unique_ptr<Block> ifTrue, std::unique_ptr<Block> ifFalse, bool elsed)
    : Statement(E_If_Statement), condition(std::move(condition)), ifTrue(std::move(ifTrue)), ifFalse(std::move(ifFalse)), isElsed(elsed) {}

  std::unique_ptr<Expression> condition;
  std::unique_ptr<Block> ifTrue;
  std::unique_ptr<Block> ifFalse;
  bool isElsed;

  bool validate() override {
    // Проверяем условие
    if (!condition->validate()) {
      return false;
    }

    // Проверяем блок ifTrue
    if (!ifTrue->validate()) {
      return false;
    }

    // Проверяем блок ifFalse, если он есть
    if (isElsed && !ifFalse->validate()) {
      return false;
    }

    return true;
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
  explicit WhileSTMT(
    std::unique_ptr<Expression> condition,
    std::unique_ptr<Block> body)
      : Statement(E_While_Loop), condition(std::move(condition)), body(std::move(body)) {}

  std::unique_ptr<Expression> condition;
  std::unique_ptr<Block> body;
};

#endif