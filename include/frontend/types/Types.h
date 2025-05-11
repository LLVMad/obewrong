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

#include "llvm/IR/Type.h"

#include <llvm/IR/DerivedTypes.h>

enum AccessKind {
  ACC_GENERAL,
  ACC_NOT_NULL,
  ACC_OWN
};

enum TypeKind {
  TYPE_UNKNOWN = -1,
  TYPE_BYTE,
  TYPE_INT,
  TYPE_I16,
  TYPE_I64,
  TYPE_U16,
  TYPE_U64,
  TYPE_U32,
  TYPE_FLOAT,
  TYPE_F64,
  TYPE_STRING,
  TYPE_ARRAY,
  TYPE_CLASS,
  TYPE_LIST,
  TYPE_FUNC, // important to distinguish, this not a type as in types in a
             // reference manual, this a type of an ENTITY, function itself has
             // a signature
  TYPE_BOOL,
  TYPE_GENERIC,
  TYPE_POINTER,
  TYPE_ACCESS,
};

class Type {
public:
  TypeKind kind;
  std::string name;
  virtual ~Type() = default;

  explicit Type(TypeKind kind, std::string name)
      : kind(kind), name(std::move((name))) {}

  virtual llvm::Type *toLLVMType(llvm::LLVMContext &lc) = 0;
};

// class Bit : public Type {
// public:
//   TypeKind kind;
// };

class TypeByte : public Type {
public:
  TypeByte() : Type(TYPE_BYTE, "byte") {}

  llvm::Type *toLLVMType(llvm::LLVMContext &lc) override {
    return llvm::Type::getInt8Ty(lc);
  };
};

class TypeAccess : public Type {
public:
  TypeAccess(const std::shared_ptr<Type> &to)
    : Type(TYPE_ACCESS, "access"), kind(ACC_GENERAL), to(to) {}

  AccessKind kind;
  std::shared_ptr<Type> to;

  llvm::Type *toLLVMType(llvm::LLVMContext &lc) override {
    auto toType = to->toLLVMType(lc);
    return llvm::PointerType::get(toType, 0);
  }
};

class TypeBuiltin : public Type {
public:
  TypeBuiltin(TypeKind kind, const std::string &name, uint8_t bitsize)
      : Type(kind, name), bitsize(bitsize), bytesize(bitsize / 8) {}

  uint8_t bitsize; // can be added
  uint8_t bytesize;

  ~TypeBuiltin() override = default;

  llvm::Type *toLLVMType(llvm::LLVMContext &lc) override {
    (void)lc;
    return nullptr;
  }
};

class TypeInt : public TypeBuiltin {
public:
  int max = INT_MAX;
  int min = INT_MIN;

  TypeInt() : TypeBuiltin(TYPE_INT, "Integer", 32) {}

  llvm::Type *toLLVMType(llvm::LLVMContext &lc) override {
    return llvm::Type::getInt32Ty(lc);
  }

  ~TypeInt() override = default;
};

class TypeInt16 : public TypeBuiltin {
public:
  int16_t max = std::numeric_limits<int16_t>::max();
  int16_t min = std::numeric_limits<int16_t>::min();

  TypeInt16() : TypeBuiltin(TYPE_I16, "Int16", 16) {}

  llvm::Type *toLLVMType(llvm::LLVMContext &lc) override {
    return llvm::Type::getInt16Ty(lc);
  }

  ~TypeInt16() override = default;
};

class TypeInt64 : public TypeBuiltin {
public:
  int64_t max = std::numeric_limits<int64_t>::max();
  int64_t min = std::numeric_limits<int64_t>::min();

  TypeInt64() : TypeBuiltin(TYPE_I64, "Int64", 64) {}

  llvm::Type *toLLVMType(llvm::LLVMContext &lc) override {
    return llvm::Type::getInt64Ty(lc);
  }

  ~TypeInt64() override = default;
};

class TypeUint16 : public TypeBuiltin {
public:
  uint16_t max = std::numeric_limits<uint16_t>::max();
  uint16_t min = std::numeric_limits<uint16_t>::min();

