#ifndef OBW_TYPES_H
#define OBW_TYPES_H

#include <string>
#include <memory>
#include <utility>
#include <variant>
#include <vector>

enum TypeKind {
  TYPE_INT,
  TYPE_FLOAT,
  TYPE_STRING,
  TYPE_ARRAY,
  TYPE_CLASS,
  TYPE_LIST,
  TYPE_FUNC,
  TYPE_BOOL,
};

class Type {
public:
  TypeKind kind;
  std::string name;
  virtual ~Type() = default;

  explicit Type(TypeKind kind, std::string  name) : kind(kind), name(std::move(name)) {}
};

class TypeBuiltin : public Type {
public:
  // @TODO add max/min and epsilon
  uint8_t bitsize;

  TypeBuiltin(TypeKind kind, const std::string &name, uint8_t bitsize)
    : Type(kind, name), bitsize(bitsize) {}
};

class TypeArray : public Type {
public:
  std::unique_ptr<Type> el_type;
  uint32_t size;

  TypeArray(TypeKind kind, const std::string &name, uint32_t size, std::unique_ptr<Type> el_type)
    : Type(kind, name), el_type(std::move(el_type)), size(size) {}
};

class TypeFunc : public Type {
public:
  // store just the signature
  // so just return type and arg types
  std::unique_ptr<Type> return_type;
  std::vector<std::unique_ptr<Type>> args;

  TypeFunc(TypeKind kind, const std::string &name, std::unique_ptr<Type> return_type, std::vector<std::unique_ptr<Type>> args)
    : Type(kind, name), return_type(std::move(return_type)), args(std::move(args)) {}
};

class TypeClass : public Type {
public:
//  std::string name;
  // @TODO add base class info
  std::vector<std::unique_ptr<Type>> fields_types;
  std::vector<std::unique_ptr<TypeFunc>> methods_types;

  TypeClass(TypeKind kind, const std::string &name, std::vector<std::unique_ptr<Type>> fields_types, std::vector<std::unique_ptr<TypeFunc>> methods_types)
    : Type(kind, name), fields_types(std::move(fields_types)), methods_types(std::move(methods_types)) {}
};

class TypeList : public Type {
public:
  std::unique_ptr<Type> el_type;

  TypeList(TypeKind kind, const std::string &name, std::unique_ptr<Type> el_type)
    : Type(kind, name), el_type(std::move(el_type)) {}
};

#endif
