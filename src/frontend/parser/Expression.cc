#include "frontend/parser/Expression.h"

#include "frontend/types/Decl.h"

Expression::~Expression() {}

std::shared_ptr<Type> IntLiteralEXP::resolveType(const TypeTable &typeTable, const std::shared_ptr<Scope<Entity>> &currentScope) {
  // auto type = std::make_unique<TypeBuiltin>(TYPE_INT, "Integer", 32);
  // return type;
  return typeTable.getType("Integer");
}

std::shared_ptr<Type> RealLiteralEXP::resolveType(const TypeTable &typeTable, const std::shared_ptr<Scope<Entity>> &currentScope) {
  return typeTable.getType("Real");
}

std::shared_ptr<Type> StringLiteralEXP::resolveType(const TypeTable &typeTable, const std::shared_ptr<Scope<Entity>> &currentScope) {
  // auto type = std::make_unique<TypeString>(sizeof(value.c_str()));
  // return type;
  return std::make_shared<TypeAccess>(typeTable.getType("byte"));
}

std::shared_ptr<Type> BoolLiteralEXP::resolveType(const TypeTable &typeTable, const std::shared_ptr<Scope<Entity>> &currentScope) {
  // auto type = std::make_unique<TypeBuiltin>(TYPE_BOOL, "Bool", 1);
  // return type;
  return typeTable.getType("Bool");
}

std::shared_ptr<Type> ArrayLiteralExpr::resolveType(const TypeTable &typeTable, const std::shared_ptr<Scope<Entity>> &currentScope) {
  // name of ArrayType = Array[<type_of_el_1>_<size>]
  // @TODO multi diemnsion arrays and TPYE_KIND
  auto elType = typeTable.getType(el_type);
  auto arrayTypeName = "Array[" + elType->name + "_" + std::to_string(elements.size()) + "]";
  return typeTable.getType(arrayTypeName);
}

bool ArrayLiteralExpr::validate() {
  // Ensure the class exists and arguments match the constructor.
  return true;
}

std::shared_ptr<Type> FieldRefEXP::resolveType(const TypeTable &typeTable, const std::shared_ptr<Scope<Entity>> &currentScope) {
  std::string objTypeName;

  if (el) {
    auto arrType = el->arr->resolveType(typeTable, currentScope);
    auto elType = std::dynamic_pointer_cast<TypeArray>(arrType)->el_type;
    objTypeName = elType->name;
  }
  else {
    auto objType = obj->resolveType(typeTable, currentScope);
    objTypeName = objType->name;

    if (obj->getKind() == E_This) {
      objTypeName = currentScope->prevScope()->getName();
    }
  }

  auto [decl, alloca, isInited] = *currentScope->getSymbol(objTypeName);
  auto classDecl = std::dynamic_pointer_cast<ClassDecl>(decl);
  auto fieldDecl = std::find_if(
    classDecl->fields.begin(),
    classDecl->fields.end(),
    [this](const std::shared_ptr<FieldDecl> &fd) { return fd->getName() == this->name; });
  if (fieldDecl == classDecl->fields.end()) return nullptr;
  return (*fieldDecl)->type;
  return nullptr;
}

bool FieldRefEXP::validate() {
  // Ensure the class exists and arguments match the constructor.
  return true;
}

std::shared_ptr<Type> VarRefEXP::resolveType(const TypeTable &typeTable, const std::shared_ptr<Scope<Entity>> &currentScope) {
  auto [decl, alloca, isInited] = *currentScope->getSymbol(name);
  return decl->resolveType(typeTable, currentScope);
}

bool VarRefEXP::validate() {
  // Ensure the class exists and arguments match the constructor.
  return true;
}

std::shared_ptr<Type> MethodCallEXP::resolveType(const TypeTable &typeTable, const std::shared_ptr<Scope<Entity>> &currentScope) {
  // auto varRef = std::static_pointer_cast<VarRefEXP>(left);

  // nested
  auto leftOp = left;
  while (leftOp->getKind() == E_Method_Call) {
    leftOp = std::static_pointer_cast<MethodCallEXP>(leftOp)->left;
  }

  auto [decl, _, __] = *currentScope->getSymbol(leftOp->getName());
  auto typeNameOfLeftOperand = decl->resolveType(typeTable, currentScope)->name;

  auto [typeDecl, ___, ____] = *currentScope->getSymbol(typeNameOfLeftOperand);
  auto typeDeclAsCalss = std::dynamic_pointer_cast<ClassDecl>(typeDecl);
  auto methodDecl = std::find_if(
    typeDeclAsCalss->methods.begin(),
    typeDeclAsCalss->methods.end(),
    [this](const std::shared_ptr<Decl> &md) { return md->getName() == this->name; });

  return (*methodDecl)->resolveType(typeTable, currentScope);

  return nullptr;
}

bool MethodCallEXP::validate() {
  return true;
}

std::shared_ptr<Type> FuncCallEXP::resolveType(const TypeTable &typeTable, const std::shared_ptr<Scope<Entity>> &currentScope) {
  auto symbolInfo = currentScope->getSymbol(name);
  return symbolInfo->decl->resolveType(typeTable, currentScope);
  // return nullptr;
}

bool FuncCallEXP::validate() {
  return true;
}

std::shared_ptr<Type> ClassNameEXP::resolveType(const TypeTable &typeTable, const std::shared_ptr<Scope<Entity>> &currentScope) {
  // (void)typeTable;
  return typeTable.getType(name);
}

bool ClassNameEXP::validate() {
  return true;
}

std::shared_ptr<Type> ConstructorCallEXP::resolveType(const TypeTable &typeTable, const std::shared_ptr<Scope<Entity>> &currentScope) {
  // (void)typeTable;
  // void, becouse we pass this (self ref) as first argument and return nothing
  return this->left->resolveType(typeTable, currentScope); // @FIXME
  return nullptr;
}

bool ConstructorCallEXP::validate() { return true; }

std::shared_ptr<Type> CompoundEXP::resolveType(const TypeTable &typeTable, const std::shared_ptr<Scope<Entity>> &currentScope) {
  // last executed method -> type returned
  return parts.back()->resolveType(typeTable, currentScope);
  return nullptr;
}

bool CompoundEXP::validate() { return true; }

std::shared_ptr<Type> ThisEXP::resolveType(const TypeTable &typeTable, const std::shared_ptr<Scope<Entity>> &currentScope) {
  // current scope name will be method/constr so outer scope above is named the same
  // as a class
  auto className = currentScope->prevScope()->getName();
  return typeTable.getType("this" + className);
  return nullptr;
}

bool ThisEXP::validate() {
  // ensure `this` is used within a method/constructor.
  // Временная заглушка
  return true;
}