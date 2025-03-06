#ifndef STATEMENT_H
#define STATEMENT_H

#include <memory>

#include "Entity.h"
#include "Expression.h"

/**
* Base statement entity for AST
*/
class Statement : public Entity {

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
class BlockSTMT : public Statement {

};

// Identifier := Expression
class AssignmentSTMT : public Statement {

};

// return [ Expression ]
class ReturnSTMT : public Statement {

};

// if Expression then Body [ else Body ] end
class IfSTMT : public Statement {
  Expression condition;
  BlockSTMT ifTrue;
  BlockSTMT ifFalse;
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

};

class MethodDeclSTMT : public Statement {

};

class ClassDeclSTMT : public Statement {

};
//enum BlockKind {
//  BLOCK_IF,
//  BLOCK_ELSE,
//  BLOCK_WHILE
//};



#endif
