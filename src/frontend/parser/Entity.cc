#include "frontend/parser/Entity.h"

Entity::Entity(Ekind kind) : kind(kind) {}

std::unique_ptr<Type> Entity::resolveType() {
    return nullptr; // Заглушка
}

bool Entity::validate() {
    return false; // Заглушка
}