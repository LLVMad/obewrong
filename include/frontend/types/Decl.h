#ifndef OBW_DECL_H
#define OBW_DECL_H

/*
 * Named entities
 */

#include <string>
#include <utility>
#include <vector>
#include <memory>

#include "Types.h"
#include "frontend/parser/Expression.h"

// enum DeclKind {
//   DECL_VAR,
//   DECL_FUNC,
//   DECL_CLASS,
//   DECL_ARRAY,
//   DECL_LIST,
// };

class Decl : public Entity {
public:
  explicit Decl(Ekind kind, std::string name) : Entity(kind), name(std::move(name)) {}
  std::string name;
  // points to a declaration in which scope this decl is
  // i.e. some ClassDecl or FuncDecl
  std::unique_ptr<Decl> scope;
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
  explicit FieldDecl(Ekind kind, const std::string &name) : Decl(kind, name) {}

  std::unique_ptr<Type> type;
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
  explicit VarDecl(
    Ekind kind,
    const std::string &name,
    std::unique_ptr<Type> type,
    std::unique_ptr<Expression> init)
    : Decl(kind, name), type(std::move(type)), initializer(std::move(init)) {}

  std::unique_ptr<Type> type;

  // optional initializer if present
  // if a part of declaration
  //
  // var a : Integer := 0
  //                 ^^^^
  std::unique_ptr<Expression> initializer;
};

/**
* Declaration of a method in a class body (block)
*
* BEWARE: this can be:
*   - forward declaration: `method short() : Integer` (no body)
*   - short syntax decl  : `method get() => return this.a`
*   - full               : `method set(a: Integer) is ... end`
*/
class FuncDecl : public Decl {
public:
  explicit FuncDecl(
    Ekind kind, const std::string &name,
    std::unique_ptr<TypeFunc> signature,
    std::vector<std::unique_ptr<Decl>> args,
    std::vector<std::unique_ptr<Expression>> body)
      : Decl(kind, name), signature(std::move(signature)), args(std::move(args)), body(std::move(body)) {}

  std::unique_ptr<TypeFunc> signature;
  std::vector<std::unique_ptr<Decl>> args;

  bool isForward;
  bool isShort;
  std::vector<std::unique_ptr<Expression>> body;
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
  //std::string base_class;
  //std::vector<std::string> generic_params;
  std::vector<std::unique_ptr<ClassDecl>> base_classes;
  std::vector<std::unique_ptr<FieldDecl>> fields;
  std::vector<std::unique_ptr<FuncDecl>> methods;
  std::vector<std::unique_ptr<FuncDecl>> constructors;
};

class ArrayDecl : public Decl {
public:
  std::unique_ptr<TypeArray> type;

  // optional initializer if present
  // if a part of declaration
  //
  // var a : Array[Integer] := [1, 2, 3]
  //                           ^^^^^^^^^
  std::unique_ptr<Expression> initializer;
};

class ListDecl : public Decl {
public:
  std::unique_ptr<TypeList> type;

  // optional initializer if present
  // if a part of declaration
  //
  // var a : List[Integer] := [1, 2, 3]
  //                          ^^^^^^^^^
  std::unique_ptr<Expression> initializer;
};

#endif
