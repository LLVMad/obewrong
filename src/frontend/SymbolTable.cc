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
      std::make_shared<ParameterDecl>("x", realPlusTypeParamType);
  params = {realPlusParamDecl};
  auto realPlusDecl =
      std::make_shared<MethodDecl>("Plus", realPlusType, params, true);
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
      std::make_shared<ParameterDecl>("x", realMinusTypeParamType);
  params = {realMinusParamDecl};
  auto realMinusDecl =
      std::make_shared<MethodDecl>("Minus", realMinusType, params, true);
  realMinusDecl->isBuiltin = true;

  current_scope->addSymbol("Minus", realMinusDecl);
  methods.push_back(realMinusDecl);
  method_types.push_back(realMinusType);

  // - UnaryMinus
  auto realUMinusTypeReturnType = typeTable->getType("", "Real");
  auto realUMinusType =
      std::make_shared<TypeFunc>(realUMinusTypeReturnType);
  params = {};
  auto realUMinusDecl =
      std::make_shared<MethodDecl>("UnaryMinus", realUMinusType, params, true);
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
      std::make_shared<ParameterDecl>("x", realMultTypeParamType);
  params = {realMultParamDecl};
  auto realMultDecl =
      std::make_shared<MethodDecl>("Mult", realMultType, params, true);
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
      std::make_shared<ParameterDecl>("x", realDivTypeParamType);
  params = {realDivParamDecl};
  auto realDivDecl =
      std::make_shared<MethodDecl>("Div", realDivType, params, true);
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
      std::make_shared<ParameterDecl>("x", realRemTypeParamType);
  params = {realRemParamDecl};
  auto realRemDecl =
      std::make_shared<MethodDecl>("Rem", realRemType, params, true);
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
      std::make_shared<ParameterDecl>("x", realLessTypeParamType);
  params = {realLessParamDecl};
  auto realLessDecl =
      std::make_shared<MethodDecl>("Less", realLessType, params, true);
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
      std::make_shared<ParameterDecl>("x", realGreaterTypeParamType);
  params = {realGreaterParamDecl};
  auto realGreaterDecl =
      std::make_shared<MethodDecl>("Greater", realGreaterType, params, true);
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
      std::make_shared<ParameterDecl>("x", realEqualTypeParamType);
  params = {realEqualParamDecl};
  auto realEqualDecl =
      std::make_shared<MethodDecl>("Equal", realEqualType, params, true);
  realEqualDecl->isBuiltin = true;

  current_scope->addSymbol("Equal", realEqualDecl);
  methods.push_back(realEqualDecl);
  method_types.push_back(realEqualType);

  exitScope();

  // - Real class decl
  auto realClassType = std::make_shared<TypeClass>("Real", std::vector<std::shared_ptr<Type>>(), method_types);
  auto realDecl = std::make_shared<ClassDecl>("Real", realClassType, std::vector<std::shared_ptr<FieldDecl>>(), methods);

  current_scope->addSymbol("Real", realDecl);

  exitScope();
  //========================================
  //========== BOOLEAN BUILTIN METHODS ==========
  enterScope(SCOPE_MODULE_BUILTIN, "Boolean");
  std::vector<std::shared_ptr<Decl>> booleanMethods;
  std::vector<std::shared_ptr<TypeFunc>> booleanMethodTypes;
  enterScope(SCOPE_CLASS_BUILTIN, "Boolean");

  // - And
  auto boolAndTypeReturnType = typeTable->getType("", "Boolean");
  auto boolAndTypeParamType = typeTable->getType("", "Boolean");
  paramTypes = {boolAndTypeParamType};
  auto boolAndType = std::make_shared<TypeFunc>(boolAndTypeReturnType, paramTypes);
  auto boolAndParamDecl = std::make_shared<ParameterDecl>("x", boolAndTypeParamType);
  params = {boolAndParamDecl};
  auto boolAndDecl = std::make_shared<MethodDecl>("And", boolAndType, params, true);
  boolAndDecl->isBuiltin = true;
  current_scope->addSymbol("And", boolAndDecl);
  booleanMethods.push_back(boolAndDecl);
  booleanMethodTypes.push_back(boolAndType);

  // - Or
  auto boolOrTypeReturnType = typeTable->getType("", "Boolean");
  auto boolOrTypeParamType = typeTable->getType("", "Boolean");
  paramTypes = {boolOrTypeParamType};
  auto boolOrType = std::make_shared<TypeFunc>(boolOrTypeReturnType, paramTypes);
  auto boolOrParamDecl = std::make_shared<ParameterDecl>("x", boolOrTypeParamType);
  params = {boolOrParamDecl};
  auto boolOrDecl = std::make_shared<MethodDecl>("Or", boolOrType, params, true);
  boolOrDecl->isBuiltin = true;
  current_scope->addSymbol("Or", boolOrDecl);
  booleanMethods.push_back(boolOrDecl);
  booleanMethodTypes.push_back(boolOrType);

  // - Not (Unary)
  auto boolNotTypeReturnType = typeTable->getType("", "Boolean");
  auto boolNotType = std::make_shared<TypeFunc>(boolNotTypeReturnType);
  params = {};
  auto boolNotDecl = std::make_shared<MethodDecl>("Not", boolNotType, params, true);
  boolNotDecl->isBuiltin = true;
  current_scope->addSymbol("Not", boolNotDecl);
  booleanMethods.push_back(boolNotDecl);
  booleanMethodTypes.push_back(boolNotType);

  // - Equal
  auto boolEqualTypeReturnType = typeTable->getType("", "Boolean");
  auto boolEqualTypeParamType = typeTable->getType("", "Boolean");
  paramTypes = {boolEqualTypeParamType};
  auto boolEqualType = std::make_shared<TypeFunc>(boolEqualTypeReturnType, paramTypes);
  auto boolEqualParamDecl = std::make_shared<ParameterDecl>("x", boolEqualTypeParamType);
  params = {boolEqualParamDecl};
  auto boolEqualDecl = std::make_shared<MethodDecl>("Equal", boolEqualType, params, true);
  boolEqualDecl->isBuiltin = true;
  current_scope->addSymbol("Equal", boolEqualDecl);
  booleanMethods.push_back(boolEqualDecl);
  booleanMethodTypes.push_back(boolEqualType);

  exitScope();

  // - Boolean class declaration
  auto booleanClassType = std::make_shared<TypeClass>("Boolean", std::vector<std::shared_ptr<Type>>(), booleanMethodTypes);
  auto booleanDecl = std::make_shared<ClassDecl>("Boolean", booleanClassType, std::vector<std::shared_ptr<FieldDecl>>(), booleanMethods);
  current_scope->addSymbol("Boolean", booleanDecl);
  exitScope();
  //============================================
  // ... other built-in functions ...
}