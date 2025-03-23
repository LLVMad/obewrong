#ifndef OBW_TYPES_H
#define OBW_TYPES_H

#include <cfloat>
#include <climits>
#include <math.h>
#include <memory>
#include <string>
#include <utility>
#include <variant>
#include <vector>

enum TypeKind {
  TYPE_UNKNOWN = -1,
  TYPE_INT,
  TYPE_FLOAT,
  TYPE_STRING,
  TYPE_ARRAY,
  TYPE_CLASS,
  TYPE_LIST,
  TYPE_FUNC, // important to distinguish, this not a type as in types in a
             // reference manual, this a type of an ENTITY, function itself has
             // a signature
  TYPE_BOOL,
  TYPE_GENERIC,
};

class Type {
public:
  TypeKind kind;
  std::string name;
  virtual ~Type() = default;

  explicit Type(TypeKind kind, std::string name)
      : kind(kind), name(std::move(name)) {}
};

class TypeBuiltin : public Type {
public:
  // @TODO add max/min and epsilon
  uint8_t bitsize; // can be added

  TypeBuiltin(TypeKind kind, const std::string &name, uint8_t bitsize)
      : Type(kind, name), bitsize(bitsize) {}
};

class TypeInt : public TypeBuiltin {
public:
  int max = INT_MAX;
  int min = INT_MIN;

  /*
   * !!!!! @TODO @IMPORTANT !!!!!!
   *
   * the reference implementation of
   * builtin types
   * might as well go here
   * so all methods, etc.
   */

  TypeInt() : TypeBuiltin(TYPE_INT, "Integer", 32) {}
};

class TypeReal : public TypeBuiltin {
public:
  float max = FLT_MAX;
  float min = FLT_MIN;
  float eps = FLT_EPSILON;

  TypeReal() : TypeBuiltin(TYPE_FLOAT, "Real", 32) {}
};

class TypeString : public TypeBuiltin {
public:
  // 8 bits -> 1 byte for '\0'
  TypeString() : TypeBuiltin(TYPE_STRING, "String", 8) {}

  TypeString(size_t size) : TypeBuiltin(TYPE_STRING, "String", size) {}
};

class TypeBool : public TypeBuiltin {
public:
  TypeBool() : TypeBuiltin(TYPE_BOOL, "Bool", 1) {}
};

class TypeArray : public Type {
public:
  std::unique_ptr<Type> el_type;
  uint32_t size;

  TypeArray(uint32_t size, std::unique_ptr<Type> el_type)
      : Type(TYPE_ARRAY, "Array"), el_type(std::move(el_type)), size(size) {}

  // for when we dont know the parameters yet
  TypeArray() : Type(TYPE_ARRAY, "Array"), el_type(nullptr), size(0) {}
};

// signature of a function
// is split in two field, so:
// args[0] -> args[1] -> ... -> return_type
class TypeFunc : public Type {
public:
  // store just the signature
  // so just return type and arg types
  std::unique_ptr<Type> return_type;
  std::vector<std::unique_ptr<Type>> args;

  TypeFunc(std::unique_ptr<Type> return_type,
           std::vector<std::unique_ptr<Type>> args)
      : Type(TYPE_FUNC, "Function"), return_type(std::move(return_type)),
        args(std::move(args)) {}
};

// class "signature"
// is a:
// [base_class] -> field_types[0] -> field_types[1] -> ... -> method_types[0] ->
// method_types[1] -> ...
class TypeClass : public Type {
public:
  //  std::string name;
  // @TODO add base class info
  std::vector<std::unique_ptr<TypeClass>> base_class;
  std::vector<std::unique_ptr<Type>> fields_types;
  std::vector<std::unique_ptr<TypeFunc>> methods_types;

  TypeClass(const std::string &name,
            std::vector<std::unique_ptr<Type>> fields_types,
            std::vector<std::unique_ptr<TypeFunc>> methods_types)
      : Type(TYPE_CLASS, name), fields_types(std::move(fields_types)),
        methods_types(std::move(methods_types)) {}
};

class TypeList : public Type {
public:
  std::unique_ptr<Type> el_type;

  TypeList(std::unique_ptr<Type> el_type)
      : Type(TYPE_LIST, "List"), el_type(std::move(el_type)) {}

  TypeList() : Type(TYPE_LIST, "List"), el_type(nullptr) {}
};

#endif
