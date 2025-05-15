#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <utility>

#include "Entity.h"
#include "../Scope.h"
#include "../TypeTable.h"
#include "frontend/SymbolTable.h"

/**
 * Base expression entity
 */
class Expression : public Entity {
public:
  explicit Expression(Ekind kind) : Entity(kind) {};
  explicit Expression(Ekind kind, std::string name) : Entity(kind, std::move(name)) {};

  std::shared_ptr<Type> resolveType(const TypeTable &typeTable, const std::shared_ptr<Scope<Entity>> &currentScope) override {
    (void)typeTable;
    return nullptr;
  }
  // add evaluate method
  bool validate() override { return false; };
  ~Expression() override;
};

class DummyExpression : public Expression {
public:
  explicit DummyExpression(const std::string &name)
      : Expression(E_Dummy, name) {}
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
  IntLiteralEXP(int val, size_t bytesize)
    : Expression(E_Integer_Literal, std::to_string(val)), bytesize(bytesize), _value(val) {};

  size_t getByteSize() const { return bytesize; };
  int getValue() const { return _value; }

  std::shared_ptr<Type> resolveType(const TypeTable &typeTable, const std::shared_ptr<Scope<Entity>> &currentScope) override;

private:
  size_t bytesize;
  int _value;
};

class RealLiteralEXP : public Expression {
public:
  RealLiteralEXP(double val)
    : Expression(E_Real_Literal, std::to_string(val)), _value(val) {};

  double getValue() { return _value; }

  std::shared_ptr<Type> resolveType(const TypeTable &typeTable, const std::shared_ptr<Scope<Entity>> &currentScope) override;

private:
  double _value;
};

class StringLiteralEXP : public Expression {
public:
  StringLiteralEXP(std::string val)
      : Expression(E_String_Literal), value(std::move(val)) {
     value.erase(
     std::remove(value.begin(), value.end(), '\"'),
     value.end()
     );
  };

  std::string value;

  std::shared_ptr<Type> resolveType(const TypeTable &typeTable, const std::shared_ptr<Scope<Entity>> &currentScope) override;
};

class BoolLiteralEXP : public Expression {
public:
  BoolLiteralEXP(bool val) : Expression(E_Boolean_Literal), _value(val) {};

  bool getValue() const { return _value; }

  std::shared_ptr<Type> resolveType(const TypeTable &typeTable, const std::shared_ptr<Scope<Entity>> &currentScope) override;

private:
  bool _value;
};

/**
 * Enumiration of array elements
 * between [ and ] separated by comma
 *
 * var arr : Array[Integer] := [1, 2, 3, 4]
 *                             ^^^^^^^^^^^
 */
class ArrayLiteralExpr : public Expression {
public:
  ArrayLiteralExpr(std::vector<std::shared_ptr<Expression>> elements)
      : Expression(E_Array_Literal), elements(std::move(elements))
  {
    switch (this->elements[0]->getKind()) {
      case E_Integer_Literal: el_type = TYPE_INT; break;
      case E_Real_Literal: el_type = TYPE_REAL; break;
      case E_String_Literal: el_type = TYPE_STRING; break;
      case E_Boolean_Literal: el_type = TYPE_BOOL; break;
      case E_Array_Literal: el_type = TYPE_ARRAY; break;
      default: el_type = TYPE_UNKNOWN; break;
    }
  };

  // children should be
  std::vector<std::shared_ptr<Expression>> elements;
  // void addElement(std::shared_ptr<Expression> expr) {
  //
  // }

  TypeKind el_type;

  std::shared_ptr<Type> resolveType(const TypeTable &typeTable, const std::shared_ptr<Scope<Entity>> &currentScope) override;

  bool validate() override;
};

/**
 * For when we refer to a "value" by
 * referencing associated variable name
 *
 * can/should be translate to other Decl, STMT or EXPR
 *
 * var a : Integer := 2
 * var c : Integer := a.Plus(2)
 *                    ^
 */
class VarRefEXP : public Expression {
public:
  VarRefEXP(const std::string &name)
      : Expression(E_Var_Reference, name) {};

  // no children, link to a VarDecl probably

  std::shared_ptr<Type> resolveType(const TypeTable &typeTable, const std::shared_ptr<Scope<Entity>> &currentScope) override;

  bool validate() override;
};

/**
 * For when we refer to a "value" by
 * referencing associated variable name
 *
 * can/should be translate to other Decl, STMT or EXPR
 *
 * class A is
 *   var a : Integer
 * end
 *
 * var cl : A
 * var c : Integer := cl.a
 *                    ^^^^
 */
