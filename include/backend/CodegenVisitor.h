#ifndef OBW_CODEGENVISITOR_H
#define OBW_CODEGENVISITOR_H

#include "frontend/SymbolTable.h"
#include "frontend/parser/Visitor.h"

#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Utils.h"

#include <map>

/*
class CodeGenVisitor {
public:
  CodeGenVisitor(const std::shared_ptr<Scope> &globalScope,
                 const std::shared_ptr<TypeTable> &typeTable,
                 const std::shared_ptr<Entity> &root)
      : globalScope(globalScope), typeTable(typeTable), root(root) {
    context = std::make_unique<llvm::LLVMContext>();
    builder = std::make_unique<llvm::IRBuilder>(*context);
    module = std::make_unique<llvm::Module>("Module", *context);
  }

  llvm::Value *visit(const std::shared_ptr<IntLiteralEXP> &node);
  llvm::Value *visit(const std::shared_ptr<RealLiteralEXP> &node);
  void visit(const std::shared_ptr<StringLiteralEXP> &node);
  void visit(const std::shared_ptr<BoolLiteralEXP> &node);
  void visit(const std::shared_ptr<ArrayLiteralExpr> &node);
  llvm::Value *visit(const std::shared_ptr<VarRefEXP> &node);
  void visit(const std::shared_ptr<FieldRefEXP> &node);
  void visit(const std::shared_ptr<MethodCallEXP> &node);
  llvm::Value *visit(const std::shared_ptr<FuncCallEXP> &node);
  void visit(const std::shared_ptr<ClassNameEXP> &node);
  void visit(const std::shared_ptr<ConstructorCallEXP> &node);
  void visit(const std::shared_ptr<CompoundEXP> &node);
  void visit(const std::shared_ptr<ThisEXP> &node);
  void visit(const std::shared_ptr<BinaryOpEXP> &node);
  void visit(const std::shared_ptr<FieldDecl> &node);
  void visit(const std::shared_ptr<VarDecl> &node);
  void visit(const std::shared_ptr<ParameterDecl> &node);
  void visit(const std::shared_ptr<MethodDecl> &node);
  void visit(const std::shared_ptr<ConstrDecl> &node);
  void visit(const std::shared_ptr<FuncDecl> &node);
  void visit(const std::shared_ptr<ClassDecl> &node);
  void visit(const std::shared_ptr<ArrayDecl> &node);
  void visit(const std::shared_ptr<ListDecl> &node);
  void visit(const std::shared_ptr<AssignmentSTMT> &node);
  void visit(const std::shared_ptr<ReturnSTMT> &node);
  void visit(const std::shared_ptr<IfSTMT> &node);
  void visit(const std::shared_ptr<CaseSTMT> &node);
  void visit(const std::shared_ptr<SwitchSTMT> &node);
  void visit(const std::shared_ptr<WhileSTMT> &node);
  void visit(const std::shared_ptr<ForSTMT> &node);

  void dumpIR() const { module->print(llvm::outs(), nullptr); }
private:
  // entry for a symboltable (actually a tree of scopes)
  std::shared_ptr<Scope> globalScope;
  // global type table
  std::shared_ptr<TypeTable> typeTable;
  // root &node of an AST
  std::shared_ptr<Entity> root;

  std::unique_ptr<llvm::LLVMContext> context;
  std::unique_ptr<llvm::IRBuilder<>> builder;
  std::unique_ptr<llvm::Module> module;

  std::map<std::string, llvm::AllocaInst *> varEnv;
}; */

#endif
