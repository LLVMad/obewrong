#ifndef OBW_WRAPPERS_H
#define OBW_WRAPPERS_H

/**
 * Wrappers for mixed constructs that can
 * be viewed both as statements and expressions
 */

#include "Expression.h"

// Forward declaration
class AssignmentSTMT;

class AssignmentWrapperEXP : public Expression {
public:
  explicit AssignmentWrapperEXP(std::shared_ptr<AssignmentSTMT> assignment,
                                Loc loc)
      : Expression(E_Assignment_Wrapper, loc),
        assignment(std::move(assignment)) {}

  std::shared_ptr<AssignmentSTMT> assignment;

  std::shared_ptr<Type>
  resolveType(const TypeTable &typeTable,
              const std::shared_ptr<Scope<Entity>> &currentScope) override {
    return assignment->expression->resolveType(typeTable, currentScope);
  }

  bool validate() override { return assignment->validate(); }

  DEFINE_VISITABLE()
};

#endif