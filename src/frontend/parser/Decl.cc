#include "frontend/types/Decl.h"

Decl::~Decl() = default;

std::shared_ptr<Type> Decl::resolveType(const TypeTable &typeTable, const std::shared_ptr<Scope<Entity>> &currentScope) {
  // @TODO
  return nullptr;
}

bool Decl::validate() { return false; }

std::shared_ptr<Type> FieldDecl::resolveType(const TypeTable &typeTable, const std::shared_ptr<Scope<Entity>> &currentScope) {
  (void)typeTable;
  return type;
}

bool FieldDecl::validate() {
  // @TODO
  return true;
}

std::shared_ptr<Type> VarDecl::resolveType(const TypeTable &typeTable, const std::shared_ptr<Scope<Entity>> &currentScope) {
  (void)typeTable;
  return type;
}

bool VarDecl::validate() {
  // @TODO
  return true;
}

std::shared_ptr<Type> ParameterDecl::resolveType(const TypeTable &typeTable, const std::shared_ptr<Scope<Entity>> &currentScope) {
  (void)typeTable;
  return type;
}

bool ParameterDecl::validate() {
  // @TODO
  return true;
}

std::shared_ptr<Type> MethodDecl::resolveType(const TypeTable &typeTable, const std::shared_ptr<Scope<Entity>> &currentScope) {
  (void)typeTable;
  return signature->return_type;
}

bool MethodDecl::validate() {
  // @TODO
  return true;
}

std::shared_ptr<Type> FuncDecl::resolveType(const TypeTable &typeTable, const std::shared_ptr<Scope<Entity>> &currentScope) {
  (void)typeTable;
  return signature->return_type;
}

bool FuncDecl::validate() {
  // @TODO
  return true;
}

std::shared_ptr<Type> ClassDecl::resolveType(const TypeTable &typeTable, const std::shared_ptr<Scope<Entity>> &currentScope) {
  (void)typeTable;
  return type;
}

bool ClassDecl::validate() {
  // @TODO
  return true;
}

std::shared_ptr<Type> ArrayDecl::resolveType(const TypeTable &typeTable, const std::shared_ptr<Scope<Entity>> &currentScope) {
  (void)typeTable;
  return type;
}

bool ArrayDecl::validate() {
  // @TODO
  return true;
}

std::shared_ptr<Type> ListDecl::resolveType(const TypeTable &typeTable, const std::shared_ptr<Scope<Entity>> &currentScope) {
  (void)typeTable;
  return type;
}

bool ListDecl::validate() {
  // @TODO
  return true;
}

std::shared_ptr<Type> ConstrDecl::resolveType(const TypeTable &typeTable, const std::shared_ptr<Scope<Entity>> &currentScope) {
  (void)typeTable;
  return signature;
}

bool ConstrDecl::validate() {
  // @TODO
  return true;
}