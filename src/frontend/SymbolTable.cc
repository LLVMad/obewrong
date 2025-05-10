#include "frontend/SymbolTable.h"

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

  enterScope(SCOPE_CLASS, "Integer");
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

  exitScope();
  exitScope();
  //========================================
  // ... other built-in functions ...
}

/*
SymbolTable::SymbolTable() { scopes["Global"] = std::make_shared<Scope>(); }

bool SymbolTable::addSymbol(const std::string &scopeParent,
                            const std::string &name,
                            std::shared_ptr<Decl> decl) {
  // if (scopes.empty() || scopes.back()->symbols.contains(name)) {
  //   return false;
  // }
  // auto scope = scopes[scopeParent];
  if (scopes[scopeParent] == nullptr)
    scopes[scopeParent] = std::make_shared<Scope>();
  scopes[scopeParent]->symbols[name] = decl;
  // scopes.back()->symbols[name] = decl;
  return true;
}

std::shared_ptr<Decl> SymbolTable::lookup(const std::string &scopeParent,
                                          const std::string &name) {
  if (scopes[scopeParent] == nullptr)
    scopes[scopeParent] = std::make_shared<Scope>();
  ;
  auto decl = scopes[scopeParent]->symbols[name];
  if (!decl)
    return nullptr;
  return decl;
}

void GlobalSymbolTable::addToGlobalScope(const std::string &moduleName,
                                         const std::string &parentDeclName,
                                         std::shared_ptr<Decl> decl) {
  moduleSymbolTables[moduleName].addSymbol(parentDeclName, decl->name, decl);
}

std::shared_ptr<Decl> GlobalSymbolTable::lookup(std::string moduleName,
                                                std::string parentScope,
                                                const std::string &name) {
  auto modScope = moduleSymbolTables[moduleName];
  auto decl = modScope.lookup(parentScope, name);
  if (decl)
    return decl;
  // search global
  auto decl_glob = modScope.lookup("Global", name);
  if (decl_glob)
    return decl_glob;
  return nullptr;
}

void GlobalSymbolTable::initBuiltinFuncs(
    const std::shared_ptr<GlobalTypeTable> &typeTable) {

  // For integer
  // - Plus
  auto intPlusTypeReturnType = typeTable->getType("", "Integer");
  auto intPlusTypeParamType = typeTable->getType("", "Integer");
  std::vector<std::shared_ptr<Type>> paramTypes = {intPlusTypeParamType};
  auto intPlusType =
      std::make_shared<TypeFunc>(intPlusTypeReturnType, paramTypes);
  // auto intPlusBody = std::make_shared<Block>(std::vec)
  auto intPlusParamDecl =
      std::make_shared<ParameterDecl>("x", intPlusTypeParamType);
  std::vector<std::shared_ptr<Decl>> params = {intPlusParamDecl};
  auto intPlusDecl =
      std::make_shared<MethodDecl>("Plus", intPlusType, params, true);

  builtinSymbols.addSymbol("Global", "Plus", intPlusDecl);
}

void GlobalSymbolTable::copyBuiltinsToModule(const std::string &moduleName) {
  auto globalScope = builtinSymbols.scopes["Global"];
  for (const auto &decl : globalScope->symbols) {
    moduleSymbolTables[moduleName].addSymbol("Global", decl.second->name,
                                             decl.second);
  }
}
*/