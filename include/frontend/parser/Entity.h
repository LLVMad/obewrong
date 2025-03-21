#ifndef OBW_EINFO_H
#define OBW_EINFO_H

#include <memory>
#include <ostream>
#include <unordered_map>
#include "util/Logger.h"

#define DEBUG 1 


#include "frontend/types/Types.h"

struct Loc {
  size_t line;
  size_t col;
  std::string file;
};

/**
* Describes different kind of
* entitys available in
* Obewrong language
*/
enum Ekind {
  // Declarations
  E_Class_Decl,
  E_Constructor_Decl,
  E_Variable_Decl,
  E_Parameter_Decl,
  E_Field_Decl,
  E_Function_Decl,
  E_Array_Decl,
  E_List_Decl,

  // Type-related entities
  E_Integer_Literal,
  E_Integer_Type,
  E_Real_Literal,
  E_Real_Type,
  E_String_Literal,
  E_String_Type,
  E_Boolean_Literal,
  E_Boolean_Type,
  E_Function,
  E_Chained_Functions, // compound expressions, a.Plus(2).Minus(1)...
  E_Function_Type,
  E_Class_Name,

  // Special entities
  E_This, // current instance of object
  E_Library_Class,
  E_Generic_Class,

  // Statement-related entities
  E_Assignment,
  E_While_Loop,
  E_If_Statement,
  E_Return_Statement,
  E_Block,

  E_Var_Reference,
  E_Field_Reference,

  // fake entities, that just used to link meaningfully ones
  E_Expression,
  E_Arguments, // arguments list that is passed to a function
};

/**
* Foundation for obewrong entities
*/
class Entity {
public:
  virtual ~Entity() = default;
  explicit Entity(Ekind kind);

  Ekind getKind() const;
  Loc getLoc() const;

  virtual std::unique_ptr<Type> resolveType() const;
  virtual bool validate();

  // ======== NODE LINKS ========
  std::unique_ptr<Entity> scope;
  std::vector<std::unique_ptr<Entity>> children;

  // Таблица символов для текущей области видимости
  std::unordered_map<std::string, std::unique_ptr<Type>> symbolTable;

  void printSymbolTable() const {
    LOG_NOARGS("Symbol Table: ");
    LOG_NOARGS("-------------");
    for (const auto& pair : symbolTable) {
        const std::string& name = pair.first;
        const Type* type = pair.second.get(); // Получаем сырой указатель
        LOG("Name: %s, Type: %s", name.c_str(), type->name.c_str());
    }
    LOG_NOARGS("-------------");
  }

  // Добавляем символ в таблицу
  void addSymbol(const std::string& name, std::unique_ptr<Type> type) {
    if (!type) {
      throw std::runtime_error("Type is nullptr");
    }
    if (symbolTable.find(name) != symbolTable.end()) {
      throw std::runtime_error("Duplicate symbol: " + name);
    }
    printSymbolTable();
    symbolTable[name] = std::move(type);
    printSymbolTable();
    LOG_NOARGS("Add symbol to symbolTable is success");
  }

  // Ищем символ в таблице
  std::unique_ptr<Type> resolveSymbol(const std::string& name) const {
    auto it = symbolTable.find(name);
    if (it != symbolTable.end()) {
      LOG("In simbolTable found simbol: %s", name.c_str());
      return std::make_unique<Type>(*it->second);
    }
    LOG_ERR("In simbolTable coundn't find simbol: %s", name.c_str());
    printSymbolTable();
    return nullptr;
  }

protected:
  Ekind kind;
  Loc location;
};

/**
* @TODO construct classes for each entitys
* i.e. `class ClassDeclaration : Entity`
* this will make it possible to return `Entity*` from some
* `parse*` functions like for example from parseParameters()
*/



#endif
