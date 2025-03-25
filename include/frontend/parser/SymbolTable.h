#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "frontend/types/Decl.h"
#include "frontend/types/Types.h"
#include <memory>
#include <unordered_map>
#include <vector>

class SymbolTable {
  struct Scope {
    std::unordered_map<std::string, std::shared_ptr<Decl>> symbols;
  };

  std::vector<std::shared_ptr<Scope>> scopes;

public:
  SymbolTable() {
    enterScope(); // Инициализация глобальной области
  }

  void enterScope() { scopes.push_back(std::shared_ptr<Scope>()); }

  void exitScope() {
    if (!scopes.empty()) {
      scopes.pop_back();
    }
  }

  bool addSymbol(const std::string &name, std::shared_ptr<Decl> decl) {
    if (scopes.empty() || scopes.back()->symbols.contains(name)) {
      return false;
    }
    scopes.back()->symbols[name] = std::move(decl);
    return true;
  }

  std::shared_ptr<Decl> lookup(const std::string &name) {
    // Обход областей видимости от текущей к глобальной
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
      auto &scope = *it;
      auto found = scope->symbols.find(name);
      if (found != scope->symbols.end()) {
        return std::shared_ptr<Decl>(found->second.get());
      }
    }
    return nullptr;
  }

  // when we encounter this it is in some method scope
  // therefore we need to go 2 scopes "above" to find its class
  // std::shared_ptr<Decl> findClassForThis() {
  //
  // }

  bool isGlobalScope() const { return scopes.size() == 1; }
};

#endif