#include "backend/CodegenVisitor.h"

/*
llvm::Value *CodeGenVisitor::visit(const std::shared_ptr<IntLiteralEXP> &node) {
  return llvm::ConstantInt::getSigned((llvm::Type::getInt32Ty(*context)),
                                    node->getValue());
}

llvm::Value *
CodeGenVisitor::visit(const std::shared_ptr<RealLiteralEXP> &node) {
  return llvm::ConstantFP::get(*context, llvm::APFloat(node->getValue()));
}
// llvm::Value *CodeGenVisitor::visit(const std::shared_ptr<VarRefEXP> &node) {
//   llvm::Value * var = llvm::NamedV
// } */