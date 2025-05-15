#include "frontend/parser/Statement.h"

Statement::~Statement() = default;

std::shared_ptr<Type> Statement::resolveType(const TypeTable &typeTable, const std::shared_ptr<Scope<Entity>> &currentScope) {
  (void)typeTable;
  // @TODO error on type request from STMT
  return nullptr;
}

bool Statement::validate() {
  // @TODO
  return false;
}
