#include "frontend/SymbolTable.h"

SymbolTable::SymbolTable() {
  scopes["Global"] = std::make_shared<Scope>();
  // this->scopes
  // enterScope(); // Инициализация глобальной области
}

// void SymbolTable::enterScope() { scopes.push_back(new Scope()); }
//
// void SymbolTable::exitScope() {
//   if (!scopes.empty()) {
//     scopes.pop_back();
//   }
// }

bool SymbolTable::addSymbol(const std::string &scopeParent, const std::string &name, std::shared_ptr<Decl> decl) {
  // if (scopes.empty() || scopes.back()->symbols.contains(name)) {
  //   return false;
  // }
  // auto scope = scopes[scopeParent];
  if (scopes[scopeParent] == nullptr) scopes[scopeParent] = std::make_shared<Scope>();
  scopes[scopeParent]->symbols[name] = decl;
  // scopes.back()->symbols[name] = decl;
  return true;
}

std::shared_ptr<Decl> SymbolTable::lookup(const std::string &scopeParent, const std::string &name) {
  // Обход областей видимости от текущей к глобальной
  // for (auto it = scopes[scopeParent].begin(); it != scopes[scopeParent]; ++it) {
  //   auto &scope = *it;
  //   auto found = scopesymbols.find(name);
  //   if (found != scope->symbols.end()) {
  //     return found->second;
  //   }
  // }
  auto scope = scopes[scopeParent];
  auto decl = scope->symbols[name];
  if (!decl) return nullptr;
  return decl;
}

// bool SymbolTable::isGlobalScope() const { return scopes.size() == 1; }

// void ModuleSymbolTable::addToModuleScope(std::shared_ptr<Decl> decl) {
//   this->symbolTables[moduleName].addSymbol(decl->name, decl);
// }

// void ModuleSymbolTable::addToLocalScope(const std::string &moduleName, const
// std::string &parentDeclName, std::shared_ptr<Decl> decl){
//
// }

void GlobalSymbolTable::addToGlobalScope(
    const std::string &moduleName,
    const std::string &parentDeclName,
    std::shared_ptr<Decl> decl
) {
  // auto &moduleSymTab = moduleSymbolTables.try_emplace(moduleName, moduleName).first->second;
  // moduleSymTab.scopes[parentDeclName].addSymbol(decl->name, decl);
  // auto modScope = moduleSymbolTables[moduleName];
  moduleSymbolTables[moduleName].addSymbol(parentDeclName, decl->name, decl);
}

std::shared_ptr<Decl> GlobalSymbolTable::lookup(
    std::string moduleName,
    std::string parentScope,
    const std::string &name
) {
  auto modScope = moduleSymbolTables[moduleName];
  auto decl = modScope.lookup(parentScope, name);
  if (decl) return decl;
  // search global
  auto decl_glob = modScope.lookup("Global", name);
  if (decl_glob) return decl_glob;
  return nullptr;
  // auto moduleIt = moduleSymbolTables.find(moduleName);
  // if (moduleIt == moduleSymbolTables.end()) return nullptr; // Module not found
  //
  // auto &symTab = moduleIt->second.symbolTables[parentScope];
  // auto sym = symTab.lookup(name);
  //
  // if (!sym) {
  //   auto builtIn = builtinSymbols.lookup(name);
  //   if (builtIn) return builtIn;
  //   return nullptr;
  // }
  //
  // return sym;
}

void GlobalSymbolTable::initBuiltinFuncs(const std::shared_ptr<GlobalTypeTable> &typeTable) {

  // For integer
  // - Plus
  auto intPlusTypeReturnType = typeTable->getType("", "Integer");
  auto intPlusTypeParamType = typeTable->getType("", "Integer");
  std::vector<std::shared_ptr<Type>> paramTypes = { intPlusTypeParamType };
  auto intPlusType = std::make_shared<TypeFunc>(intPlusTypeReturnType, paramTypes);
  // auto intPlusBody = std::make_shared<Block>(std::vec)
  auto intPlusParamDecl = std::make_shared<ParameterDecl>("x", intPlusTypeParamType);
  std::vector<std::shared_ptr<Decl>> params = { intPlusParamDecl };
  auto intPlusDecl = std::make_shared<MethodDecl>(
    "Plus",
    intPlusType,
    params,
    true
  );

  builtinSymbols.addSymbol("Global", "Plus", intPlusDecl);
}

void GlobalSymbolTable::copyBuiltinsToModule(const std::string& moduleName) {
  auto globalScope = builtinSymbols.scopes["Global"];
  for (const auto &decl : globalScope->symbols) {
    moduleSymbolTables[moduleName].addSymbol("Global", decl.second->name, decl.second);
  }
}
