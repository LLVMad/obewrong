#include "frontend/SymbolTable.h"

SymbolTable::SymbolTable() {
  enterScope(); // Инициализация глобальной области
}

void SymbolTable::enterScope() { scopes.push_back(new Scope()); }

void SymbolTable::exitScope() {
  if (!scopes.empty()) {
    scopes.pop_back();
  }
}

bool SymbolTable::addSymbol(const std::string &name, Decl* decl) {
  if (scopes.empty() || scopes.back()->symbols.contains(name)) {
    return false;
  }
  scopes.back()->symbols[name] = decl;
  return true;
}

Decl* SymbolTable::lookup(const std::string &name) {
  // Обход областей видимости от текущей к глобальной
  for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
    auto &scope = *it;
    auto found = scope->symbols.find(name);
    if (found != scope->symbols.end()) {
      return found->second;
    }
  }
  return {};
}

bool SymbolTable::isGlobalScope() const { return scopes.size() == 1; }

// void ModuleSymbolTable::addToModuleScope(Decl* decl) {
//   this->symbolTables[moduleName].addSymbol(decl->name, decl);
// }

// void ModuleSymbolTable::addToLocalScope(const std::string &moduleName, const
// std::string &parentDeclName, Decl* decl){
//
// }

void GlobalSymbolTable::addToGlobalScope(
    const std::string &moduleName,
    const std::string &parentDeclName,
    Decl* decl
) {
  auto &moduleSymTab = moduleSymbolTables.try_emplace(moduleName, moduleName).first->second;
  moduleSymTab.symbolTables[parentDeclName].addSymbol(decl->name, decl);
}

Decl* GlobalSymbolTable::lookup(
    std::string moduleName,
    std::string parentScope,
    const std::string &name
) {
  auto moduleIt = moduleSymbolTables.find(moduleName);
  if (moduleIt == moduleSymbolTables.end()) return nullptr; // Module not found

  auto &symTab = moduleIt->second.symbolTables[parentScope];
  return symTab.lookup(name);
}

