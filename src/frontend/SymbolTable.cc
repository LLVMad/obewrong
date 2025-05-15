#include "frontend/SymbolTable.h"

#include "frontend/types/Decl.h"

void SymbolTable::initBuiltinFunctions(
    const std::shared_ptr<GlobalTypeTable> &typeTable) {
  // auto global = getGlobalScope();

  //=============== GENERAL ===============
  // - printl
  bool isVoid = true;
  // @TODO body??? args??? AnyRef ???
  auto printDecl = std::make_shared<FuncDecl>("printf");

  global_scope->addSymbol("printf", printDecl);

  //========================================

  //========== INTEGER BUILTIN METHODS ==========
  enterScope(SCOPE_MODULE_BUILTIN, "Integer");

  std::vector<std::shared_ptr<Decl>> methods;
  std::vector<std::shared_ptr<TypeFunc>> method_types;

  enterScope(SCOPE_CLASS_BUILTIN, "Integer");
  // - Plus
  auto intPlusTypeReturnType = typeTable->getType("", "Integer");
  auto intPlusTypeParamType = typeTable->getType("", "Integer");
  std::vector paramTypes = {intPlusTypeParamType};
  auto intPlusType =
      std::make_shared<TypeFunc>(intPlusTypeReturnType, paramTypes);
  auto intPlusParamDecl =
      std::make_shared<ParameterDecl>("x", intPlusTypeParamType);
  std::vector<std::shared_ptr<ParameterDecl>> params = {intPlusParamDecl};
  auto intPlusDecl =
      std::make_shared<MethodDecl>("Plus", intPlusType, params, true);
  intPlusDecl->isBuiltin = true;

  current_scope->addSymbol("Plus", intPlusDecl);
  methods.push_back(intPlusDecl);
  method_types.push_back(intPlusType);

  // - Minus
  auto intMinusTypeReturnType = typeTable->getType("", "Integer");
  auto intMinusTypeParamType = typeTable->getType("", "Integer");
  paramTypes = {intMinusTypeParamType};
  auto intMinusType =
      std::make_shared<TypeFunc>(intMinusTypeReturnType, paramTypes);
  auto intMinusParamDecl =
      std::make_shared<ParameterDecl>("x", intMinusTypeParamType);
  params = {intPlusParamDecl};
  auto intMinusDecl =
      std::make_shared<MethodDecl>("Minus", intMinusType, params, true);
  intMinusDecl->isBuiltin = true;

  current_scope->addSymbol("Minus", intMinusDecl);
  methods.push_back(intMinusDecl);
  method_types.push_back(intMinusType);

  // - UnaryMinus
  auto intUMinusTypeReturnType = typeTable->getType("", "Integer");
  // auto intUMinusTypeParamType = typeTable->getType("", "Integer");
  // paramTypes = {intMinusTypeParamType};
  auto intUMinusType =
      std::make_shared<TypeFunc>(intMinusTypeReturnType);
  // auto intUMinusParamDecl =
  //     std::make_shared<ParameterDecl>("x", intMinusTypeParamType);
  params = {};
  auto intUMinusDecl =
      std::make_shared<MethodDecl>("UnaryMinus", intUMinusType, params, true);
  intUMinusDecl->isBuiltin = true;


  current_scope->addSymbol("UnaryMinus", intUMinusDecl);
  methods.push_back(intUMinusDecl);
  method_types.push_back(intUMinusType);

  // - Mult
  auto intMultTypeReturnType = typeTable->getType("", "Integer");
  auto intMultTypeParamType = typeTable->getType("", "Integer");
  paramTypes = {intMultTypeParamType};
  auto intMultType =
      std::make_shared<TypeFunc>(intMultTypeReturnType, paramTypes);
  auto intMultParamDecl =
      std::make_shared<ParameterDecl>("x", intMultTypeParamType);
  params = {intMultParamDecl};
  auto intMultDecl =
      std::make_shared<MethodDecl>("Mult", intMultType, params, true);
  intMultDecl->isBuiltin = true;

  current_scope->addSymbol("Mult", intMultDecl);
  methods.push_back(intMultDecl);
  method_types.push_back(intMultType);

  // - Div
  auto intDivTypeReturnType = typeTable->getType("", "Integer");
  auto intDivTypeParamType = typeTable->getType("", "Integer");
  paramTypes = {intDivTypeParamType};
  auto intDivType =
      std::make_shared<TypeFunc>(intDivTypeReturnType, paramTypes);
  auto intDivParamDecl =
      std::make_shared<ParameterDecl>("x", intDivTypeParamType);
  params = {intDivParamDecl};
  auto intDivDecl =
      std::make_shared<MethodDecl>("Div", intDivType, params, true);
  intDivDecl->isBuiltin = true;

  current_scope->addSymbol("Div", intDivDecl);
  methods.push_back(intDivDecl);
  method_types.push_back(intDivType);

  // - Rem
  auto intRemTypeReturnType = typeTable->getType("", "Integer");
  auto intRemTypeParamType = typeTable->getType("", "Integer");
  paramTypes = {intRemTypeParamType};
  auto intRemType =
      std::make_shared<TypeFunc>(intRemTypeReturnType, paramTypes);
  auto intRemParamDecl =
      std::make_shared<ParameterDecl>("x", intRemTypeParamType);
  params = {intRemParamDecl};
  auto intRemDecl =
      std::make_shared<MethodDecl>("Rem", intRemType, params, true);
  intRemDecl->isBuiltin = true;

  current_scope->addSymbol("Rem", intRemDecl);
  methods.push_back(intRemDecl);
  method_types.push_back(intRemType);

  // - Less
  auto intLessTypeReturnType = typeTable->getType("", "Boolean");
  auto intLessTypeParamType = typeTable->getType("", "Integer");
  paramTypes = {intLessTypeParamType};
  auto intLessType =
      std::make_shared<TypeFunc>(intLessTypeReturnType, paramTypes);
  auto intLessParamDecl =
      std::make_shared<ParameterDecl>("x", intLessTypeParamType);
  params = {intLessParamDecl};
  auto intLessDecl =
      std::make_shared<MethodDecl>("Less", intLessType, params, true);
  intLessDecl->isBuiltin = true;

  current_scope->addSymbol("Less", intLessDecl);
  methods.push_back(intLessDecl);
  method_types.push_back(intLessType);

  // - Greater
  auto intGreaterTypeReturnType = typeTable->getType("", "Boolean");
  auto intGreaterTypeParamType = typeTable->getType("", "Integer");
  paramTypes = {intGreaterTypeParamType};
  auto intGreaterType =
      std::make_shared<TypeFunc>(intGreaterTypeReturnType, paramTypes);
  auto intGreaterParamDecl =
      std::make_shared<ParameterDecl>("x", intGreaterTypeParamType);
  params = {intGreaterParamDecl};
  auto intGreaterDecl =
      std::make_shared<MethodDecl>("Greater", intGreaterType, params, true);
  intGreaterDecl->isBuiltin = true;

  current_scope->addSymbol("Greater", intGreaterDecl);
  methods.push_back(intGreaterDecl);
  method_types.push_back(intGreaterType);

  // - Equal
  auto intEqualTypeReturnType = typeTable->getType("", "Boolean");
  auto intEqualTypeParamType = typeTable->getType("", "Integer");
  paramTypes = {intEqualTypeParamType};
  auto intEqualType =
      std::make_shared<TypeFunc>(intEqualTypeReturnType, paramTypes);
  auto intEqualParamDecl =
      std::make_shared<ParameterDecl>("x", intEqualTypeParamType);
  params = {intEqualParamDecl};
  auto intEqualDecl =
      std::make_shared<MethodDecl>("Equal", intEqualType, params, true);
  intEqualDecl->isBuiltin = true;

  current_scope->addSymbol("Equal", intEqualDecl);
  methods.push_back(intEqualDecl);
  method_types.push_back(intEqualType);

  exitScope();

  // - Integer class decl
  auto integerClassType = std::make_shared<TypeClass>("Integer", std::vector<std::shared_ptr<Type>>(), method_types);
  auto integerDecl = std::make_shared<ClassDecl>("Integer", integerClassType, std::vector<std::shared_ptr<FieldDecl>>(), methods);

  current_scope->addSymbol("Integer", integerDecl);

  exitScope();
  //========================================
  // ... other built-in functions ...
}