#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <utility>

#include "Entity.h"

/**
* Base expression entity
*/
class Expression : public Entity {
public:
  explicit Expression(Ekind kind) : Entity(kind) {};
  std::unique_ptr<Type> resolveType() override;
  // add evaluate method
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
  IntLiteralEXP(int val) : Expression(E_Integer_Literal), _value(val) {};

  int getValue() { return _value; }

  std::unique_ptr<Type> resolveType() override {
    auto type = std::make_unique<TypeBuiltin>(TYPE_INT, "Integer", 32);
    return type;
  }
private:
  int _value;
};

class RealLiteralEXP : public Expression {
public:
  RealLiteralEXP(double val) : Expression(E_Real_Literal), _value(val) {};

  double getValue() { return _value; }

  std::unique_ptr<Type> resolveType() override {
    auto type = std::make_unique<TypeBuiltin>(TYPE_FLOAT, "Real", 32);
    return type;
  }
private:
  double _value;
};

// @TODO
// class StringLiteralEXP : public Expression {
// public:
//   StringLiteralEXP(std::string val) : Expression(E_Literal), _value(val) {};
//
//   std::string getValue() { return _value; }
//
//   std::unique_ptr<Type> resolveType() override {
//     auto type = std::make_unique<TypeBuiltin>(TYPE_STRING, "String", 32);
//     return E_String_Type;
//   }
// private:
//   std::string _value;
// };

class BoolLiteralEXP : public Expression {
public:
  BoolLiteralEXP(bool val) : Expression(E_Boolean_Literal), _value(val) {};

  bool getValue() { return _value; }

  std::unique_ptr<Type> resolveType() override {
    auto type = std::make_unique<TypeBuiltin>(TYPE_BOOL, "Bool", 1);
    return type;
  }
private:
  bool _value;
};

class VarRefEXP : public Expression {

};

/**
* A call expression calls a method
*
* var adder : Adder
* adder.add(2, 2)
*
* ALSO: 2.Plus(2)
*/
class MethodCallEXP : public Expression {
public:
  MethodCallEXP(
    std::unique_ptr<Expression> left,
    std::string method_name,
    std::vector<std::unique_ptr<Expression>> args
  ) : Expression(E_Function),
      left(std::move(left)),
      method_name(std::move(method_name)),
      arguments(std::move(args)) {}

  std::unique_ptr<Expression> left;
  std::string method_name;
  std::vector<std::unique_ptr<Expression>> arguments;

  std::unique_ptr<Type> resolveType() override {
    // @TODO
    return nullptr;
  }

  bool validate() override {
    // Ensure the class exists and arguments match the constructor.
    return true;
  }
};

/**
* Evaluates to a class field
*
* Adder.x, MyPair.first, LuxMeter.et.pop()coeffA
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
  ClassNameEXP(std::string name) : Expression(E_Class_Name), _name(std::move(name)) {};
private:
  std::string _name;
};

#endif
