#ifndef STATEMENT_H
#define STATEMENT_H

/*
 * Entities that control
 * the control-flow of program
 */
#include <memory>
#include <utility>
#include <vector>

#include "Entity.h"
#include "Expression.h"
#include "frontend/types/Decl.h"
#include "frontend/types/Types.h"

class Statement : public Entity {
public:
  explicit Statement(Ekind kind, Loc loc) : Entity(kind, loc) {}

  std::shared_ptr<Type>
  resolveType(const TypeTable &typeTable,
              const std::shared_ptr<Scope<Entity>> &currentScope) override;

  bool validate() override;
  // std::string name;
  ~Statement() override;

  DEFINE_VISITABLE()
};

enum AssKind {
  VAR_ASS,
  FIELD_ASS,
  EL_ASS, // a[0] :=
  NONE,
};

// Identifier := Expression
class AssignmentSTMT : public Statement {
public:
  AssignmentSTMT(std::shared_ptr<VarRefEXP> lhs,
                 std::shared_ptr<Expression> rhs, Loc loc)
      : Statement(E_Assignment, loc), assKind(VAR_ASS),
        variable(std::move(lhs)), field(nullptr), element(nullptr),
        expression(std::move(rhs)) {}

  AssignmentSTMT(std::shared_ptr<FieldRefEXP> lhs,
                 std::shared_ptr<Expression> rhs, Loc loc)
      : Statement(E_Assignment, loc), assKind(FIELD_ASS), variable(nullptr),
        field(std::move(lhs)), element(nullptr), expression(std::move(rhs)) {}

  AssignmentSTMT(std::shared_ptr<ElementRefEXP> lhs,
                 std::shared_ptr<Expression> rhs, Loc loc)
      : Statement(E_Assignment, loc), assKind(EL_ASS), variable(nullptr),
        field(nullptr), element(std::move(lhs)), expression(std::move(rhs)) {}

  AssignmentSTMT(std::shared_ptr<Expression> lhs,
                 std::shared_ptr<Expression> rhs, Loc loc)
      : Statement(E_Assignment, loc) {
    switch (lhs->getKind()) {
    case E_Element_Reference: {
      element = std::static_pointer_cast<ElementRefEXP>(std::move(lhs));
      variable = nullptr;
      field = nullptr;
      assKind = EL_ASS;
      expression = std::move(rhs);
    } break;
    case E_Field_Reference: {
      field = std::static_pointer_cast<FieldRefEXP>(std::move(lhs));
      variable = nullptr;
      element = nullptr;
      assKind = FIELD_ASS;
      expression = std::move(rhs);
    } break;
    case E_Var_Reference: {
      variable = std::static_pointer_cast<VarRefEXP>(std::move(lhs));
      field = nullptr;
      element = nullptr;
      assKind = VAR_ASS;
      expression = std::move(rhs);
    } break;
    default: {
      variable = nullptr;
      field = nullptr;
      element = nullptr;
      assKind = NONE;
      expression = std::move(rhs);
    } break;
    }
  }

  // children are
  AssKind assKind;

  std::shared_ptr<VarRefEXP> variable;
  std::shared_ptr<FieldRefEXP> field;
  std::shared_ptr<ElementRefEXP> element;

  std::shared_ptr<Expression> expression;

  ~AssignmentSTMT() override = default;
  // void addVariable(std::shared_ptr<VarRefEXP> variable) {
  //   this->children.push_back(std::move(variable));
  // }
  //
  // void addExpression(std::shared_ptr<Expression> expression) {
  //   this->children.push_back(std::move(expression));
  // }

  DEFINE_VISITABLE()
};

// return [ Expression ]
class ReturnSTMT : public Statement {
public:
  explicit ReturnSTMT(std::shared_ptr<Expression> expr, Loc loc)
      : Statement(E_Return_Statement, loc), expr(std::move(expr)) {}

  [[deprecated("Use for error recovery only")]] ReturnSTMT(Loc loc)
      : Statement(E_Return_Statement, loc) {}

  // children are
  std::shared_ptr<Expression> expr;
  ~ReturnSTMT() override = default;
  // void addExpression(std::shared_ptr<Expression> expression) {
  //   this->children.push_back(std::move(expression));
  // }

  DEFINE_VISITABLE()
};

// class ElseSTMT : public Statement {
// public:
// }

