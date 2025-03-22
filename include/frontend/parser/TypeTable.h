#ifndef TYPE_TABLE_H
#define TYPE_TABLE_H

#include <unordered_map>
#include <memory>
#include <string>
#include <vector>
#include "frontend/types/Types.h"

class TypeTable {
    std::unordered_map<std::string, std::unique_ptr<Type>> types;

public:
    // Инициализация встроенных типов
    void initBuiltinTypes() {
        addType("Integer", std::make_unique<TypeInt>());
        addType("Real", std::make_unique<TypeReal>());
        addType("Bool", std::make_unique<TypeBool>());
        addType("String", std::make_unique<TypeString>());
    }

    // Добавление пользовательского типа
    bool addClassType(const std::string& className) {
        if (exists(className)) {
            return false;
        }
        addType(className, std::make_unique<TypeClass>(className, std::vector<std::unique_ptr<Type>>(), std::vector<std::unique_ptr<TypeFunc>>()));
        return true;
    }

    bool addArrayType(const std::string& name, std::unique_ptr<Type> elementType, uint32_t size) {
        if (exists(name)) {
            return false;
        }
        addType(name, std::make_unique<TypeArray>(size, std::move(elementType)));
        return true;
    }

    bool addListType(const std::string& name, std::unique_ptr<Type> elementType) {
        if (exists(name)) {
            return false;
        }
        addType(name, std::make_unique<TypeList>(std::move(elementType)));
        return true;
    }

    bool addFuncType(const std::string& name, std::unique_ptr<Type> returnType, std::vector<std::unique_ptr<Type>> args) {
        if (exists(name)) {
            return false;
        }
        addType(name, std::make_unique<TypeFunc>(std::move(returnType), std::move(args)));
        return true;
    }

    void addType(const std::string& name, std::unique_ptr<Type> type) {
        types[name] = std::move(type);
    }

    Type* getType(const std::string& name) {
        auto it = types.find(name);
        return (it != types.end()) ? it->second.get() : nullptr;
    }

    bool exists(const std::string& name) const {
        return types.count(name);
    }
};

#endif