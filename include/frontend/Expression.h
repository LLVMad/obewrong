#ifndef EXPRESSION_H
#define EXPRESSION_H

#include "Entity.h"

/**
* Base expression entity for AST
*/
class Expression : public Entity {

};

/**
* A literal expression is an expression consisting
* of a single token, rather than a sequence of tokens,
* that immediately and directly denotes the value it evaluates to,
* rather than referring to it by name or some other evaluation rule.
*
* 1, "Hello", 0.95
*/
class LiteralEXP : public Expression {

};

/**
* A class expression creates a class
* same as a class constructor
*
* Integer(2), String("Hi"), Real(1.5)
*/
class ClassEXP : public Expression {

};

/**
* A call expression calls a method
*
* Adder.add(2, 2), Set.pop()
*/
class MethodCallEXP : public Expression {

};

/**
* Evaluates to the location of a field of a struct
*
* Adder.x, MyPair.first, LuxMeter.coeffA
*/
class FieldAccessEXP : public Expression {

};

/**
* Represents chained method calls
*
* a.Minus(2).Plus(5).Div(2)
*/
class CompoundEXP : public Expression {

};

#endif
