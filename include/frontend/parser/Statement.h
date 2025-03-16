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

/**
 * Base statement entity
 *
 * Yes, this is a dupliacte of Decl class
 * this is done just for distinction
 * well, maybe this is incorrect idk
 */
class Statement : public Entity {
  explicit Statement(Ekind kind, std::string name) : Entity(kind), name(std::move(name)) {}
  std::string name;
  // points to a declaration in which scope this stmt is
  // i.e. some ClassDecl or FuncDecl
  std::unique_ptr<Decl> scope;
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
  explicit Block(
    Ekind kind,
    const std::string &name,
    std::vector<std::unique_ptr<Expression>> body)
    : Statement(kind, name), body(std::move(body)) {}

  std::vector<std::unique_ptr<Expression>> body;
};

// Identifier := Expression
// class AssignmentSTMT : public Statement {
//
// };

// return [ Expression ]
class ReturnSTMT : public Statement {
public:
  explicit ReturnSTMT(
    Ekind kind,
    const std::string &name,
    std::unique_ptr<Expression> expr)
  : Statement(kind, name), expr(std::move(expr)) {}

  std::unique_ptr<Expression> expr;
};

// class ElseSTMT : public Statement {
// public:
// }

// if Expression then Body [ else Body ] end
class IfSTMT : public Statement {
public:
  std::unique_ptr<Expression> condition;
  Block ifTrue;
  Block ifFalse;
  bool isElsed;
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
    Ekind kind,
    std::string name,
    std::unique_ptr<Expression> condition,
    std::unique_ptr<Block> body)
      : Statement(kind, std::move(name)), condition(std::move(condition)), body(std::move(body)) {}

  std::unique_ptr<Expression> condition;
  std::unique_ptr<Block> body;
};

#endif