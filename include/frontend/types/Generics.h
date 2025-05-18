#ifndef OBW_GENERICS_H
#define OBW_GENERICS_H

// Reference : Implementing Swift generics
// but obviously without constraints

#include "Types.h"

class TypeOpaque : public Type {
public:
  TypeOpaque()
    : Type(TYPE_OPAQUE, "Opaque") {}

  llvm::Type *toLLVMType(llvm::LLVMContext &lc) override {
    return llvm::StructType::getTypeByName(lc, "obw.opaque");
  };
};

class ValueWitnessTable {
public:
  size_t size;
  size_t align;


};

//####========= METADATA FOR SIMPLE TYPES =========####


#endif