class FieldRefEXP : public Expression {
public:
  FieldRefEXP(std::string name, std::shared_ptr<VarRefEXP> obj)
      : Expression(E_Field_Reference, std::move(name)), obj(obj) {};

  FieldRefEXP(std::string name)
      : Expression(E_Field_Reference, std::move(name)),
        obj(nullptr) {};

  std::shared_ptr<VarRefEXP> obj; // object which field is referenced
  size_t index;

  std::shared_ptr<Type> resolveType(const TypeTable &typeTable, const std::shared_ptr<Scope<Entity>> &currentScope) override;
  bool validate() override;
};

// class ArrayRefEXP : public Expression {
// public:
//
// };

class ElementRefEXP : public Expression {
public:
  ElementRefEXP(std::shared_ptr<Expression> index,
                std::shared_ptr<VarRefEXP> arr)
      : Expression(E_Element_Reference), arr(arr), index(index) {};

  ElementRefEXP()
      : Expression(E_Element_Reference), arr(nullptr), index(nullptr) {};

  std::shared_ptr<VarRefEXP> arr;
  std::shared_ptr<Expression> index;
};

/**
 * Represents object creation by invoking a constructor
 *
 * var adder : Adder
 * adder.add(2, 2)
 *
 * ALSO: 2.Plus(2)
 */
class MethodCallEXP : public Expression {
public:
  MethodCallEXP(std::string method_name, std::shared_ptr<Expression> left,
                std::vector<std::shared_ptr<Expression>> arguments)
      : Expression(E_Method_Call, method_name),
        left(left), arguments(arguments) {};

  MethodCallEXP() : Expression(E_Method_Call) {}

  MethodCallEXP(const std::string &name)
      : Expression(E_Method_Call, name) {};

  // children should be
  std::shared_ptr<Expression> left;
  std::vector<std::shared_ptr<Expression>> arguments;

  std::shared_ptr<Type> resolveType(const TypeTable &typeTable, const std::shared_ptr<Scope<Entity>> &currentScope) override;
  bool validate() override;
};

class FuncCallEXP : public Expression {
public:
  FuncCallEXP(std::string method_name,
              std::vector<std::shared_ptr<Expression>> arguments)
      : Expression(E_Function_Call, std::move(method_name)),
        arguments(std::move(arguments)), isVoided(false) {};

  FuncCallEXP(std::string method_name)
      : Expression(E_Function_Call, std::move(method_name)),
        isVoided(true) {};

  FuncCallEXP() : Expression(E_Function_Call) {}

  // children should be
  std::vector<std::shared_ptr<Expression>> arguments;
  bool isVoided;

  // void addArgument(const std::shared_ptr<Expression> &arg) {
  //   this->children.push_back(arg);
  // }

  std::shared_ptr<Type> resolveType(const TypeTable &typeTable, const std::shared_ptr<Scope<Entity>> &currentScope) override;

  bool validate() override;
};

/**
 * Represents a type reference
 * (e.g., in declarations, parameters).
 *
 * var a : Integer := 2
 *         ^^^^^^^
 * method func(a : Integer, b : Integer) is ...
 *                 ^^^^^^^      ^^^^^^^
 * var array : Array[Integer]
 *             ^^^^  ^^^^^^
 * var c : MyClass
 *         ^^^^^^
 */
class ClassNameEXP : public Expression {
public:
  ClassNameEXP(std::string _name)
      : Expression(E_Class_Name, std::move(_name)) {};

  // В классе ClassNameEXP
  std::shared_ptr<Type> resolveType(const TypeTable &typeTable, const std::shared_ptr<Scope<Entity>> &currentScope) override;

  bool validate() override;
};

/**
 * A constr expression calls a creation-of-object method
 * a.k.a a constructor
 *
 * var adder : Adder := Adder(2, 2)
 *                      ^^^^^^^^^^^
 *                      search for Adder::this(Integer, Integer)
 * var adder : Adder
 *             ^^^^^
 *             search for Adder::this(), default constr
 */
class ConstructorCallEXP : public Expression {
public:
  ConstructorCallEXP(std::shared_ptr<ClassNameEXP> left,
                     std::vector<std::shared_ptr<Expression>> arguments)
      : Expression(E_Constructor_Call), left(std::move(left)),
        arguments(std::move(arguments)), isDefault(false) {};

  // Default constr
  ConstructorCallEXP(std::shared_ptr<ClassNameEXP> left)
      : Expression(E_Constructor_Call), left(std::move(left)), arguments(),
        isDefault(true) {};

  // children should be
  std::shared_ptr<ClassNameEXP> left;
  std::vector<std::shared_ptr<Expression>> arguments;
  bool isDefault;

