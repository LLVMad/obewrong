#include "frontend/types/Decl.h"

Decl::~Decl() = default;

std::shared_ptr<Type> Decl::resolveType(TypeTable typeTable) {
  // @TODO
  return nullptr;
}

bool Decl::validate() { return false; }

std::shared_ptr<Type> FieldDecl::resolveType(TypeTable typeTable) {
  (void)typeTable;
  return type;
}

bool FieldDecl::validate() {
  // @TODO
  return true;
}

std::shared_ptr<Type> VarDecl::resolveType(TypeTable typeTable) {
  (void)typeTable;
  return type;
}

bool VarDecl::validate() {
  // @TODO
  return true;
}

std::shared_ptr<Type> ParameterDecl::resolveType(TypeTable typeTable) {
  (void)typeTable;
  return type;
}

bool ParameterDecl::validate() {
  // @TODO
  return true;
}

std::shared_ptr<Type> MethodDecl::resolveType(TypeTable typeTable) {
  (void)typeTable;
  return signature;
}

bool MethodDecl::validate() {
  // @TODO
  return true;
}

std::shared_ptr<Type> FuncDecl::resolveType(TypeTable typeTable) {
  (void)typeTable;
  return signature;
}

bool FuncDecl::validate() {
  // @TODO
  return true;
}

std::shared_ptr<Type> ClassDecl::resolveType(TypeTable typeTable) {
  (void)typeTable;
  return type;
}

bool ClassDecl::validate() {
  // @TODO
  return true;
}

std::shared_ptr<Type> ArrayDecl::resolveType(TypeTable typeTable) {
  (void)typeTable;
  return type;
}

bool ArrayDecl::validate() {
  // @TODO
  return true;
}

std::shared_ptr<Type> ListDecl::resolveType(TypeTable typeTable) {
  (void)typeTable;
  return type;
}

bool ListDecl::validate() {
  // @TODO
  return true;
}

std::shared_ptr<Type> ConstrDecl::resolveType(TypeTable typeTable) {
  (void)typeTable;
  return nullptr;
}

bool ConstrDecl::validate() {
  // @TODO
  return true;
}