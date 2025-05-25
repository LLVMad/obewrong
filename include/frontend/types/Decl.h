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
#include "frontend/parser/Entity.h"
#include "frontend/parser/Expression.h"
#include "frontend/parser/Statement.h"

#include <filesystem>
#include <map>

class Decl : public Entity {
public:
  explicit Decl(Ekind kind, std::string name, Loc loc)
      : Entity(kind, std::move(name), loc) {}

  std::shared_ptr<Type>
  resolveType(const TypeTable &typeTable,
              const std::shared_ptr<Scope<Entity>> &currentScope) override;

  bool validate() override;

  ~Decl() override;

  DEFINE_VISITABLE()
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
  explicit FieldDecl(const std::string &name, std::shared_ptr<Type> type,
                     Loc loc)
      : Decl(E_Field_Decl, name, loc), type(std::move(type)) {}

  std::shared_ptr<Type> type;
  size_t index; // index of a field in a class
  std::shared_ptr<Type>
  resolveType(const TypeTable &typeTable,
              const std::shared_ptr<Scope<Entity>> &currentScope) override;

  bool validate() override;

  ~FieldDecl() override = default;

  DEFINE_VISITABLE()
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
  explicit VarDecl(const std::string &name, std::shared_ptr<Type> type, Loc loc)
      : Decl(E_Variable_Decl, name, loc), type(std::move(type)) {}

  std::shared_ptr<Type> type;

  // optional initializer if present
  // if a part of declaration
  //
  // var a : Integer := 0
  //                 ^^^^
  std::shared_ptr<Expression> initializer;

  std::shared_ptr<Type>
  resolveType(const TypeTable &typeTable,
              const std::shared_ptr<Scope<Entity>> &currentScope) override;

  bool validate() override;

  ~VarDecl() override = default;

  DEFINE_VISITABLE()
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
  explicit ParameterDecl(const std::string &name, std::shared_ptr<Type> type,
                         Loc loc)
      : Decl(E_Parameter_Decl, name, loc), type(std::move(type)) {}

  std::shared_ptr<Type> type;

  std::shared_ptr<Type>
  resolveType(const TypeTable &typeTable,
              const std::shared_ptr<Scope<Entity>> &currentScope) override;

  bool validate() override;

  ~ParameterDecl() override = default;

  DEFINE_VISITABLE()
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
  MethodDecl(const std::string &name, Loc loc)
      : Decl(E_Method_Decl, name, loc), isForward(false), isShort(false),
        isVoided(false), isVoid(false), isBuiltin(false), isStatic(false),
        isPrivate(false) {}
  explicit MethodDecl(const std::string &name,
                      std::shared_ptr<TypeFunc> signature,
                      std::vector<std::shared_ptr<ParameterDecl>> args,
                      std::shared_ptr<Block> body, Loc loc)
      : Decl(E_Method_Decl, name, loc), signature(std::move(signature)),
        args(std::move(args)), isForward(false), isShort(false),
        isVoided(false), isVoid(signature->isVoid), isBuiltin(false),
        isStatic(false), isPrivate(false), body(std::move(body)) {}

  explicit MethodDecl(const std::string &name,
                      std::shared_ptr<TypeFunc> signature,
                      std::shared_ptr<Block> body, Loc loc)
      : Decl(E_Method_Decl, name, loc), signature(std::move(signature)), args(),
        isForward(false), isShort(false), isVoided(true),
        isVoid(signature->isVoid), isBuiltin(false), isStatic(false),
        isPrivate(false), body(std::move(body)) {}

  explicit MethodDecl(const std::string &name,
                      const std::shared_ptr<TypeFunc> &signature,
                      const std::vector<std::shared_ptr<ParameterDecl>> &args,
                      bool isBuiltin, Loc loc)
      : Decl(E_Method_Decl, name, loc), signature(signature), args(args),
        isForward(false), isShort(false), isVoided(false),
        isVoid(signature->isVoid), isBuiltin(isBuiltin), isStatic(false),
        isPrivate(false), body() {}

  std::shared_ptr<TypeFunc> signature;
  std::vector<std::shared_ptr<ParameterDecl>> args;

  bool isForward;
  bool isShort;
  bool isVoided; // no parameters
  bool isVoid;   // no return type
  bool isBuiltin;
  bool isStatic;
  bool isPrivate;
  std::shared_ptr<Block> body;

  std::shared_ptr<Type>
  resolveType(const TypeTable &typeTable,
              const std::shared_ptr<Scope<Entity>> &currentScope) override;

  bool validate() override;

  ~MethodDecl() override = default;

  DEFINE_VISITABLE()
};

class ConstrDecl : public Decl {
public:
  ConstrDecl(const std::string &name, Loc loc)
      : Decl(E_Constructor_Decl, name, loc) {}
  explicit ConstrDecl(const std::string &name,
                      std::shared_ptr<TypeFunc> signature,
                      std::vector<std::shared_ptr<ParameterDecl>> args,
                      std::shared_ptr<Block> body, Loc loc)
      : Decl(E_Method_Decl, name, loc), signature(std::move(signature)),
        args(std::move(args)), body(std::move(body)) {}

  std::shared_ptr<TypeFunc> signature;
  std::vector<std::shared_ptr<ParameterDecl>> args;

  std::shared_ptr<Block> body;

  std::shared_ptr<Type>
  resolveType(const TypeTable &typeTable,
              const std::shared_ptr<Scope<Entity>> &currentScope) override;

  bool validate() override;

  // no parameters
  bool isDefault;

  ~ConstrDecl() override = default;

