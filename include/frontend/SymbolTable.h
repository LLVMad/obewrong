#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "types/Decl.h"
#include "types/Types.h"
#include <llvm/IR/Instructions.h>
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

struct SymbolInfo {
  std::shared_ptr<Decl> decl; // for parsing phase
  llvm::AllocaInst* alloca;   // for codegen
  bool isInitialized;
};

/**
 * Symbol table for a single scope
 * of a class, method or func
 */
class Scope : public std::enable_shared_from_this<Scope> {
public:
  Scope(ScopeKind kind, const std::string &name, std::weak_ptr<Scope> parent)
      : kind(kind), name(name), parent(parent), depth(-1) {}

  /**
   * @phase Syntax analysis \n
   * Puts a Declaration info of a certain construct into a map
   * @param name
   * @param decl Declaration info
   */
  bool addSymbol(const std::string &name, std::shared_ptr<Decl> decl) {
    if (symbols.contains(name))
      return false;
    symbols[name].decl = decl;
    return true;
  }

  /**
   * @phase Code generation \n
   * Puts an allocation intance of a variable into a map
   * with previous Decl set
   * @param name
   * @param alloca instance of allocation
   */
  bool addSymbol(const std::string &name, llvm::AllocaInst *alloca) {
    if (!symbols.contains(name))
      return false;
    symbols[name].alloca = alloca;
    return true;
  }

  /**
   * @phase Code generation \n
   * Marks a variable as initialized
   * @param name
   */
  bool markInitialized(const std::string &name) {
    if (!symbols.contains(name))
      return false;
    symbols[name].isInitialized = true;
    return true;
  }

  void addChild(std::shared_ptr<Scope> child) {
    children.push_back(std::move(child));
  }

  /**
   * Go into the next children scope
   * @param depth depth of scope relative to the parent
   *              0 - scope itself
   * @return children scope at depth
   */
  std::shared_ptr<Scope> nextScope() {
    depth++;
    if (depth > static_cast<int>(children.size() - 1)) return std::shared_ptr(parent);
    return children[depth];
  }

  /**
   * Go into the prev children scope
   * @param depth depth of scope relative to the parent
   *              0 - parent scope
   * @return children scope at depth
   */
  std::shared_ptr<Scope> prevScope() {
    return std::shared_ptr(parent);
  }

  SymbolInfo* getSymbol(const std::string &name) {
    if (auto it = symbols.find(name); it != symbols.end()) {
      return &it->second;
    }

    // Recursively check parent scopes
    if (auto parent_ptr = parent.lock()) {
      return parent_ptr->getSymbol(name);
    }

    return nullptr;
  }

  std::shared_ptr<Decl> lookup(const std::string &name) {
    if (auto sym = this->getSymbol(name)) return sym->decl;
    return nullptr;
  }

  llvm::AllocaInst* lookupAlloca(const std::string &name) {
    if (auto sym = this->getSymbol(name)) return sym->alloca;
    return nullptr;
  }

  bool isDeclInitialized(const std::string &name) {
    if (auto sym = this->getSymbol(name)) return sym->isInitialized;
    return false;
  }

  std::shared_ptr<Decl> lookupInClass(const std::string &name,
                                      const std::string &className) const {
    // idk need to think about this
    if (this->name == className) {
      if (auto it = symbols.find(name); it != symbols.end()) {
        return it->second.decl;
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
  std::unordered_map<std::string, SymbolInfo> &getSymbols() {
    return symbols;
  }

  void setName(const std::string &name) { this->name = name; }
  void appendToName(const std::string &name) { this->name += name; }

private:
  ScopeKind kind;
  std::string name;
  std::weak_ptr<Scope> parent;
  std::vector<std::shared_ptr<Scope>> children;
  std::unordered_map<std::string, SymbolInfo> symbols;

  int depth;
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

    std::unordered_map<std::string, SymbolInfo> symbolsToCopy;
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
          scope->addSymbol(decl.first, decl.second.decl);
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
    std::unordered_map<std::string, SymbolInfo> symbolsToCopy;
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
          scope->addSymbol(decl.first, decl.second.decl);
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
    return nullptr;
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