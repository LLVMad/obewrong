#include "frontend/parser/Statement.h"

Statement::~Statement() = default;

std::shared_ptr<Type> Statement::resolveType(TypeTable typeTable) {
  (void)typeTable;
  // @TODO error on type request from STMT
  return nullptr;
}

bool Statement::validate() {
  // @TODO
  return false;
}