  TypeUint16() : TypeBuiltin(TYPE_U16, "Uint16", 16) {}

  // llvm::Type *toLLVMType(llvm::LLVMContext &lc) override {
  //   return llvm::Type::getInt16Ty(lc);
  // }

  ~TypeUint16() override = default;

  llvm::Type *toLLVMType(llvm::LLVMContext &lc) override {
    (void)lc;
    return nullptr;
  }
};

class TypeUint32 : public TypeBuiltin {
public:
  uint32_t max = std::numeric_limits<uint32_t>::max();
  uint32_t min = std::numeric_limits<uint32_t>::min();

  TypeUint32() : TypeBuiltin(TYPE_U32, "Uint32", 32) {}

  ~TypeUint32() override = default;

  llvm::Type *toLLVMType(llvm::LLVMContext &lc) override {
    (void)lc;
    return nullptr;
  }
};

class TypeUint64 : public TypeBuiltin {
public:
  uint64_t max = std::numeric_limits<uint64_t>::max();
  uint64_t min = std::numeric_limits<uint64_t>::min();

  TypeUint64() : TypeBuiltin(TYPE_U64, "Uint64", 64) {}

  ~TypeUint64() override = default;

  llvm::Type *toLLVMType(llvm::LLVMContext &lc) override {
    (void)lc;
    return nullptr;
  }
};

class TypeFloat64 : public TypeBuiltin {
public:
  double min = std::numeric_limits<double>::min();
  double max = std::numeric_limits<double>::max();
  double eps = std::numeric_limits<double>::epsilon();

  TypeFloat64() : TypeBuiltin(TYPE_F64, "Float64", 64) {}

  ~TypeFloat64() override = default;

  llvm::Type *toLLVMType(llvm::LLVMContext &lc) override {
    (void)lc;
    return nullptr;
  }
};

// f32
class TypeReal : public TypeBuiltin {
public:
  float max = std::numeric_limits<float>::max();
  float min = std::numeric_limits<float>::min();
  float eps = std::numeric_limits<float>::epsilon();

  TypeReal() : TypeBuiltin(TYPE_FLOAT, "Real", 32) {}

  ~TypeReal() override = default;

  llvm::Type *toLLVMType(llvm::LLVMContext &lc) override {
    (void)lc;
    return nullptr;
  }
};

class TypeString : public TypeBuiltin {
public:
  // 8 bits -> 1 byte for '\0'
  TypeString() : TypeBuiltin(TYPE_STRING, "String", 8) {}

  TypeString(size_t size) : TypeBuiltin(TYPE_STRING, "String", size) {}

  llvm::Type *toLLVMType(llvm::LLVMContext &lc) override {
    (void)lc;
    return nullptr;
  }
};

class TypeBool : public TypeBuiltin {
public:
  TypeBool() : TypeBuiltin(TYPE_BOOL, "Bool", 1) {}

  llvm::IntegerType *toLLVMType(llvm::LLVMContext &lc) override {
    return llvm::Type::getInt1Ty(lc);
  }

  ~TypeBool() override = default;
};

class TypeArray : public Type {
public:
  std::shared_ptr<Type> el_type;
  uint32_t size;

  TypeArray(uint32_t size, std::shared_ptr<Type> el_type)
      : Type(TYPE_ARRAY, "Array"), el_type(std::move(el_type)), size(size) {}

  // for when we dont know the parameters yet
  TypeArray() : Type(TYPE_ARRAY, "Array"), el_type(nullptr), size(0) {}

  llvm::Type *toLLVMType(llvm::LLVMContext &lc) override {
    (void)lc;
    return nullptr;
  }

  ~TypeArray() override = default;
};

// signature of a function
// is split in two field, so:
// args[0] -> args[1] -> ... -> return_type
class TypeFunc : public Type {
public:
  // store just the signature
  // so just return type and arg types
  std::shared_ptr<Type> return_type;
  std::vector<std::shared_ptr<Type>> args;
  bool isVoided; // no parameters
  bool isVoid;   // no return type

