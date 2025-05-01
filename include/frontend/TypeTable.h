#ifndef TYPE_TABLE_H
#define TYPE_TABLE_H

#include "types/Types.h"

#include <algorithm>
#include <bits/ranges_algo.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

inline std::string str_tolower(std::string s) {
  std::ranges::transform(
      s, s.begin(), [](unsigned char c) { return std::tolower(c); } // correct
  );
  return s;
}

class TypeTable {
public:
  std::unordered_map<std::string, std::shared_ptr<Type>> types;

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
    addType(name,
            std::make_shared<TypeFunc>(std::move(returnType), std::move(args)));
    return true;
  }

  void addType(const std::string &name, std::shared_ptr<Type> type) {
    types[name] = std::move(type);
  }

  std::shared_ptr<Type> getType(const std::string &name) {
    auto it = types.find(str_tolower(name));
    return ((it != types.end()) ? it->second : nullptr);
  }

  bool exists(const std::string &name) const { return types.contains(name); }

  // ~TypeTable() = default;
  // ~TypeTable() {
  //   for (auto it = types.begin(); it != types.end(); ++it) {
  //     it->second.reset();
  //   }
  // };
};

class GlobalTypeTable {
public:
  // GlobalTypeTable() {}

  // Инициализация встроенных типов
  void initBuiltinTypes() {
    builtinTypes.addType("byte", std::make_shared<TypeByte>());
    // builtinTypes.addType("access", std::make_shared<TypeAccess>());
    // builtinTypes.addType("Integer", std::make_shared<TypeInt>());
    // builtinTypes.addType("Int16", std::make_shared<TypeInt16>());
    // builtinTypes.addType("Int64", std::make_shared<TypeInt64>());
    // builtinTypes.addType("Uint16", std::make_shared<TypeUint16>());
    // builtinTypes.addType("Uint32", std::make_shared<TypeUint32>());
    // builtinTypes.addType("Uint64", std::make_shared<TypeUint64>());
    // builtinTypes.addType("Real", std::make_shared<TypeReal>());
    // builtinTypes.addType("Float64", std::make_shared<TypeFloat64>());
    // // builtinTypes.addType("Bool", std::make_shared<TypeBool>());
    // builtinTypes.addType("Boolean", std::make_shared<TypeBool>());
    // builtinTypes.addType("String", std::make_shared<TypeString>());
    // builtinTypes.addType("Array", std::make_shared<TypeArray>());
  }

  std::unordered_map<std::string, TypeTable> types;
  TypeTable builtinTypes;

  void addType(const std::string &moduleName, const std::string &typeName,
               std::shared_ptr<Type> type) {
    auto tName = str_tolower(typeName);
    types[moduleName].addType(tName, type);
  }

  std::shared_ptr<Type> getType(const std::string &moduleName,
                                const std::string &typeName) {
    // first search through builtins
    auto tName = str_tolower(typeName);
    auto it_bins = builtinTypes.getType(tName);
    if (it_bins == nullptr) {
      auto it = types.find(moduleName);
      if (it == types.end()) {
        return nullptr;
      }
      return it->second.getType(tName);
    }

    return it_bins;
  }

  // ~GlobalTypeTable() = default;
};

#endif