// if Expression then Body [ else Body ] end
class IfSTMT : public Statement {
public:
  IfSTMT(std::shared_ptr<Expression> condition, std::shared_ptr<Block> ifTrue,
         std::shared_ptr<Block> ifFalse, bool elsed, Loc loc)
      : Statement(E_If_Statement, loc), condition(std::move(condition)),
        ifTrue(std::move(ifTrue)), ifFalse(std::move(ifFalse)), isElsed(elsed) {
  }

  IfSTMT(std::shared_ptr<Expression> condition, std::shared_ptr<Block> ifTrue,
         Loc loc)
      : Statement(E_If_Statement, loc), condition(std::move(condition)),
        ifTrue(std::move(ifTrue)), ifFalse(nullptr), isElsed(false) {}

  IfSTMT(std::shared_ptr<Expression> condition, std::shared_ptr<Block> ifTrue,
         std::shared_ptr<Entity> ifFalse, Loc loc)
      : Statement(E_If_Statement, loc), condition(std::move(condition)),
        ifTrue(std::move(ifTrue)), ifFalse(std::move(ifFalse)), isElsed(true) {}

  // children are
  std::shared_ptr<Expression> condition;
  std::shared_ptr<Block> ifTrue;
  std::shared_ptr<Entity> ifFalse;
  bool isElsed;

  ~IfSTMT() override = default;

  DEFINE_VISITABLE()
};

class CaseSTMT : public Statement {
public:
  CaseSTMT(std::shared_ptr<Expression> condition, std::shared_ptr<Block> body,
           Loc loc)
      : Statement(E_Case_Statement, loc),
        condition_literal(std::move(condition)), body(std::move(body)),
        isDefault(false) {}

  CaseSTMT(std::shared_ptr<Block> body, Loc loc)
      : Statement(E_Case_Statement, loc), body(std::move(body)),
        isDefault(true) {}

  std::shared_ptr<Expression> condition_literal;
  std::shared_ptr<Block> body;
  bool isDefault;

  ~CaseSTMT() override = default;

  DEFINE_VISITABLE()
};

class SwitchSTMT : public Statement {
public:
  SwitchSTMT(std::shared_ptr<Expression> condition,
             std::vector<std::shared_ptr<CaseSTMT>> cases, Loc loc)
      : Statement(E_Switch_Statement, loc), condition(std::move(condition)),
        cases(std::move(cases)) {}

  explicit SwitchSTMT(std::shared_ptr<Expression> condition, Loc loc)
      : Statement(E_Switch_Statement, loc), condition(std::move(condition)) {}

  std::shared_ptr<Expression> condition;
  std::vector<std::shared_ptr<CaseSTMT>> cases;

  ~SwitchSTMT() override = default;

  // void addCase(std::shared_ptr<Expression> condition,
  //              std::shared_ptr<Block> body) {
  //   cases.push_back(
  //       std::make_unique<CaseSTMT>(std::move(condition), std::move(body)));
  // }

  void addCase(std::shared_ptr<CaseSTMT> cas) {
    cases.push_back(std::move(cas));
  }

  DEFINE_VISITABLE()
};

/**
 * A while loop begins by evaluating
 * the boolean loop conditional operand.
 * If the loop conditional operand evaluates to true,
 * the loop body block executes,
 * then control returns to the loop conditional operand.
 *
 * while a.Less(0) then
 *   a.Minus(1)
 * end
 */
class WhileSTMT : public Statement {
public:
  explicit WhileSTMT(std::shared_ptr<Expression> condition,
                     std::shared_ptr<Block> body, Loc loc)
      : Statement(E_While_Loop, loc), condition(std::move(condition)),
        body(std::move(body)) {}

  std::shared_ptr<Expression> condition;
  std::shared_ptr<Block> body;

  ~WhileSTMT() override = default;

  DEFINE_VISITABLE()
};

class ForSTMT : public Statement {
public:
  explicit ForSTMT(std::shared_ptr<VarRefEXP> varWithAss,
                   std::shared_ptr<Expression> condition,
                   std::shared_ptr<AssignmentSTMT> post,
                   std::shared_ptr<Block> body, Loc loc)
      : Statement(E_For_Loop, loc), varWithAss(std::move(varWithAss)),
        condition(std::move(condition)), post(std::move(post)),
        body(std::move(body)) {}

  [[deprecated("Use only for parse error")]] ForSTMT(Loc loc)
      : Statement(E_Dummy, loc) {}

  std::shared_ptr<VarRefEXP> varWithAss;
  std::shared_ptr<Expression> condition;
  std::shared_ptr<AssignmentSTMT> post;
  std::shared_ptr<Block> body;

  ~ForSTMT() override = default;

  DEFINE_VISITABLE()
};

#endif