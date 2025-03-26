#ifndef TYPE_TABLE_H
#define TYPE_TABLE_H

#include "types/Types.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class TypeTable {
  std::unordered_map<std::string, std::shared_ptr<Type>> types;

public:
  // Инициализация встроенных типов
  void initBuiltinTypes() {
    addType("Integer", std::make_shared<TypeInt>());
    addType("Int16", std::make_shared<TypeInt16>());
    addType("Int64", std::make_shared<TypeInt64>());
    addType("Uint16", std::make_shared<TypeUint16>());
    addType("Uint32", std::make_shared<TypeUint32>());
    addType("Uint64", std::make_shared<TypeUint64>());
    addType("Real", std::make_shared<TypeReal>());
    addType("Float64", std::make_shared<TypeFloat64>());
    addType("Bool", std::make_shared<TypeBool>());
    addType("String", std::make_shared<TypeString>());
  }

  // Добавление пользовательского типа
  bool addClassType(const std::string &className) {
    if (exists(className)) {
      return false;
    }
    addType(className, std::make_shared<TypeClass>(
                           className, std::vector<std::shared_ptr<Type>>(),
                           std::vector<std::shared_ptr<TypeFunc>>()));
    return true;
  }

  bool addBaseClass(const std::string &className) {
    (void)className;
    return true; // @TODO
  }

  bool addArrayType(const std::string &name, std::shared_ptr<Type> elementType,
                    uint32_t size) {
    if (exists(name)) {
      return false;
    }
    addType(name, std::make_shared<TypeArray>(size, std::move(elementType)));
    return true;
  }

  bool addListType(const std::string &name, std::shared_ptr<Type> elementType) {
    if (exists(name)) {
      return false;
    }
    addType(name, std::make_shared<TypeList>(std::move(elementType)));
    return true;
  }

  bool addFuncType(const std::string &name, std::shared_ptr<Type> returnType,
                   std::vector<std::shared_ptr<Type>> args) {
    if (exists(name)) {
      return false;
    }
    addType(name, std::make_shared<TypeFunc>(std::move(returnType), std::move(args)));
    return true;
  }

  void addType(const std::string &name, std::shared_ptr<Type> type) {
    types[name] = std::move(type);
  }

  std::shared_ptr<Type> getType(const std::string &name) {
    auto it = types.find(name);
    return std::shared_ptr<Type>((it != types.end()) ? it->second.get() : nullptr);
  }

  bool exists(const std::string &name) const { return types.contains(name); }
};

class GlobalTypeTable {
public:
  // GlobalTypeTable() {}

  std::unordered_map<std::string, TypeTable> types;

  void addType(const std::string &moduleName, const std::string &typeName, std::shared_ptr<Type> type) {
    types[moduleName].addType(typeName, type);
  }

  std::shared_ptr<Type> getType(const std::string &moduleName, const std::string &typeName) {
    auto it = types.find(moduleName);
    if (it == types.end()) {
      return nullptr;
    }
    return it->second.getType(typeName);
  }
};

#endif