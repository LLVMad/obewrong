#ifndef OBW_EINFO_H
#define OBW_EINFO_H

#include <ostream>
#include <memory>

struct Loc {
  size_t line;
  size_t col;
};

/**
* Describes different kind of
* entitys available in
* Obewrong language
*/
enum Ekind {
  // Class-related entities
  E_Class,
  E_Constructor,
  E_Method,
  E_Variable,

  // Type-related entities
  E_Integer_Type,
  E_Real_Type,
  E_Boolean_Type,
  E_Array_Type,
  E_List_Type,

  // Special entities
  E_This,
  E_Library_Class,
  E_Generic_Class,

  // Statement-related entities
  E_Assignment,
  E_While_Loop,
  E_If_Statement,
  E_Return_Statement
};

/**
* Foundation for obewrong entities
*/
class Entity {
public:
  Ekind getKind() const;

  Loc getLoc() const;

  // return scope parent
  std::unique_ptr<Entity> getScopePar() const;

protected:
  Ekind kind;

  Loc location;

  // Point to entity in whose scope the current entity is
  // i.e. if Ekind == E_Method, then it points to
  // an E_Class, in which this method is declared
  std::unique_ptr<Entity> scope;
};

/**
* @TODO construct classes for each entitys
* i.e. `class ClassDeclaration : Entity`
* this will make it possible to return `Entity*` from some
* `parse*` functions like for example from parseParameters()
*/



#endif
