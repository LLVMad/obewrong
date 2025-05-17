#ifndef OBW_EINFO_H
#define OBW_EINFO_H

#include "../Scope.h"
#include "../TypeTable.h"
#include "frontend/lexer/Lexer.h"
#include "Visitor.h"

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
  E_Assignment_Wrapper, // Wraps assignments to be used in expressions
  E_Conversion,

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
class Entity : public VisitableBase {
public:
  virtual ~Entity() = default;
  explicit Entity(Ekind kind) : kind(kind), name(), location() {}

  explicit Entity(Ekind kind, std::string name) : kind(kind), name(std::move(name)), location() {}

  Ekind getKind() const { return kind; };
  Loc getLoc() const { return location; };
  std::string getName() const { return name; };
  void appendToName(const std::string &name) { this->name += name; };

  virtual std::shared_ptr<Type> resolveType(const TypeTable &typeTable, const std::shared_ptr<Scope<Entity>> &currentScope) = 0;
  virtual bool validate() = 0;

  // STRUCTURAL LINK
  // children nodes
  std::shared_ptr<Entity> next;

  // DEFINE_VISITABLE()

protected:
  Ekind kind;
  std::string name;
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

  std::shared_ptr<Type> resolveType(const TypeTable &typeTable, const std::shared_ptr<Scope<Entity>> &currentScope) override {
    (void)typeTable;
    return nullptr;
  }

  bool validate() override { return false; }

  DEFINE_VISITABLE()

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
  std::shared_ptr<Type> resolveType(const TypeTable &typeTable, const std::shared_ptr<Scope<Entity>> &currentScope) override {
    (void)typeTable;
    return nullptr;
  }

  bool validate() override { return false; }

  DEFINE_VISITABLE()
};

template<typename T>
std::shared_ptr<T> castEntity(const std::shared_ptr<Entity>& node) {
    return std::static_pointer_cast<T>(node);
}

#define DERIVED(node, visitor) \
    switch (node->getKind()) { \
      /* Expressions */ \
      case E_Integer_Literal: std::static_pointer_cast<IntLiteralEXP>(node)->acceptAGuestVisitor(visitor); break; \
      case E_Real_Literal: std::static_pointer_cast<RealLiteralEXP>(node)->acceptAGuestVisitor(visitor); break; \
      case E_String_Literal: std::static_pointer_cast<StringLiteralEXP>(node)->acceptAGuestVisitor(visitor); break; \
      case E_Array_Literal: std::static_pointer_cast<ArrayLiteralExpr>(node)->acceptAGuestVisitor(visitor); break; \
      case E_Element_Reference: std::static_pointer_cast<ElementRefEXP>(node)->acceptAGuestVisitor(visitor); break; \
      case E_Binary_Operator: std::static_pointer_cast<BinaryOpEXP>(node)->acceptAGuestVisitor(visitor); break; \
      case E_Function_Call: std::static_pointer_cast<FuncCallEXP>(node)->acceptAGuestVisitor(visitor); break; \
      case E_Constructor_Call: std::static_pointer_cast<ConstructorCallEXP>(node)->acceptAGuestVisitor(visitor); break; \
      case E_Method_Call: std::static_pointer_cast<MethodCallEXP>(node)->acceptAGuestVisitor(visitor); break; \
      case E_Var_Reference: std::static_pointer_cast<VarRefEXP>(node)->acceptAGuestVisitor(visitor); break; \
      case E_Field_Reference: std::static_pointer_cast<FieldRefEXP>(node)->acceptAGuestVisitor(visitor); break; \
      case E_Assignment_Wrapper: std::static_pointer_cast<AssignmentWrapperEXP>(node)->acceptAGuestVisitor(visitor); break; \
      case E_Chained_Functions: std::static_pointer_cast<CompoundEXP>(node)->acceptAGuestVisitor(visitor); break; \
      case E_Conversion: std::static_pointer_cast<ConversionEXP>(node)->acceptAGuestVisitor(visitor); break; \
      /* Declarations */ \
      case E_Field_Decl: std::static_pointer_cast<FieldDecl>(node)->acceptAGuestVisitor(visitor); break; \
      case E_Variable_Decl: std::static_pointer_cast<VarDecl>(node)->acceptAGuestVisitor(visitor); break; \
      case E_Parameter_Decl: std::static_pointer_cast<ParameterDecl>(node)->acceptAGuestVisitor(visitor); break; \
      case E_Method_Decl: std::static_pointer_cast<MethodDecl>(node)->acceptAGuestVisitor(visitor); break; \
      case E_Constructor_Decl: std::static_pointer_cast<ConstrDecl>(node)->acceptAGuestVisitor(visitor); break; \
      case E_Function_Decl: std::static_pointer_cast<FuncDecl>(node)->acceptAGuestVisitor(visitor); break; \
      case E_Main_Decl: std::static_pointer_cast<FuncDecl>(node)->acceptAGuestVisitor(visitor); break; \
      case E_Class_Decl: std::static_pointer_cast<ClassDecl>(node)->acceptAGuestVisitor(visitor); break; \
      case E_Module_Decl: std::static_pointer_cast<ModuleDecl>(node)->acceptAGuestVisitor(visitor); break; \
      case E_Enum_Decl: std::static_pointer_cast<EnumDecl>(node)->acceptAGuestVisitor(visitor); break; \
      /* Statements */ \
      case E_Assignment: std::static_pointer_cast<AssignmentSTMT>(node)->acceptAGuestVisitor(visitor); break; \
      case E_Return_Statement: std::static_pointer_cast<ReturnSTMT>(node)->acceptAGuestVisitor(visitor); break; \
      case E_For_Loop: std::static_pointer_cast<ForSTMT>(node)->acceptAGuestVisitor(visitor); break; \
      case E_If_Statement: std::static_pointer_cast<IfSTMT>(node)->acceptAGuestVisitor(visitor); break; \
      case E_Block: std::static_pointer_cast<Block>(node)->acceptAGuestVisitor(visitor); break; \
      default: node = node; \
    } \

#endif
