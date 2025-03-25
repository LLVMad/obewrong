#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <utility>

#include "Entity.h"

/**
 * Base expression entity
 */
class Expression : public Entity {
public:
  explicit Expression(Ekind kind) : Entity(kind){};
  std::shared_ptr<Type> resolveType(TypeTable typeTable) override;
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
  IntLiteralEXP(int val) : Expression(E_Integer_Literal), _value(val){};

  int getValue() { return _value; }

  std::shared_ptr<Type> resolveType(TypeTable typeTable) override {
    // auto type = std::make_unique<TypeBuiltin>(TYPE_INT, "Integer", 32);
    // return type;
    return typeTable.getType("Integer");
  }

private:
  int _value;
};

class RealLiteralEXP : public Expression {
public:
  RealLiteralEXP(double val) : Expression(E_Real_Literal), _value(val){};

  double getValue() { return _value; }

  std::shared_ptr<Type> resolveType(TypeTable typeTable) override {
    return typeTable.getType("Real");
  }

private:
  double _value;
};

class StringLiteralEXP : public Expression {
public:
  StringLiteralEXP(std::string val)
      : Expression(E_String_Literal), value(std::move(val)){};

  std::string value;

  std::shared_ptr<Type> resolveType(TypeTable typeTable) override {
    // auto type = std::make_unique<TypeString>(sizeof(value.c_str()));
    // return type;
    return typeTable.getType("String");
  }
};

class BoolLiteralEXP : public Expression {
public:
  BoolLiteralEXP(bool val) : Expression(E_Boolean_Literal), _value(val){};

  bool getValue() { return _value; }

  std::shared_ptr<Type> resolveType(TypeTable typeTable) override {
    // auto type = std::make_unique<TypeBuiltin>(TYPE_BOOL, "Bool", 1);
    // return type;
    return typeTable.getType("Bool");
  }

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
  ArrayLiteralExpr(std::vector<std::unique_ptr<Expression>>)
    : Expression(E_Array_Literal), elements(std::move(elements)) {};

  // children should be
  std::vector<std::unique_ptr<Expression>> elements;

  // void addElement(std::shared_ptr<Expression> expr) {
  //
  // }

  std::shared_ptr<Type> resolveType(TypeTable typeTable) override {
    (void)typeTable;
    /*
     * @TODO
     * search for a FieldDecl IN class
     *
     */
    return nullptr;
  }

  bool validate() override {
    // Ensure the class exists and arguments match the constructor.
    return true;
  }
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
 * var c : Integer := a.Plus(2)
 *                    ^
 */
class FieldRefEXP : public Expression {
public:
  FieldRefEXP(std::string name)
      : Expression(E_Field_Reference), field_name(std::move(name)){};

  std::string field_name;

  std::shared_ptr<Type> resolveType(TypeTable typeTable) override {
    (void)typeTable;
    /*
     * @TODO
     * search for a FieldDecl IN class
     *
     */
    return nullptr;
  }

  bool validate() override {
    // Ensure the class exists and arguments match the constructor.
    return true;
  }
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
      : Expression(E_Var_Reference), var_name(name) {};

  std::string var_name;

  // no children, link to a VarDecl probably

  std::shared_ptr<Type> resolveType(TypeTable typeTable) override {
    (void)typeTable;
    /*
     * @TODO
     * search for a VarDecl IN SCOPE
     *
     */
    return nullptr;
  }

  bool validate() override {
    // Ensure the class exists and arguments match the constructor.
    return true;
  }
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
  MethodCallEXP(std::string method_name, std::unique_ptr<Expression> left, std::vector<std::unique_ptr<Expression>> arguments)
      : Expression(E_Method_Call), method_name(std::move(method_name)), left(std::move(left)), arguments(std::move(arguments)) {};

  MethodCallEXP() : Expression(E_Method_Call) {}

  std::string method_name;

  // children should be
  std::unique_ptr<Expression> left;
  std::vector<std::unique_ptr<Expression>> arguments;

  // void addLhs(const std::shared_ptr<Expression> &lhs) {
  //   this->children.push_back(lhs);
  // }
  //
  // void addArgument(const std::shared_ptr<Expression> &arg) {
  //   this->children.push_back(arg);
  // }

  std::shared_ptr<Type> resolveType(TypeTable typeTable) override {
    (void)typeTable;
    // @TODO
    return nullptr;
  }

  bool validate() override { return true; }
};

class FuncCallEXP : public Expression {
public:
  FuncCallEXP(std::string method_name, std::vector<std::unique_ptr<Expression>> arguments)
      : Expression(E_Function_Call),
        func_name(std::move(method_name)), arguments(std::move(arguments)) {};

