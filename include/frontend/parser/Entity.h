#ifndef OBW_EINFO_H
#define OBW_EINFO_H

#include "../TypeTable.h"
#include "frontend/lexer/Lexer.h"

#include <memory>
#include <ostream>

#include "frontend/types/Types.h"

#include <functional>

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
  E_Module_Decl,
  E_Enum_Decl,
  E_Main_Decl,

  // Type related entities
  E_Integer_Type,
  E_Real_Type,
  E_String_Type,
  E_Boolean_Type,
  E_Function_Type,

  // Expression related
  E_Integer_Literal,
  E_Real_Literal,
  E_String_Literal,
  E_Array_Literal,
  E_Var_Reference,
  E_Field_Reference,
  E_Boolean_Literal,
  E_Function,
  E_Constructor_Call,
  E_Method,
  E_Chained_Functions, // compound expressions, a.Plus(2).Minus(1)...
  E_Class_Name,
  E_Dummy,
  E_Function_Call,
  E_Method_Call,
  E_Binary_Operator,
  E_Unary_Operator,
  E_Enum_Reference,
  E_Element_Reference,

  // Special entities
  E_This, // current instance of object
  // E_Library_Class,
  // E_Generic_Class,

  // Statement-related entities
  E_Assignment,
  E_While_Loop,
  E_For_Loop,
  E_If_Statement,
  E_Switch_Statement,
  E_Case_Statement,
  E_Return_Statement,
  E_Block,
};

/**
 * Foundation for obewrong entities
 */
class Entity {
public:
  virtual ~Entity() = default;
  explicit Entity(Ekind kind) : location() { this->kind = kind; };

  Ekind getKind() const { return kind; };
  Loc getLoc() const { return location; };

  virtual std::shared_ptr<Type> resolveType(TypeTable typeTable) = 0;
  virtual bool validate() = 0;

  // ======== NODE LINKS ========
  // different types of connection
  // between nodes in a AAST
  // type, attributes are defined in children classes

  // SCOPE LINK
  // points to a declaration in which scope this decl is
  // i.e. some ClassDecl or FuncDecl
  // std::shared_ptr<Entity> scope;
  // @deprecated now we use symbol table for scope

  // STRUCTURAL LINK
  // children nodes
  std::shared_ptr<Entity> next;

protected:
  Ekind kind;

  Loc location;
};

enum BlockKind {
  BLOCK_IN_CLASS,
  BLOCK_IN_METHOD,
  BLOCK_IN_FUNCTION,
  BLOCK_IN_WHILE,
  BLOCK_IN_IF,
  BLOCK_IN_FOR,
  BLOCK_IN_SWITCH,
};

class Block : public Entity {
public:
  Block(std::vector<std::shared_ptr<Entity>> parts, BlockKind kind)
      : Entity(E_Block), parts(std::move(parts)), kind(kind) {};

  std::vector<std::shared_ptr<Entity>> parts;
  BlockKind kind;

  std::shared_ptr<Type> resolveType(TypeTable typeTable) override {
    (void)typeTable;
    return nullptr;
  }

  bool validate() override { return false; }

  ~Block() override = default;
};

/**
 * This node is pushed to AST
 * in panic mode while doing
 * error recovery
 */
class EDummy : public Entity {
public:
  EDummy() : Entity(E_Dummy) {}

  std::shared_ptr<Type> assumedType;
  TokenKind expectedKind;
  std::string valueIfPresent;
  std::shared_ptr<Type> resolveType(TypeTable typeTable) override {
    (void)typeTable;
    return nullptr;
  }

  bool validate() override { return false; }
};
/**
 * @TODO construct classes for each entitys
 * i.e. `class ClassDeclaration : Entity`
 * this will make it possible to return `Entity*` from some
 * `parse*` functions like for example from parseParameters()
 */

#endif
