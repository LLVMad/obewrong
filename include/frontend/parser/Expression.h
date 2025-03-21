#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <utility>

#include "Entity.h"
#include "util/Logger.h"

#define DEBUG 1 

/**
* Base expression entity
*/
class Expression : public Entity {
public:
  explicit Expression(Ekind kind) : Entity(kind) {};
  std::unique_ptr<Type> resolveType() const override;
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

  std::unique_ptr<Type> resolveType() const override {
    return new TypeBuiltin(TYPE_INT, "Integer", 32);
  }

  bool validate() override {
    // Для литерала проверка не требуется, так как тип известен
    LOG_NOARGS("Validate IntLiteralEXP is OK");
    return true;
  }

private:
  int _value;
};

class RealLiteralEXP : public Expression {
public:
  RealLiteralEXP(double val) : Expression(E_Real_Literal), _value(val) {};

  double getValue() { return _value; }

  std::unique_ptr<Type> resolveType() const override {
    return new TypeBuiltin(TYPE_FLOAT, "Real", 32);;
  }
private:
  double _value;
};

class StringLiteralEXP : public Expression {
public:
  StringLiteralEXP(std::string val) : Expression(E_String_Literal), value(std::move(val)) {};

  std::string value;

  std::unique_ptr<Type> resolveType() const override {
    return new TypeString(sizeof(value.c_str()));
  }
};

class BoolLiteralEXP : public Expression {
public:
  BoolLiteralEXP(bool val) : Expression(E_Boolean_Literal), _value(val) {};

  bool getValue() { return _value; }

  std::unique_ptr<Type> resolveType() const override {
    return new TypeBuiltin(TYPE_BOOL, "Bool", 1);;
  }
private:
  bool _value;
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
  VarRefEXP(std::string name) : Expression(E_Var_Reference), var_name(std::move(name)) {};

  std::string var_name;

  std::unique_ptr<Type> resolveType() const override {
    // Ищем переменную в таблице символов
    const auto& type = resolveSymbol(var_name);
    if (!type) {
      throw std::runtime_error("Undefined variable: " + var_name);
    }
    return type;
  }

  bool validate() override {
    LOG_NOARGS("Try to validate VarRefEXP...");
    // Проверяем, что переменная объявлена
    auto type = resolveType();
    if (type != nullptr) {
      LOG_NOARGS("Validate VarRefEXP is OK");
    }
    else {
      LOG_ERR_NOARGS("Can't validate VarRefEXP");
    }
    return type != nullptr;
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
  MethodCallEXP() : Expression(E_Function) {}

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

  std::unique_ptr<Type> resolveType() const override {
    // @TODO: Реализовать разрешение типа для вызова метода
    return nullptr;
  }

  bool validate() override {
    // Проверяем, что левая часть выражения является объектом или классом
    if (!left->validate()) {
      LOG_ERR_NOARGS("Can't validate left part of MethodCallEXP");
      return false;
    }

    // Проверяем, что метод существует
    auto leftType = left->resolveType();
    if (!leftType) {
      throw std::runtime_error("Left side of method call has no type");
    }

    // @TODO: Проверить, что метод method_name существует для типа leftType

    // Проверяем аргументы
    for (auto& arg : arguments) {
      if (!arg->validate()) {
        LOG_ERR_NOARGS("Can't validate arguments for MethodCallEXP");
        return false;
      }
    }

    return true;
  }
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
  ClassNameEXP(std::string name) : Expression(E_Class_Name), _name(std::move(name)) {};
        const std::string& getName() const {
            return _name;
        }
    
  std::unique_ptr<Type> resolveType() const override {
    if (_name == "Integer") {
      return new TypeInt();
    } else if (_name == "Real") {
      return new TypeReal();
    } else if (_name == "Bool") {
      return new TypeBool();
    } else if (_name == "String") {
      return new TypeString();
    } else if (_name == "Array") {
      return new TypeArray();
    } else if (_name == "List") {
      return new TypeList();
    } else {
      // @TODO: Если это пользовательский класс, нужно искать его в таблице символов
      return new Type(TYPE_UNKNOWN, _name);
    }
  }

  bool validate() override {
    // Проверяем, что имя класса корректно
    if (_name.empty()) {
      LOG_ERR_NOARGS("Can't validate ClassNameEXP"); 
      return false;
    }
    // @TODO: Добавить проверку, что класс существует в таблице символов
    LOG_NOARGS("Validate ClassNameEXP is OK");
    return true;
  }

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
  ConstructorCallEXP(
    std::unique_ptr<ClassNameEXP> left,
    std::vector<std::unique_ptr<Expression>> args
  ) : Expression(E_Function),
      left(std::move(left)),
      arguments(std::move(args)) {}

  std::unique_ptr<ClassNameEXP> left;
  std::vector<std::unique_ptr<Expression>> arguments;

  std::unique_ptr<Type> resolveType() const override {
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

  bool validate() override {
    LOG_NOARGS("Validate ConstructorCallEXP is OK");
    return true;
  }
};

/**
* Evaluates to a class field
*
* Adder.x, MyPair.first, LuxMeter.coeffA
*/
class FieldAccessEXP : public Expression {
public:
  FieldAccessEXP() : Expression(E_Field_Reference) {}

  FieldAccessEXP(std::unique_ptr<Expression> left, std::string name)
    : Expression(E_Field_Reference), left(std::move(left)), field_name(std::move(name)) {};

  std::unique_ptr<Expression> left;
  std::string field_name;

  std::unique_ptr<Type> resolveType() const override {
    /*
     * @TODO
     * 1) search for left side in a class declarations
     * 2) get type of a field by name
     * 3) return its type
     */
    return nullptr;
  }

  bool validate() override {
    LOG_NOARGS("Validate FieldAccessEXP is OK");
    return true;
  }
};

/**
* Represents chained method calls
*
* a.Minus(2).Plus(5).Div(2)
*/
class CompoundEXP : public Expression {
public:
  CompoundEXP(std::vector<std::unique_ptr<Expression>> parts)
    : Expression(E_Chained_Functions), parts(std::move(parts)) {}

  std::vector<std::unique_ptr<Expression>> parts;

  std::unique_ptr<Type> resolveType() const override {
    // Если parts пуст, возвращаем nullptr или тип по умолчанию
    if (parts.empty()) {
      return new Type(TYPE_UNKNOWN, "Unknown");
    }

    // Возвращаем тип последнего выражения в цепочке
    return parts.back()->resolveType();
  }

  bool validate() override {
    // Проверяем все выражения в цепочке
    for (const auto& part : parts) {
      if (!part->validate()) {
        LOG_ERR_NOARGS("Can't validate CompoundEXP"); 
        return false;
      }
    }
    LOG_NOARGS("Validate CompoundEXP is OK");
    return true;
  }
};

class ThisEXP : public Expression {
public:
  ThisEXP() : Expression(E_This) {}

  std::unique_ptr<Type> resolveType() const override {
    // Возвращаем тип текущего класса из scope
    if (scope) {
      return scope->resolveType();
    }
    // Если scope не установлен, возвращаем nullptr или тип по умолчанию
    return new Type(TYPE_UNKNOWN, "Unknown");
  }

  bool validate() override {
    // Проверяем, что `this` используется внутри метода/конструктора
    if (scope == nullptr) {
      LOG_ERR_NOARGS("Can't validate ThisEXP"); 
    }
    else {
      LOG_NOARGS("Validate ThisEXP is OK");
    }
    return scope != nullptr;
  }
};

#endif
