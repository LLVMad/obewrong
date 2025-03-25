#ifndef OBW_DECL_H
#define OBW_DECL_H

/*
 * Named entities
 */

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "Types.h"
#include "frontend/parser/Expression.h"
#include "frontend/parser/Statement.h"

// enum DeclKind {
//   DECL_VAR,
//   DECL_FUNC,
//   DECL_CLASS,
//   DECL_ARRAY,
//   DECL_LIST,
// };

class Decl : public Entity {
public:
  explicit Decl(Ekind kind, std::string name)
      : Entity(kind), name(std::move(name)) {}
  std::string name;

  std::shared_ptr<Type> resolveType(TypeTable typeTable) override;

  bool validate() override;

  ~Decl() override;
};

/**
 * Declaration of a class field
 * looks like a variable declaration,
 * except is **cant have initializers**
 *
 * <ClassDeclaration>
 *   var a : Integer
 *   var coeff : Real
 *   ...
 * end
 */
class FieldDecl : public Decl {
public:
  explicit FieldDecl(const std::string &name, std::shared_ptr<Type> type)
    : Decl(E_Field_Decl, name), type(std::move(type)) {}

  std::shared_ptr<Type> type;

  std::shared_ptr<Type> resolveType(TypeTable typeTable) override;

  bool validate() override;
};

/**
 * Declaration of a variable
 * **can have initializers**
 *
 * <ClassDeclaration>
 *   var a : Integer
 *   var coeff : Real
 *   ...
 * end
 */
class VarDecl : public Decl {
public:
  explicit VarDecl(const std::string &name, std::shared_ptr<Type> type)
      : Decl(E_Variable_Decl, name), type(std::move(type)) {}

  std::shared_ptr<Type> type;

  // optional initializer if present
  // if a part of declaration
  //
  // var a : Integer := 0
  //                 ^^^^
  std::unique_ptr<Expression> initializer;

  std::shared_ptr<Type> resolveType(TypeTable typeTable) override;

  bool validate() override;
};

/**
 * Declaration of an argument
 * **cant have initializers**
 *
 * method func(a : Integer, b : Integer) is ...
 *             ^^^^^^^^^^   ^^^^^^^^^^^
 */
class ParameterDecl : public Decl {
public:
  explicit ParameterDecl(const std::string &name, std::shared_ptr<Type> type)
      : Decl(E_Parameter_Decl, name), type(type) {}

  std::shared_ptr<Type> type;

  std::shared_ptr<Type> resolveType(TypeTable typeTable) override;

  bool validate() override;
};

/**
 * Declaration of a method in a class body (block)
 *
 * BEWARE: this can be:
 *   - forward declaration: `method short() : Integer` (no body)
 *   - short syntax decl  : `method get() => return this.a`
 *   - full               : `method set(a: Integer) is ... end`
 */
class MethodDecl : public Decl {
public:
  explicit MethodDecl(const std::string &name,
                    std::shared_ptr<TypeFunc> signature,
                    std::vector<std::unique_ptr<Decl>> args,
                    std::unique_ptr<Block> body)
      : Decl(E_Method_Decl, name), signature(signature),
        args(std::move(args)), body(std::move(body)) {}

  std::shared_ptr<TypeFunc> signature;
  std::vector<std::unique_ptr<Decl>> args;

  bool isForward;
  bool isShort;
  std::unique_ptr<Block> body;

  std::shared_ptr<Type> resolveType(TypeTable typeTable) override;

  bool validate() override;
};

class FuncDecl : public Decl {
public:
  explicit FuncDecl(const std::string &name,
                    std::shared_ptr<TypeFunc> signature,
                    std::vector<std::unique_ptr<ParameterDecl>> args,
                    std::unique_ptr<Block> body)
      : Decl(E_Function_Decl, name), signature(signature),
        args(std::move(args)), body(std::move(body)) {}

  std::shared_ptr<TypeFunc> signature;
  std::vector<std::unique_ptr<ParameterDecl>> args;

  bool isForward;
  bool isShort;
  std::unique_ptr<Block> body;

  std::shared_ptr<Type> resolveType(TypeTable typeTable) override;

  bool validate() override;
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
class ClassDecl : public Decl {
public:
  ClassDecl(
    const std::string &name, std::shared_ptr<TypeClass> type,
    std::vector<std::shared_ptr<ClassDecl>> base_class,
    std::vector<std::unique_ptr<FieldDecl>> fields,
    std::vector<std::unique_ptr<FuncDecl>> methods,
    std::vector<std::unique_ptr<FuncDecl>> constructors
  ) : Decl(E_Class_Decl, name), type(type), base_classes(std::move(base_class)),
      fields(std::move(fields)), methods(std::move(methods)), constructors(std::move(constructors)) {}

  ClassDecl(
    const std::string &name, std::shared_ptr<TypeClass> type,
    std::vector<std::unique_ptr<FieldDecl>> fields,
    std::vector<std::unique_ptr<FuncDecl>> methods,
    std::vector<std::unique_ptr<FuncDecl>> constructors
  ) : Decl(E_Class_Decl, name), type(type),
  fields(std::move(fields)), methods(std::move(methods)), constructors(std::move(constructors)) {}

  std::shared_ptr<TypeClass> type;
  std::vector<std::shared_ptr<ClassDecl>> base_classes;
  std::vector<std::unique_ptr<FieldDecl>> fields;
  std::vector<std::unique_ptr<FuncDecl>> methods;
  std::vector<std::unique_ptr<FuncDecl>> constructors;

  std::shared_ptr<Type> resolveType(TypeTable typeTable) override;

  bool validate() override;
};

class ArrayDecl : public Decl {
public:
  ArrayDecl(const std::string &name, std::shared_ptr<TypeArray> type, std::unique_ptr<ArrayLiteralExpr> initz)
    : Decl(E_Array_Decl, name), type(type), initializer(std::move(initz)) {}

  std::shared_ptr<TypeArray> type;

  // optional initializer if present
  // if a part of declaration
  //
  // var a : Array[Integer] := [1, 2, 3]
  //                           ^^^^^^^^^
  std::unique_ptr<ArrayLiteralExpr> initializer;

  std::shared_ptr<Type> resolveType(TypeTable typeTable) override;

  bool validate() override;
};

class ListDecl : public Decl {
public:
  ListDecl(const std::string &name, std::shared_ptr<TypeList> type, std::unique_ptr<ArrayLiteralExpr> initz)
    : Decl(E_List_Decl, name), type(std::move(type)), initializer(std::move(initz)) {}

  std::shared_ptr<TypeList> type;

  // optional initializer if present
  // if a part of declaration
  //
  // var a : List[Integer] := [1, 2, 3]
  //                          ^^^^^^^^^
  std::unique_ptr<ArrayLiteralExpr> initializer;

  std::shared_ptr<Type> resolveType(TypeTable typeTable) override;

  bool validate() override;
};

#endif