  std::shared_ptr<Type> resolveType(const TypeTable &typeTable, const std::shared_ptr<Scope<Entity>> &currentScope) override;

  bool validate() override;
};

/**
 * Represents chained method calls
 *
 * a.Minus(2).Plus(5).Div(2)
 */
class CompoundEXP : public Expression {
public:
  CompoundEXP() : Expression(E_Chained_Functions), parts() {}

  // children should be
  std::vector<std::shared_ptr<Expression>> parts;

  // moves the pointer
  void addExpression(std::shared_ptr<Expression> expr) {
    this->parts.push_back(std::move(expr));
  }

  std::shared_ptr<Type> resolveType(const TypeTable &typeTable, const std::shared_ptr<Scope<Entity>> &currentScope) override;

  bool validate() override;
};

// @TODO
class ThisEXP : public Expression {
public:
  ThisEXP() : Expression(E_This, "this") {}

  // no children, just a link to ???
  // @TODO add link to idk what

  std::shared_ptr<Type> resolveType(const TypeTable &typeTable, const std::shared_ptr<Scope<Entity>> &currentScope) override;

  bool validate() override;
};

class ConversionEXP : public Expression {
public:
  ConversionEXP(const std::shared_ptr<Expression> &from, const std::shared_ptr<Type> &to)
      : Expression(E_Conversion, "conversion"), to(to), from(from) {};

  std::shared_ptr<Type> to;
  std::shared_ptr<Expression> from;

  std::shared_ptr<Type> resolveType(const TypeTable &typeTable, const std::shared_ptr<Scope<Entity>> &currentScope) override {
    return to;
  };
  bool validate() override { return true; };
};

// TOKEN_EQUAL,           // * ==
// TOKEN_NOT_EQUAL,       // * !=
// TOKEN_WRONG_ASSIGN,    // =
// TOKEN_MORE,            // >, illigel again
// TOKEN_LESS,            // * <
// TOKEN_MORE_EQUAL,      // * >=
// TOKEN_LESS_EQUAL,      // * <=
// TOKEN_BIT_AND,         // * &
// TOKEN_BIT_OR,          // * |
// TOKEN_BIT_XOR,         // * ^
// TOKEN_BIT_INV,         // * ~
// TOKEN_LOGIC_NOT,       // * !
// TOKEN_LOGIC_AND,       // * &&
// TOKEN_LOGIC_OR,        // * ||
// TOKEN_BIT_SHIFT_LEFT,  // * <<
// TOKEN_BIT_SHIFT_RIGHT, // * >>
// TOKEN_PLUS,            // * +
// TOKEN_MINUS,           // * -
// TOKEN_STAR,            // * *
// TOKEN_SLASH,           // * /
// TOKEN_PERCENT,         // * %

enum OperatorKind {
  OP_EQUAL,
  OP_NOT_EQUAL,
  OP_MORE,
  OP_MORE_EQUAL,
  OP_LESS,
  OP_LESS_EQUAL,
  OP_NOT,
  OP_BIT_AND,
  OP_LOGIC_AND,
  OP_BIT_OR,
  OP_LOGIC_OR,
  OP_LOGIC_NOT,
  OP_BIT_XOR,
  OP_BIT_NOT,
  OP_BIT_LSHIFT,
  OP_BIT_RSHIFT,
  OP_PLUS,
  OP_INCREMENT,
  OP_MINUS,
  OP_UNARY_MINUS,
  OP_DECREMENT,
  OP_MULTIPLY,
  OP_DIVIDE,
  OP_MODULUS,
  OP_LPAREN,
  OP_RPAREN
};

class BinaryOpEXP : public Expression {
public:
  BinaryOpEXP(OperatorKind op, std::shared_ptr<Expression> left,
              std::shared_ptr<Expression> right)
      : Expression(E_Binary_Operator), left(std::move(left)),
        right(std::move(right)), op(op) {};

  std::shared_ptr<Expression> left;
  std::shared_ptr<Expression> right;
  OperatorKind op;
};

class UnaryOpEXP : public Expression {
public:
  UnaryOpEXP(OperatorKind op, std::shared_ptr<Expression> operand)
      : Expression(E_Unary_Operator), operand(std::move(operand)), op(op) {};

  std::shared_ptr<Expression> operand;
  OperatorKind op;
};

class EnumRefEXP : public Expression {
public:
  EnumRefEXP(const std::string &enumName, const std::string &itemName)
      : Expression(E_Enum_Reference, itemName), enumName(enumName) {};

  EnumRefEXP(const std::string &enumName)
      : Expression(E_Enum_Reference), enumName(enumName) {};
  std::string enumName;
  std::string itemName;
};

#endif
