#include "frontend/parser/Expression.h"

Expression::~Expression() {}

std::shared_ptr<Type> IntLiteralEXP::resolveType(TypeTable typeTable) {
  // auto type = std::make_unique<TypeBuiltin>(TYPE_INT, "Integer", 32);
  // return type;
  return typeTable.getType("Integer");
}

std::shared_ptr<Type> RealLiteralEXP::resolveType(TypeTable typeTable) {
  return typeTable.getType("Real");
}

std::shared_ptr<Type> StringLiteralEXP::resolveType(TypeTable typeTable) {
  // auto type = std::make_unique<TypeString>(sizeof(value.c_str()));
  // return type;
  return typeTable.getType("String");
}

std::shared_ptr<Type> BoolLiteralEXP::resolveType(TypeTable typeTable) {
  // auto type = std::make_unique<TypeBuiltin>(TYPE_BOOL, "Bool", 1);
  // return type;
  return typeTable.getType("Bool");
}

std::shared_ptr<Type> ArrayLiteralExpr::resolveType(TypeTable typeTable) {
  (void)typeTable;
  /*
   * @TODO
   * search for a FieldDecl IN class
   *
   */
  return nullptr;
}

bool ArrayLiteralExpr::validate() {
  // Ensure the class exists and arguments match the constructor.
  return true;
}

std::shared_ptr<Type> FieldRefEXP::resolveType(TypeTable typeTable) {
  (void)typeTable;
  /*
   * @TODO
   * search for a FieldDecl IN class
   *
   */
  return nullptr;
}

bool FieldRefEXP::validate() {
  // Ensure the class exists and arguments match the constructor.
  return true;
}

std::shared_ptr<Type> VarRefEXP::resolveType(TypeTable typeTable) {
  (void)typeTable;
  /*
   * @TODO
   * search for a VarDecl IN SCOPE
   *
   */
  return nullptr;
}

bool VarRefEXP::validate() {
  // Ensure the class exists and arguments match the constructor.
  return true;
}

std::shared_ptr<Type> MethodCallEXP::resolveType(TypeTable typeTable) {
  (void)typeTable;
  // @TODO
  return nullptr;
}

bool MethodCallEXP::validate() { return true; }

std::shared_ptr<Type> FuncCallEXP::resolveType(TypeTable typeTable) {
  (void)typeTable;
  // @TODO
  return nullptr;
}

bool FuncCallEXP::validate() { return true; }

std::shared_ptr<Type> ClassNameEXP::resolveType(TypeTable typeTable) {
  (void)typeTable;
  // if (_name == "Integer") {
  //   return
  // } else if (_name == "Real") {
  //   return std::make_unique<TypeReal>();
  // } else if (_name == "Bool") {
  //   return std::make_unique<TypeBool>();
  // } else if (_name == "String") {
  //   return std::make_unique<TypeString>();
  // } else if (_name == "Array") {
  //   return std::make_unique<TypeArray>();
  // } else if (_name == "List") {
  //   return std::make_unique<TypeList>();
  // } else {
  //   return std::make_unique<Type>(TYPE_UNKNOWN, _name);
  // }
  // @TODO not found
  return typeTable.getType(name);
}

bool ClassNameEXP::validate() { return true; }

std::shared_ptr<Type> ConstructorCallEXP::resolveType(TypeTable typeTable) {
  (void)typeTable;
  /*
   * @TODO
   * this is interesting
   * because as far as i understand
   * the return type of this
   * should be a pointer to a class
   * but we dont have pointers...
   */
  return nullptr;
}

bool ConstructorCallEXP::validate() { return true; }

// std::shared_ptr<Type> FieldAccessEXP::resolveType(TypeTable typeTable) {
//   (void)typeTable;
//   /*
//    * @TODO
//    * 1) search for left side in a class declarations
//    * 2) get type of a field by name
//    * 3) return its type
//    */
//   return nullptr;
// }
//
// bool FieldAccessEXP::validate() { return true; }

std::shared_ptr<Type> CompoundEXP::resolveType(TypeTable typeTable) {
  (void)typeTable;
  /*
   * @TODO
   * call resolveType on each
   * exp in parts
   */
  // Временная заглушка
  // return std::make_unique<Type>(TYPE_UNKNOWN, "Unresolved compound type");
  return nullptr;
}

bool CompoundEXP::validate() { return true; }

std::shared_ptr<Type> ThisEXP::resolveType(TypeTable typeTable) {
  (void)typeTable;
  // return the type of the enclosing class (from the scope).
  // Временная заглушка
  // return std::make_unique<Type>(TYPE_UNKNOWN, "Unresolved 'this' type");
  return nullptr;
}

bool ThisEXP::validate() {
  // ensure `this` is used within a method/constructor.
  // Временная заглушка
  return true;
}