  TypeFunc(std::shared_ptr<Type> return_type,
           std::vector<std::shared_ptr<Type>> args)
      : Type(TYPE_FUNC, "Function"), return_type(std::move((return_type))),
        args(std::move((args))), isVoided(false), isVoid(false) {}

  TypeFunc(const std::shared_ptr<Type> &return_type)
      : Type(TYPE_FUNC, "Function"), return_type(return_type), isVoided(true),
        isVoid(false) {}

  // default constructor
  TypeFunc()
      : Type(TYPE_FUNC, "Function"), return_type(nullptr), isVoided(true),
        isVoid(true) {}

  TypeFunc(std::vector<std::shared_ptr<Type>> args)
      : Type(TYPE_FUNC, "Function"), return_type(nullptr),
        args(std::move(args)), isVoided(false), isVoid(true) {}

  ~TypeFunc() override = default;

  llvm::Type *toLLVMType(llvm::LLVMContext &lc) override {
    (void)lc;
    return nullptr;
  }
};

// class "signature"
// is a:
// [base_class] -> field_types[0] -> field_types[1] -> ... -> method_types[0] ->
// method_types[1] -> ...
class TypeClass : public Type {
public:
  //  std::string name;
  // @TODO add base class info
  std::shared_ptr<TypeClass> base_class;
  std::vector<std::shared_ptr<Type>> fields_types;
  std::vector<std::shared_ptr<TypeFunc>> methods_types;

  TypeClass(const std::string &name,
            std::vector<std::shared_ptr<Type>> fields_types,
            std::vector<std::shared_ptr<TypeFunc>> methods_types)
      : Type(TYPE_CLASS, name), fields_types(std::move(fields_types)),
        methods_types(std::move((methods_types))) {}

  void addBaseClass(const std::shared_ptr<TypeClass> &base) {
    base_class = base;
  }

  std::shared_ptr<TypeFunc> getMethod(const std::string &name) const {
    for (const auto &method : methods_types) {
      if (method->name == name)
        return method;
    }
    return nullptr;
  }

  llvm::Type *toLLVMType(llvm::LLVMContext &lc) override {
    (void)lc;
    return nullptr;
  }

  std::shared_ptr<Type> getField(const std::string &fieldName) const {
    for (const auto &field : fields_types) {
      if (field->name == fieldName) {
        return field;
      }
    }
    return nullptr;
  }

  ~TypeClass() override = default;
};

class TypeList : public Type {
public:
  std::shared_ptr<Type> el_type;

  TypeList(std::shared_ptr<Type> el_type)
      : Type(TYPE_LIST, "List"), el_type(std::move((el_type))) {}

  TypeList() : Type(TYPE_LIST, "List"), el_type(nullptr) {}

  llvm::Type *toLLVMType(llvm::LLVMContext &lc) override {
    (void)lc;
    return nullptr;
  }

  ~TypeList() override = default;
};

/**
 * The approach i want to take
 * is essentialy an access types from ADA
 * but in the form of anonymous access types
 * from ADA
 *
 * Constraints:
 * - Pointer MUST BE initialized before use
 *
 * - Pointers can only point to a variables tht are
 *   explicitly marked as available to be referenced
 *   example: `var i : ref Integer`
 *                     ^^^
 *
 * - this attributes for pointer types are allowed:
 *    - `general` - raw C pointer 
 *    - `own` - unique pointer, move semantic 
 *    - `not_null` - cannot be nulled 
 */
class TypePointer : public Type {
public:
  TypePointer(const std::shared_ptr<Type> &toType, size_t depth,
              bool allowsNull, bool isLimited, bool isGeneral)
      : Type(TYPE_POINTER, "Pointer"), toType(toType), depth(depth),
        allowsNull(allowsNull), isLimited(isLimited), isGeneral(isGeneral) {}

  std::shared_ptr<Type> toType; // type that it points to
  size_t depth;
  bool allowsNull;
  bool isLimited;
  bool isGeneral;
};

#endif
