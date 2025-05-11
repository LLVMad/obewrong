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
    explicit AssignmentWrapperEXP(std::shared_ptr<AssignmentSTMT> assignment)
        : Expression(E_Assignment_Wrapper), assignment(std::move(assignment)) {}

    std::shared_ptr<AssignmentSTMT> assignment;

    std::shared_ptr<Type> resolveType(TypeTable typeTable) override {
        return assignment->expression->resolveType(typeTable);
    }

    bool validate() override {
        return assignment->validate();
    }
};

#endif