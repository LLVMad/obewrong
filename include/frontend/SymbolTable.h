#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "frontend/types/Types.h"
#include "frontend/parser/Entity.h"
#include <llvm/IR/Instructions.h>
#include <memory>
#include <stack>
#include <unordered_map>
#include <vector>


class SymbolTable {
public:
  SymbolTable() {
    global_scope = std::make_shared<Scope<Entity>>(
        SCOPE_GLOBAL, "Global", std::weak_ptr<Scope<Entity>>());
    current_scope = global_scope;
  }

  std::shared_ptr<Scope<Entity>> enterScope(ScopeKind kind,
                                            const std::string &name) {
    current_scope = current_scope->createChild(kind, name);
    return current_scope;
  }

  // copyying from module to module
  // => globalScope based
  // @FIXME check if already present
  void copySymbolFromModulesToCurrent(const std::string &from,
                                      const std::string &to) {

    std::unordered_map<std::string, SymbolInfo<Entity>> symbolsToCopy;
    std::vector<std::shared_ptr<Scope<Entity>>> scopeToCopy;
    for (auto &scope : global_scope->getChildren()) {
      if (scope->getName() == from) {
        symbolsToCopy = scope->getSymbols();
        scopeToCopy = scope->getChildren();
        break;
      }
    }

    for (auto &scope : global_scope->getChildren()) {
      if (scope->getName() == to) {
        for (auto &decl : symbolsToCopy) {
          scope->addSymbol(decl.first, decl.second.decl);
        }
        for (auto &scopeCopy : scopeToCopy) {
          scopeCopy->external = true;
          scope->addChild(scopeCopy);
        }

        return;
      }
    }

    throw std::runtime_error(
        "SymbolTable::copySymbolFromModulesToCurrent: No such symbol");
  }

  // copying from scopd to scope
  // sc -> scope in which exists FROM and TO scopes
  // used for inheritence
  void copySymbolsAndChildren(std::shared_ptr<Scope<Entity>> &sc,
                              const std::string &from, const std::string &to,
                              bool doesInherit = false) {
    std::unordered_map<std::string, SymbolInfo<Entity>> symbolsToCopy;
    std::vector<std::shared_ptr<Scope<Entity>>> scopeToCopy;
    for (auto scope : sc->getChildren()) {
      if (scope->getName() == from) {
        symbolsToCopy = scope->getSymbols();
        scopeToCopy = scope->getChildren();
        break;
      }
    }

    for (auto &scope : sc->getChildren()) {
      if (scope->getName() == to) {
        for (auto &decl : symbolsToCopy) {
          if (decl.second.decl->getKind() != E_Constructor_Decl)
            scope->addSymbol(decl.first, decl.second.decl);
        }
        for (auto &scopeCopy : scopeToCopy) {
          if (scopeCopy->getKind() == SCOPE_METHOD /*@FIXME we can do better than that */ ) {
            auto _scopeCopy = std::make_shared<Scope<Entity>>(*scopeCopy);
            _scopeCopy->external = true; // @FIXME marks original as external too
            scope->addChild(_scopeCopy);
          }
        }

        return;
      }
    }

    throw std::runtime_error(
        "SymbolTable::copySymbolsAndChildren: No such symbol");
  }

  void exitScope() {
    if (auto parent = current_scope->getParent().lock()) {
      current_scope = parent;
    }
  }

  void initBuiltinFunctions(const std::shared_ptr<GlobalTypeTable> &typeTable);

  // return scope of type SCOPE_MODULE for
  // a nested scope inside that module
  std::shared_ptr<Scope<Entity>>
  getModuleScope(std::shared_ptr<Scope<Entity>> sc) {
    if (auto parent = sc->getParent().lock()) {
      if (parent->getKind() == SCOPE_MODULE)
        return parent;
      if (parent->getName() == "Global")
        return nullptr;
      return getModuleScope(parent);
    }
  }

  std::shared_ptr<Scope<Entity>> getCurrentScope() const {
    return current_scope;
  }

  std::shared_ptr<Scope<Entity>>& getCurrentScopeCopy() {
    return current_scope;
  }

  std::shared_ptr<Scope<Entity>> getGlobalScope() const { return global_scope; }

private:
  std::shared_ptr<Scope<Entity>>
      global_scope; // everything is stored in this pointer
  std::shared_ptr<Scope<Entity>> current_scope;
};

/* @note gavno snizu
// SymbolTable for a module
// @NOTE not global
class SymbolTable {
public:
  std::unordered_map<std::string, std::shared_ptr<Scope<Entity>>> scopes;

  SymbolTable();

  // void enterScope();
  //
  // void exitScope();

  bool addSymbol(const std::string &scopeParent, const std::string &name,
                 std::shared_ptr<Decl> decl);

  std::shared_ptr<Decl> lookup(const std::string &scopeParent,
                               const std::string &name);

  // bool isGlobalScope() const;
};

class GlobalSymbolTable {
public:
  GlobalSymbolTable() : moduleSymbolTables() {}

  std::unordered_map<std::string, SymbolTable> moduleSymbolTables;
  SymbolTable builtinSymbols;

  void initBuiltinFuncs(const std::shared_ptr<GlobalTypeTable> &typeTable);

  void addToGlobalScope(const std::string &moduleName,
                        const std::string &parentDeclName,
                        std::shared_ptr<Decl> decl);

  std::shared_ptr<Decl> lookup(std::string moduleName, std::string parentScope,
                               const std::string &name);

  void copyBuiltinsToModule(const std::string &moduleName);
};
*/
#endif