  FuncCallEXP() : Expression(E_Function_Call) {}

  std::string func_name;

  // children should be
  std::vector<std::unique_ptr<Expression>> arguments;

  // void addArgument(const std::shared_ptr<Expression> &arg) {
  //   this->children.push_back(arg);
  // }

  std::shared_ptr<Type> resolveType(TypeTable typeTable) override {
    (void)typeTable;
    // @TODO
    return nullptr;
  }

  bool validate() override { return true; }
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
  ClassNameEXP(std::string name)
      : Expression(E_Class_Name), _name(std::move(name)){};

  // В классе ClassNameEXP
  std::shared_ptr<Type> resolveType(TypeTable typeTable) override {
    (void)typeTable;
    // if (_name == "Integer") {
    //   return
    // } else if (_name == "Real") {
    //   return std::make_unique<TypeReal>();
    // } else if (_name == "Bool") {
    //   return std::make_unique<TypeBool>();
    // } else if (_name == "String") {
    //   return std::make_unique<TypeString>();
    // } else if (_name == "Array") {
    //   return std::make_unique<TypeArray>();
    // } else if (_name == "List") {
    //   return std::make_unique<TypeList>();
    // } else {
    //   return std::make_unique<Type>(TYPE_UNKNOWN, _name);
    // }
    // @TODO not found
    return typeTable.getType(_name);
  }

  bool validate() override { return true; }

private:
  std::string _name;
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
  ConstructorCallEXP(std::unique_ptr<ClassNameEXP> left, std::vector<std::unique_ptr<Expression>> arguments)
      : Expression(E_Function), left(std::move(left)), arguments(std::move(arguments)) {};

  // children should be
  std::unique_ptr<ClassNameEXP> left;
  std::vector<std::unique_ptr<Expression>> arguments;

  // void addLhs(const std::shared_ptr<ClassNameEXP> &lhs) {
  //   this->children.push_back(lhs);
  // }
  //
  // void addArgument(const std::shared_ptr<Entity> &arg) {
  //   this->children.push_back(arg);
  // }

  std::shared_ptr<Type> resolveType(TypeTable typeTable) override {
    (void)typeTable;
    /*
     * @TODO
     * this is interesting
     * because as far as i understand
     * the return type of this
     * should be a pointer to a class
     * but we dont have pointers...
     */
    return nullptr;
  }

  bool validate() override { return true; }
};

/**
 * Evaluates to a class field
 *
 * Adder.x, MyPair.first, LuxMeter.coeffA
 */
class FieldAccessEXP : public Expression {
public:
  FieldAccessEXP(std::string name, std::unique_ptr<Expression> left)
      : Expression(E_Field_Reference), left(std::move(left)), field_name(std::move(name)) {};

  // children should be
  std::unique_ptr<Expression> left;
  std::string field_name;

  // void addLhs(const std::shared_ptr<Expression> &lhs) {
  //   this->children.push_back(lhs);
  // }

  std::shared_ptr<Type> resolveType(TypeTable typeTable) override {
    (void)typeTable;
    /*
     * @TODO
     * 1) search for left side in a class declarations
     * 2) get type of a field by name
     * 3) return its type
     */
    return nullptr;
  }

  bool validate() override { return true; }
};

/**
 * Represents chained method calls
 *
 * a.Minus(2).Plus(5).Div(2)
 */
class CompoundEXP : public Expression {
public:
  CompoundEXP()
      : Expression(E_Chained_Functions) {}

  // children should be
  std::vector<std::unique_ptr<Expression>> parts;

  // moves the pointer
  void addExpression(std::unique_ptr<Expression> expr) {
    this->parts.push_back(std::move(expr));
  }

  std::shared_ptr<Type> resolveType(TypeTable typeTable) override {
    (void)typeTable;
    /*
     * @TODO
     * call resolveType on each
     * exp in parts
     */
    // Временная заглушка
    // return std::make_unique<Type>(TYPE_UNKNOWN, "Unresolved compound type");
    return nullptr;
  }

  bool validate() override { return true; }
};

// @TODO
class ThisEXP : public Expression {
public:
  ThisEXP() : Expression(E_This) {}

  // no children, just a link to ???

  std::shared_ptr<Type> resolveType(TypeTable typeTable) override {
    (void)typeTable;
    // return the type of the enclosing class (from the scope).
    // Временная заглушка
    // return std::make_unique<Type>(TYPE_UNKNOWN, "Unresolved 'this' type");
    return nullptr;
  }

  bool validate() override {
    // ensure `this` is used within a method/constructor.
    // Временная заглушка
    return true;
  }
};

#endif
