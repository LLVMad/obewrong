#ifndef TYPE_TABLE_H
#define TYPE_TABLE_H

#include <unordered_map>
#include <memory>
#include "frontend/types/Types.h"

class TypeTable {
    std::unordered_map<std::string, std::unique_ptr<Type>> types;
    
public:
    void addType(const std::string& name, std::unique_ptr<Type> type) {
        types[name] = std::move(type);
    }
    
    Type* getType(const std::string& name) {
        auto it = types.find(name);
        return it != types.end() ? it->second.get() : nullptr;
    }
    
    bool exists(const std::string& name) const {
        return types.count(name);
    }
};

#endif