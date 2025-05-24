#ifndef OBW_SCOPE_H
#define OBW_SCOPE_H

#include <llvm/IR/Instructions.h>
#include <memory>
#include <stack>
#include <unordered_map>

enum ScopeKind {
  SCOPE_GLOBAL, // scope of all project (multiple cu's)
  SCOPE_MODULE,
  SCOPE_CLASS,
  SCOPE_METHOD,
  SCOPE_FUNCTION,
  SCOPE_ENUM,
  SCOPE_LOOP,
  SCOPE_MODULE_BUILTIN, // >= 7
  SCOPE_METHOD_BUILTIN,
  SCOPE_CLASS_BUILTIN,
};

template<typename T>
struct SymbolInfo {
  std::shared_ptr<T> decl; // for parsing phase
  llvm::AllocaInst* alloca;   // for codegen
  bool isInitialized;
};

/**
 * Symbol table for a single scope
 * of a class, method or func
 *
 * @template Decl - is here mostly due to dependency issues
 */
template<typename T>
class Scope : public std::enable_shared_from_this<Scope<T>> {
public:
  Scope(ScopeKind kind, const std::string &name, std::weak_ptr<Scope> parent)
      : external(false), kind(kind), name(name), parent(parent), depth(-1) {}

  /**
   * @phase Syntax analysis \n
   * Puts a Declaration info of a certain construct into a map
   * @param name
   * @param decl Declaration info
   */
  template<typename U>
  bool addSymbol(const std::string &name, std::shared_ptr<U> decl) {
    static_assert(std::derived_from<U, T>, "Must be derived from Entity");
    // if (symbols.contains(name))
    //   return false;
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

    // skip builtins
    while (children[depth]->kind >= 7 || children[depth]->external) {
      depth++;
    }

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

  template<typename U = T>
  SymbolInfo<U>* getSymbol(const std::string &name) {
    if (auto it = symbols.find(name); it != symbols.end()) {
      return reinterpret_cast<SymbolInfo<U>*>(&it->second);
    }

    // recursively check parent scopes
    if (auto parent_ptr = parent.lock()) {
      return parent_ptr->template getSymbol<U>(name);
    }

    return nullptr;
  }

  template<typename U = T>
  std::shared_ptr<U> lookup(const std::string &name) {
    static_assert(std::derived_from<U, T>, "Must be derived from Entity");

    if (auto sym = this->getSymbol(name))
      return std::static_pointer_cast<U>(sym->decl);
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

  std::shared_ptr<T> lookupInClass(const std::string &name,
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

  // std::shared_ptr<Scope> getModuleScope
  // std::shared_ptr<T>

  std::shared_ptr<Scope> createChild(ScopeKind kind, std::string name) {
    auto child = std::make_shared<Scope>(kind, name, this->shared_from_this());
    children.push_back(child);
    return child;
  }

  ScopeKind getKind() const { return kind; }
  const std::string &getName() const { return name; }
  // void setName(std::string name) { this->name = name; }
  auto &getChildren() { return children; }
  auto copyChildren() const { return children; }
  std::weak_ptr<Scope> getParent() const { return parent; }
  void setParent(std::shared_ptr<Scope<T>> parent) { this->parent = parent; }
  std::unordered_map<std::string, SymbolInfo<T>> &getSymbols() {
    return symbols;
  }

  void setName(const std::string &name) { this->name = name; }
  void appendToName(const std::string &name) { this->name += name; }

  bool external; // do we need to visit it? if copied to antoher module => true

private:
  ScopeKind kind;
  std::string name;
  std::weak_ptr<Scope> parent;
  std::vector<std::shared_ptr<Scope>> children;
  std::unordered_map<std::string, SymbolInfo<T>> symbols;

  int depth;
};

#endif
