#ifndef STATEMENT_H
#define STATEMENT_H

#include <memory>

#include "Entity.h"
#include "Expression.h"

/**
* Base statement entity
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
  Expression condition;
  BlockSTMT whileTrue;
};

/**
* Basic "block" of every OBW programm
* constuct a new type of <ClassName>
* with constructor, fields and methods
*
* class MyClass is
*   var a : Integer
*   this() is ... end
*   method get() => return a
* end
*/
class ClassDeclSTMT : public Statement {

};

/**
* Declaration of a method in a class body (block)
*
* BEWARE: this can be:
*   - forward declaration: `method short() : Integer` (no body)
*   - short syntax decl  : `method get() => return this.a`
*   - full               : `method set(a: Integer) is ... end`
*/
class MethodDeclSTMT : public Statement {

};

/**
* Declaration of a new type (class) constructor
*
* this() is ... end
* this(a: Integer) => this.a = a
*/
class ConstructoDeclSTMT : public Statement {

};

/**
* Declaration of a class field
* looks like a variable declaration,
* except is **cant invoke constructors**
*
* <ClassDeclaration>
*   var a : Integer
*   var coeff : Real
*   ...
* end
*/
class FieldDeclSTMT : public Statement {

};

/**
* Declaration of method parameters
*
* (a: Integer, b : Real)
*/
class ParamDeclSTMT : public Statement {

};

/**
* Declaration of arguments passed to method call
*
* (a, 5, "Hi")
*/
class ArgumentDeclSTMT : public Statement {

};

#endif