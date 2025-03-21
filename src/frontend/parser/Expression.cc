#include "frontend/parser/Expression.h"

std::unique_ptr<Type> Expression::resolveType() const {
  return new Type(TYPE_UNKNOWN, "Unknown");
}

bool Expression::validate() {
  return true;
}