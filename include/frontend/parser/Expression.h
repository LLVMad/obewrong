#ifndef EXPRESSION_H
#define EXPRESSION_H

#include "Entity.h"

/**
* Base expression entity
*/
class Expression : public Entity {
public:
  explicit Expression(Ekind kind) : Entity(kind) {};
  Ekind resolveType() override;
  bool validate() override;
};

/**
* A literal expression is an expression consisting
* of a single token, rather than a sequence of tokens,
* that immediately and directly denotes the value it evaluates to,
* rather than referring to it by name or some other evaluation rule.
*
* 1, "Hello", 0.95
*/
class IntLiteralEXP : public Expression {
public:
  IntLiteralEXP(int val) : Expression(E_Literal), _value(val) {};

  int getValue() { return _value; }

  Ekind resolveType() override { return E_Integer_Type; }
private:
  int _value;
};

class RealLiteralEXP : public Expression {
public:
  RealLiteralEXP(double val) : Expression(E_Literal), _value(val) {};

  double getValue() { return _value; }

  Ekind resolveType() override { return E_Real_Type; }
private:
  double _value;
};

class StringLiteralEXP : public Expression {
public:
  StringLiteralEXP(std::string val) : Expression(E_Literal), _value(val) {};

  std::string getValue() { return _value; }

  Ekind resolveType() override { return E_String_Type; }
private:
  std::string _value;
};

class BoolLiteralEXP : public Expression {
public:
  BoolLiteralEXP(bool val) : Expression(E_Literal), _value(val) {};

  bool getValue() { return _value; }

  Ekind resolveType() override { return E_Boolean_Type; }
private:
  bool _value;
};

/**
* A call expression calls a method
*
* Adder.add(2, 2), Set.pop()
*/
class MethodCallEXP : public Expression {

};

/**
* Evaluates to a class field
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

class ThisEXP : public Expression {

};

/**
* A class name expression creates a class
* same as a class constructor ?
*
* Type system: Each language construct (operator, expression,
* statement, …) is associated with a type expression. The type
* system is a collection of rules for assigning type expressions
* to these constructs
*
* Integer(2), String("Hi"), Real(1.5), Integer.Min
*/
class ClassNameEXP : public Expression {
public:
  ClassNameEXP(std::string name) : Expression(E_New_Type), _name(name) {};
private:
  std::string _name;
};

#endif
