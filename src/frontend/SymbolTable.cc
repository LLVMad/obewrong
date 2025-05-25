#include "frontend/SymbolTable.h"

#include "frontend/types/Decl.h"

void SymbolTable::initBuiltinFunctions(
    const std::shared_ptr<GlobalTypeTable> &typeTable) {
  // auto global = getGlobalScope();

  //=============== GENERAL ===============
  // - printl
  bool isVoid = true;
  Loc loc{0, 0};
  // @TODO body??? args??? AnyRef ???
  auto printDecl = std::make_shared<FuncDecl>(std::string("printf"), loc);

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
      std::make_shared<ParameterDecl>("x", intPlusTypeParamType, loc);
  std::vector<std::shared_ptr<ParameterDecl>> params = {intPlusParamDecl};
  auto intPlusDecl =
      std::make_shared<MethodDecl>("Plus", intPlusType, params, true, loc);
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
      std::make_shared<ParameterDecl>("x", intMinusTypeParamType, loc);
  params = {intPlusParamDecl};
  auto intMinusDecl =
      std::make_shared<MethodDecl>("Minus", intMinusType, params, true, loc);
  intMinusDecl->isBuiltin = true;

  current_scope->addSymbol("Minus", intMinusDecl);
  methods.push_back(intMinusDecl);
  method_types.push_back(intMinusType);

  // - UnaryMinus
  auto intUMinusTypeReturnType = typeTable->getType("", "Integer");
  // auto intUMinusTypeParamType = typeTable->getType("", "Integer");
  // paramTypes = {intMinusTypeParamType};
  auto intUMinusType = std::make_shared<TypeFunc>(intMinusTypeReturnType);
  // auto intUMinusParamDecl =
  //     std::make_shared<ParameterDecl>("x", intMinusTypeParamType);
  params = {};
  auto intUMinusDecl = std::make_shared<MethodDecl>("UnaryMinus", intUMinusType,
                                                    params, true, loc);
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
      std::make_shared<ParameterDecl>("x", intMultTypeParamType, loc);
  params = {intMultParamDecl};
  auto intMultDecl =
      std::make_shared<MethodDecl>("Mult", intMultType, params, true, loc);
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
      std::make_shared<ParameterDecl>("x", intDivTypeParamType, loc);
  params = {intDivParamDecl};
  auto intDivDecl =
      std::make_shared<MethodDecl>("Div", intDivType, params, true, loc);
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
      std::make_shared<ParameterDecl>("x", intRemTypeParamType, loc);
  params = {intRemParamDecl};
  auto intRemDecl =
      std::make_shared<MethodDecl>("Rem", intRemType, params, true, loc);
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
      std::make_shared<ParameterDecl>("x", intLessTypeParamType, loc);
  params = {intLessParamDecl};
  auto intLessDecl =
      std::make_shared<MethodDecl>("Less", intLessType, params, true, loc);
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
      std::make_shared<ParameterDecl>("x", intGreaterTypeParamType, loc);
  params = {intGreaterParamDecl};
  auto intGreaterDecl = std::make_shared<MethodDecl>("Greater", intGreaterType,
                                                     params, true, loc);
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
      std::make_shared<ParameterDecl>("x", intEqualTypeParamType, loc);
  params = {intEqualParamDecl};
  auto intEqualDecl =
      std::make_shared<MethodDecl>("Equal", intEqualType, params, true, loc);
  intEqualDecl->isBuiltin = true;

  current_scope->addSymbol("Equal", intEqualDecl);
  methods.push_back(intEqualDecl);
  method_types.push_back(intEqualType);

  exitScope();

  // - Integer class decl
  auto integerClassType = std::make_shared<TypeClass>(
      "Integer", std::vector<std::shared_ptr<Type>>(), method_types);
  auto integerDecl = std::make_shared<ClassDecl>(
      "Integer", integerClassType, std::vector<std::shared_ptr<FieldDecl>>(),
      methods, loc);

  current_scope->addSymbol("Integer", integerDecl);

  exitScope();
  //========================================

  //========== REAL BUILTIN METHODS ==========
  enterScope(SCOPE_MODULE_BUILTIN, "Real");

  methods.clear();
  method_types.clear();

  enterScope(SCOPE_CLASS_BUILTIN, "Real");
  // - Plus
  auto realPlusTypeReturnType = typeTable->getType("", "Real");
  auto realPlusTypeParamType = typeTable->getType("", "Real");
  paramTypes = {realPlusTypeParamType};
  auto realPlusType =
      std::make_shared<TypeFunc>(realPlusTypeReturnType, paramTypes);
  auto realPlusParamDecl =
      std::make_shared<ParameterDecl>("x", realPlusTypeParamType, loc);
  params = {realPlusParamDecl};
  auto realPlusDecl =
      std::make_shared<MethodDecl>("Plus", realPlusType, params, true, loc);
  realPlusDecl->isBuiltin = true;

  current_scope->addSymbol("Plus", realPlusDecl);
  methods.push_back(realPlusDecl);
  method_types.push_back(realPlusType);

  // - Minus
  auto realMinusTypeReturnType = typeTable->getType("", "Real");
  auto realMinusTypeParamType = typeTable->getType("", "Real");
  paramTypes = {realMinusTypeParamType};
  auto realMinusType =
      std::make_shared<TypeFunc>(realMinusTypeReturnType, paramTypes);
  auto realMinusParamDecl =
      std::make_shared<ParameterDecl>("x", realMinusTypeParamType, loc);
  params = {realMinusParamDecl};
  auto realMinusDecl =
      std::make_shared<MethodDecl>("Minus", realMinusType, params, true, loc);
  realMinusDecl->isBuiltin = true;

  current_scope->addSymbol("Minus", realMinusDecl);
  methods.push_back(realMinusDecl);
  method_types.push_back(realMinusType);

  // - UnaryMinus
  auto realUMinusTypeReturnType = typeTable->getType("", "Real");
  auto realUMinusType = std::make_shared<TypeFunc>(realUMinusTypeReturnType);
  params = {};
  auto realUMinusDecl = std::make_shared<MethodDecl>(
      "UnaryMinus", realUMinusType, params, true, loc);
  realUMinusDecl->isBuiltin = true;

  current_scope->addSymbol("UnaryMinus", realUMinusDecl);
  methods.push_back(realUMinusDecl);
  method_types.push_back(realUMinusType);

  // - Mult
  auto realMultTypeReturnType = typeTable->getType("", "Real");
  auto realMultTypeParamType = typeTable->getType("", "Real");
  paramTypes = {realMultTypeParamType};
  auto realMultType =
      std::make_shared<TypeFunc>(realMultTypeReturnType, paramTypes);
  auto realMultParamDecl =
      std::make_shared<ParameterDecl>("x", realMultTypeParamType, loc);
  params = {realMultParamDecl};
  auto realMultDecl =
      std::make_shared<MethodDecl>("Mult", realMultType, params, true, loc);
  realMultDecl->isBuiltin = true;

  current_scope->addSymbol("Mult", realMultDecl);
  methods.push_back(realMultDecl);
  method_types.push_back(realMultType);

  // - Div
  auto realDivTypeReturnType = typeTable->getType("", "Real");
  auto realDivTypeParamType = typeTable->getType("", "Real");
  paramTypes = {realDivTypeParamType};
  auto realDivType =
      std::make_shared<TypeFunc>(realDivTypeReturnType, paramTypes);
  auto realDivParamDecl =
      std::make_shared<ParameterDecl>("x", realDivTypeParamType, loc);
  params = {realDivParamDecl};
  auto realDivDecl =
      std::make_shared<MethodDecl>("Div", realDivType, params, true, loc);
  realDivDecl->isBuiltin = true;

  current_scope->addSymbol("Div", realDivDecl);
  methods.push_back(realDivDecl);
  method_types.push_back(realDivType);

  // - Rem
  auto realRemTypeReturnType = typeTable->getType("", "Real");
  auto realRemTypeParamType = typeTable->getType("", "Real");
  paramTypes = {realRemTypeParamType};
  auto realRemType =
      std::make_shared<TypeFunc>(realRemTypeReturnType, paramTypes);
  auto realRemParamDecl =
      std::make_shared<ParameterDecl>("x", realRemTypeParamType, loc);
  params = {realRemParamDecl};
  auto realRemDecl =
      std::make_shared<MethodDecl>("Rem", realRemType, params, true, loc);
  realRemDecl->isBuiltin = true;

  current_scope->addSymbol("Rem", realRemDecl);
  methods.push_back(realRemDecl);
  method_types.push_back(realRemType);

  // - Less
  auto realLessTypeReturnType = typeTable->getType("", "Boolean");
  auto realLessTypeParamType = typeTable->getType("", "Real");
  paramTypes = {realLessTypeParamType};
  auto realLessType =
      std::make_shared<TypeFunc>(realLessTypeReturnType, paramTypes);
  auto realLessParamDecl =
      std::make_shared<ParameterDecl>("x", realLessTypeParamType, loc);
  params = {realLessParamDecl};
  auto realLessDecl =
      std::make_shared<MethodDecl>("Less", realLessType, params, true, loc);
  realLessDecl->isBuiltin = true;

  current_scope->addSymbol("Less", realLessDecl);
  methods.push_back(realLessDecl);
  method_types.push_back(realLessType);

  // - Greater
  auto realGreaterTypeReturnType = typeTable->getType("", "Boolean");
  auto realGreaterTypeParamType = typeTable->getType("", "Real");
  paramTypes = {realGreaterTypeParamType};
  auto realGreaterType =
      std::make_shared<TypeFunc>(realGreaterTypeReturnType, paramTypes);
  auto realGreaterParamDecl =
      std::make_shared<ParameterDecl>("x", realGreaterTypeParamType, loc);
  params = {realGreaterParamDecl};
  auto realGreaterDecl = std::make_shared<MethodDecl>(
      "Greater", realGreaterType, params, true, loc);
  realGreaterDecl->isBuiltin = true;

  current_scope->addSymbol("Greater", realGreaterDecl);
  methods.push_back(realGreaterDecl);
  method_types.push_back(realGreaterType);

  // - Equal
  auto realEqualTypeReturnType = typeTable->getType("", "Boolean");
  auto realEqualTypeParamType = typeTable->getType("", "Real");
  paramTypes = {realEqualTypeParamType};
  auto realEqualType =
      std::make_shared<TypeFunc>(realEqualTypeReturnType, paramTypes);
  auto realEqualParamDecl =
      std::make_shared<ParameterDecl>("x", realEqualTypeParamType, loc);
  params = {realEqualParamDecl};
  auto realEqualDecl =
      std::make_shared<MethodDecl>("Equal", realEqualType, params, true, loc);
  realEqualDecl->isBuiltin = true;

  current_scope->addSymbol("Equal", realEqualDecl);
  methods.push_back(realEqualDecl);
  method_types.push_back(realEqualType);

  exitScope();

  // - Real class decl
  auto realClassType = std::make_shared<TypeClass>(
      "Real", std::vector<std::shared_ptr<Type>>(), method_types);
  auto realDecl = std::make_shared<ClassDecl>(
      "Real", realClassType, std::vector<std::shared_ptr<FieldDecl>>(), methods,
      loc);

  current_scope->addSymbol("Real", realDecl);

  exitScope();
  //========================================
  // ... other built-in functions ...
}