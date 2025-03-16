#ifndef OBW_EINFO_H
#define OBW_EINFO_H

#include <memory>
#include <ostream>

#include "frontend/types/Types.h"

struct Loc {
  size_t line;
  size_t col;
  std::string file;
};

/**
* Describes different kind of
* entitys available in
* Obewrong language
*/
enum Ekind {
  // Declarations
  E_Class_Decl,
  E_Constructor_Decl,
  E_Variable_Decl,
  E_Function_Decl,
  E_Array_Decl,
  E_List_Decl,

  // Type-related entities
  E_Integer_Literal,
  E_Integer_Type,
  E_Real_Literal,
  E_Real_Type,
  E_String_Literal,
  E_String_Type,
  E_Boolean_Literal,
  E_Boolean_Type,
  E_Function,
  E_Function_Type,
  E_Class_Name,

  // Special entities
  E_This,
  E_Library_Class,
  E_Generic_Class,

  // Statement-related entities
  E_Assignment,
  E_While_Loop,
  E_If_Statement,
  E_Return_Statement,
};

/**
* Foundation for obewrong entities
*/
class Entity {
public:
  virtual ~Entity() = default;
  explicit Entity(Ekind kind);

  Ekind getKind() const;
  Loc getLoc() const;

  virtual std::unique_ptr<Type> resolveType();
  virtual bool validate();
  // return scope parent
  //  std::unique_ptr<Entity> getScopePar() const;

protected:
  Ekind kind;

  Loc location;

  // Point to entity in whose scope the current entity is
  // i.e. if Ekind == E_Method, then it points to
  // an E_Class, in which this method is declared
  //  std::unique_ptr<Entity> scope;
};

/**
* @TODO construct classes for each entitys
* i.e. `class ClassDeclaration : Entity`
* this will make it possible to return `Entity*` from some
* `parse*` functions like for example from parseParameters()
*/



#endif