  DEFINE_VISITABLE()
};

class FuncDecl : public Decl {
public:
  explicit FuncDecl(const std::string &name,
                    std::shared_ptr<TypeFunc> signature,
                    std::vector<std::shared_ptr<ParameterDecl>> args,
                    std::shared_ptr<Block> body, Loc loc)
      : Decl(E_Function_Decl, name, loc), signature(std::move(signature)),
        args(std::move(args)), isVoided(false), isVoid(signature->isVoid),
        body(std::move(body)) {}

  FuncDecl(const std::string &name, Loc loc)
      : Decl(E_Function_Decl, name, loc), signature(), body() {}

  // FuncDecl(const std::string &name, bool isMain)
  //     : Decl(isMain ? E_Function_Decl, name), signature(), body() {}

  std::shared_ptr<TypeFunc> signature;
  std::vector<std::shared_ptr<ParameterDecl>> args;

  bool isForward;
  bool isShort;
  bool isVoided; // no parameters
  bool isVoid;
  std::shared_ptr<Block> body;

  std::shared_ptr<Type>
  resolveType(const TypeTable &typeTable,
              const std::shared_ptr<Scope<Entity>> &currentScope) override;

  bool validate() override;

  ~FuncDecl() override = default;

  DEFINE_VISITABLE()
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
  // ClassDecl(const std::string &name, std::shared_ptr<TypeClass> type,
  //           std::vector<std::shared_ptr<ClassDecl>> base_class,
  //           std::vector<std::shared_ptr<FieldDecl>> fields,
  //           std::vector<std::shared_ptr<MethodDecl>> methods,
  //           std::vector<std::shared_ptr<ConstrDecl>> constructors)
  //     : Decl(E_Class_Decl, name), type(std::move(type)),
  //       base_classes(std::move(base_class)), fields(std::move(fields)),
  //       methods(std::move(methods)), constructors(std::move(constructors)) {}

  ClassDecl(const std::string &name, std::shared_ptr<TypeClass> type,
            std::vector<std::shared_ptr<FieldDecl>> fields,
            std::vector<std::shared_ptr<Decl>> methods, Loc loc)
      : Decl(E_Class_Decl, name, loc), type(std::move(type)),
        fields(std::move(fields)), methods(std::move(methods)) {}

  std::shared_ptr<TypeClass> type;
  std::shared_ptr<ClassDecl> base_class;
  std::vector<std::shared_ptr<FieldDecl>> fields;

  // @NOTE: becouse methods and constructs can appear in different
  // order we should keep the order of their declaration !
  // std::vector<std::shared_ptr<MethodDecl>> methods;
  // std::vector<std::shared_ptr<ConstrDecl>> constructors;
  std::vector<std::shared_ptr<Decl>> methods; // btoh methods and constrs

  std::shared_ptr<Type>
  resolveType(const TypeTable &typeTable,
              const std::shared_ptr<Scope<Entity>> &currentScope) override;

  bool validate() override;

  ~ClassDecl() override = default;

  DEFINE_VISITABLE()
};

// @TODO maybe get rid of Array and List decls
// -> VarDecl can be an array decl, so...
class ArrayDecl : public Decl {
public:
  ArrayDecl(const std::string &name, std::shared_ptr<TypeArray> type,
            std::shared_ptr<ArrayLiteralExpr> initz, Loc loc)
      : Decl(E_Array_Decl, name, loc), type(std::move(type)),
        initializer(std::move(initz)) {}

  std::shared_ptr<TypeArray> type;

  // optional initializer if present
  // if a part of declaration
  //
  // var a : Array[Integer] := [1, 2, 3]
  //                           ^^^^^^^^^
  std::shared_ptr<ArrayLiteralExpr> initializer;

  size_t size;

  std::shared_ptr<Type>
  resolveType(const TypeTable &typeTable,
              const std::shared_ptr<Scope<Entity>> &currentScope) override;

  bool validate() override;

  ~ArrayDecl() override = default;

  DEFINE_VISITABLE()
};

class ListDecl : public Decl {
public:
  ListDecl(const std::string &name, std::shared_ptr<TypeList> type,
           std::shared_ptr<ArrayLiteralExpr> initz, Loc loc)
      : Decl(E_List_Decl, name, loc), type(std::move(type)),
        initializer(std::move(initz)) {}

  std::shared_ptr<TypeList> type;

  // optional initializer if present
  // if a part of declaration
  //
  // var a : List[Integer] := [1, 2, 3]
  //                          ^^^^^^^^^
  std::shared_ptr<ArrayLiteralExpr> initializer;

  std::shared_ptr<Type>
  resolveType(const TypeTable &typeTable,
              const std::shared_ptr<Scope<Entity>> &currentScope) override;

  bool validate() override;

  ~ListDecl() override = default;

  DEFINE_VISITABLE()
};

class ModuleDecl : public Decl {
public:
  ModuleDecl(const std::string &moduleNmae, Loc loc)
      : Decl(E_Module_Decl, moduleNmae, loc) {}

  void addImport(const std::string &importName) {
    importedModules.push_back(importName);
  }

  std::vector<std::string> importedModules;
  std::vector<std::shared_ptr<Entity>> children;

  DEFINE_VISITABLE()
};

class EnumDecl : public Decl {
public:
  EnumDecl(const std::string &enumName, Loc loc)
      : Decl(E_Enum_Decl, enumName, loc), size(0) {}

  void addItem(const std::string &name) { items[name] = size++; };

  size_t size;
  std::map<std::string, uint32_t> items;

  DEFINE_VISITABLE()
};

#endif
