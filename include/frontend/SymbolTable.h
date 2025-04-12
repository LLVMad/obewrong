#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "types/Decl.h"
#include "types/Types.h"
#include <memory>
#include <stack>
#include <unordered_map>
#include <vector>

enum ScopeKind {
  SCOPE_GLOBAL, // scope of all project (multiple cu's)
  SCOPE_MODULE,
  SCOPE_CLASS,
  SCOPE_METHOD,
  SCOPE_FUNCTION,
  SCOPE_ENUM,
};

/**
 * Symbol table for a single scope
 * of a class, method or func
 */
class Scope : public std::enable_shared_from_this<Scope> {
public:
  Scope(ScopeKind kind, const std::string &name, std::weak_ptr<Scope> parent)
      : kind(kind), name(name), parent(parent) {}

  bool addSymbol(const std::string &name, std::shared_ptr<Decl> decl) {
    if (symbols.contains(name))
      return false;
    symbols[name] = decl;
    return true;
  }

  void addChild(std::shared_ptr<Scope> child) {
    children.push_back(std::move(child));
  }

  std::shared_ptr<Decl> lookup(const std::string &name) const {
    if (auto it = symbols.find(name); it != symbols.end()) {
      return it->second;
    }

    // Recursively check parent scopes
    if (auto parent_ptr = parent.lock()) {
      // if (
      //
      //   (parent_ptr->kind == ScopeKind::SCOPE_CLASS
      //     || parent_ptr->kind == ScopeKind::SCOPE_METHOD)) {
      //   for (const auto child : parent_ptr->children) {
      //     return child->lookup(name);
      //   }
      // }
      return parent_ptr->lookup(name);
    }

    return nullptr;
  }

  std::shared_ptr<Decl> lookupInClass(const std::string &name,
                                      const std::string &className) const {
    // idk need to think about this
    if (this->name == className) {
      if (auto it = symbols.find(name); it != symbols.end()) {
        return it->second;
      }
    }
    //
    for (const auto &child : children) {
      if (child->kind == SCOPE_CLASS && child->name == className) {
        auto result = child->lookup(name);
        if (result)
          return result;
        return nullptr;
      }
    }
    // for (const auto &sym : symbols) {
    //   if (sym.second->getKind() == E_Class_Decl)
    // }
    return nullptr;
  }

  // std::shared_ptr<Decl>

  std::shared_ptr<Scope> createChild(ScopeKind kind, std::string name) {
    auto child = std::make_shared<Scope>(kind, name, shared_from_this());
    children.push_back(child);
    return child;
  }

  ScopeKind getKind() const { return kind; }
  const std::string &getName() const { return name; }
  auto &getChildren() const { return children; }
  std::weak_ptr<Scope> getParent() const { return parent; }
  std::unordered_map<std::string, std::shared_ptr<Decl>> &getSymbols() {
    return symbols;
  }

  void setName(const std::string &name) { this->name = name; }
  void appendToName(const std::string &name) { this->name += name; }

private:
  ScopeKind kind;
  std::string name;
  std::weak_ptr<Scope> parent;
  std::vector<std::shared_ptr<Scope>> children;
  std::unordered_map<std::string, std::shared_ptr<Decl>> symbols;
};

class SymbolTable {
public:
  SymbolTable() {
    global_scope =
        std::make_shared<Scope>(SCOPE_GLOBAL, "Global", std::weak_ptr<Scope>());
    current_scope = global_scope;
  }

  std::shared_ptr<Scope> enterScope(ScopeKind kind, const std::string &name) {
    current_scope = current_scope->createChild(kind, name);
    return current_scope;
  }

  // copyying from module to module
  // => globalScope based
  void copySymbolFromModulesToCurrent(const std::string &from,
                                      const std::string &to) {

    std::unordered_map<std::string, std::shared_ptr<Decl>> symbolsToCopy;
    std::vector<std::shared_ptr<Scope>> scopeToCopy;
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
          scope->addSymbol(decl.first, decl.second);
        }
        for (auto &scopeCopy : scopeToCopy) {
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
  void copySymbolsAndChildren(std::shared_ptr<Scope> &sc,
                              const std::string &from, const std::string &to) {
    std::unordered_map<std::string, std::shared_ptr<Decl>> symbolsToCopy;
    std::vector<std::shared_ptr<Scope>> scopeToCopy;
    for (auto &scope : sc->getChildren()) {
      if (scope->getName() == from) {
        symbolsToCopy = scope->getSymbols();
        scopeToCopy = scope->getChildren();
        break;
      }
    }

    for (auto &scope : sc->getChildren()) {
      if (scope->getName() == to) {
        for (auto &decl : symbolsToCopy) {
          scope->addSymbol(decl.first, decl.second);
        }
        for (auto &scopeCopy : scopeToCopy) {
          scope->addChild(scopeCopy);
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
  std::shared_ptr<Scope> getModuleScope(std::shared_ptr<Scope> sc) {
    if (auto parent = sc->getParent().lock()) {
      if (parent->getKind() == SCOPE_MODULE)
        return parent;
      if (parent->getName() == "Global")
        return nullptr;
      return getModuleScope(parent);
    }
  }

  std::shared_ptr<Scope> getCurrentScope() const { return current_scope; }
  std::shared_ptr<Scope> getGlobalScope() const { return global_scope; }

private:
  std::shared_ptr<Scope> global_scope; // everything is stored in this pointer
  std::shared_ptr<Scope> current_scope;
};

/* @note gavno snizu
// SymbolTable for a module
// @NOTE not global
class SymbolTable {
public:
  std::unordered_map<std::string, std::shared_ptr<Scope>> scopes;

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