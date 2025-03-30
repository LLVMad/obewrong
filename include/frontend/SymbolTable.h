#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "types/Decl.h"
#include "types/Types.h"
#include <memory>
#include <unordered_map>
#include <vector>

struct Scope {
  std::unordered_map<std::string, std::shared_ptr<Decl>> symbols;
  Scope() : symbols() {}
};

// SymbolTable for some declaration
// for example for a class
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

class ModuleSymbolTable {
public:
  ModuleSymbolTable(const std::string &moduleName)
      : moduleName(moduleName), symbolTables(){};

  std::string moduleName;
  std::unordered_map<std::string, SymbolTable> symbolTables;

  /**
   * Adds a declaration to a scope of this module
   * @param decl module scope declaration to add
   */
  void addToModuleScope(Decl *decl);

  /**
   * Adds a declaration to a "local" scope
   * i.e. declaration in a class, etc.
   * @param moduleName module in which the declaration appears
   * @param parentDeclName name of a "parent" scope in which decl appears
   * @param decl local declaration to add
   */
  void addToLocalScope(const std::string &moduleName,
                       const std::string &parentDeclName, Decl *decl);

  std::shared_ptr<Decl> lookup(const std::string &name);
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

#endif