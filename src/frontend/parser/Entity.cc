#include "frontend/parser/Entity.h"

Entity::Entity(Ekind kind) : kind(kind) {}

std::unique_ptr<Type> Entity::resolveType() const {
    return std::make_unique<Type>(TYPE_UNKNOWN, "Unknown");
}

bool Entity::validate() {
    for (const auto& child : children) {
        if (!child->validate()) {
            return false;
        }
    }
    return true;
}

Ekind Entity::getKind() const {
    return kind;
}