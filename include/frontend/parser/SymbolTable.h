#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "frontend/types/Decl.h"
#include "frontend/types/Types.h"
#include <memory>
#include <unordered_map>
#include <vector>

class SymbolTable {
  struct Scope {
    std::unordered_map<std::string, std::unique_ptr<Decl>> symbols;
  };

  std::vector<std::unique_ptr<Scope>> scopes;

public:
  SymbolTable() {
    enterScope(); // Инициализация глобальной области
  }

  void enterScope() { scopes.push_back(std::make_unique<Scope>()); }

  void exitScope() {
    if (!scopes.empty()) {
      scopes.pop_back();
    }
  }

  bool addSymbol(const std::string &name, std::unique_ptr<Decl> decl) {
    if (scopes.empty() || scopes.back()->symbols.count(name)) {
      return false;
    }
    scopes.back()->symbols[name] = std::move(decl);
    return true;
  }

  Decl *lookup(const std::string &name) {
    // Обход областей видимости от текущей к глобальной
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
      auto &scope = *it;
      auto found = scope->symbols.find(name);
      if (found != scope->symbols.end()) {
        return found->second.get();
      }
    }
    return nullptr;
  }

  bool isGlobalScope() const { return scopes.size() == 1; }
};

#endif