#ifndef OBW_EINFO_H
#define OBW_EINFO_H

#include "TypeTable.h"

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
  E_Parameter_Decl,
  E_Field_Decl,
  E_Function_Decl,
  E_Method_Decl,
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
  E_Method,
  E_Chained_Functions, // compound expressions, a.Plus(2).Minus(1)...
  E_Function_Type,
  E_Class_Name,

  // Special entities
  E_This, // current instance of object
  E_Library_Class,
  E_Generic_Class,

  // Statement-related entities
  E_Assignment,
  E_While_Loop,
  E_If_Statement,
  E_Switch_Statement,
  E_Case_Statement,
  E_Return_Statement,
  E_Block,

  E_Var_Reference,
  E_Field_Reference,

  // fake entities, that just used to link meaningfully ones
  E_Expression,
  E_Arguments, // arguments list that is passed to a function

  E_Function_Call,
  E_Method_Call,
  E_Array_Literal,
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

  virtual std::shared_ptr<Type> resolveType(TypeTable typeTable);
  virtual bool validate();

  // ======== NODE LINKS ========
  // different types of connection
  // between nodes in a AAST
  // type, attributes are defined in children classes

  // SCOPE LINK
  // points to a declaration in which scope this decl is
  // i.e. some ClassDecl or FuncDecl
  std::shared_ptr<Entity> scope;

  // STRUCTURAL LINK
  // children nodes
  std::vector<std::unique_ptr<Entity>> children;

protected:
  Ekind kind;

  Loc location;
};

class Block : public Entity {
public:
  Block(std::vector<std::unique_ptr<Entity>> parts)
    : Entity(E_Block), parts(std::move(parts)) {}

  std::vector<std::unique_ptr<Entity>> parts;
};

/**
 * @TODO construct classes for each entitys
 * i.e. `class ClassDeclaration : Entity`
 * this will make it possible to return `Entity*` from some
 * `parse*` functions like for example from parseParameters()
 */

#endif
