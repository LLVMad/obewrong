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
#include "llvm/Support/FileSystem.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Linker/Linker.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Transforms/Utils/BuildLibCalls.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/TargetParser/Host.h"

#include <map>
#include <variant>

#define cgresult_t  std::variant<std::monostate, llvm::Value*, llvm::Type*>
#define cgvoid_t    std::monostate
#define cgnone      std::monostate {}

#define cggetval(var)    std::get<llvm::Value*>(var)
#define cggettype(var)   std::get<llvm::Type*>(var)

#define LIBC_PATH "../lib/libc.ll"

class CodeGenVisitor {
public:
  CodeGenVisitor(const std::shared_ptr<Scope> &globalScope,
                 const std::shared_ptr<GlobalTypeTable> &typeTable)
      : globalScope(globalScope), typeTable(typeTable) {
    context = std::make_unique<llvm::LLVMContext>();
    builder = std::make_unique<llvm::IRBuilder<>>(*context);
    module = std::make_unique<llvm::Module>("Module", *context);

    currentScope = globalScope;
    currDepth = 0;

    auto triple = llvm::sys::getDefaultTargetTriple();
    auto ext_std_lib_info_impl = std::make_unique<llvm::TargetLibraryInfoImpl>(
      llvm::Triple(triple));
    auto ext_std_lib_info = std::make_unique<llvm::TargetLibraryInfo>(*ext_std_lib_info_impl);

    // - printf
    auto printfType = llvm::FunctionType::get(
      llvm::Type::getInt32Ty(*context),
      {llvm::PointerType::get(llvm::Type::getInt8Ty(*context), 0),},
      true);
    llvm::getOrInsertLibFunc(module.get(), *ext_std_lib_info, llvm::LibFunc_printf, printfType);

    // - malloc
    auto mallocType = llvm::FunctionType::get(
    llvm::PointerType::get(llvm::Type::getInt8Ty(*context), 0),
    {llvm::Type::getInt64Ty(*context)},
    false);
    llvm::getOrInsertLibFunc(module.get(), *ext_std_lib_info, llvm::LibFunc_malloc, mallocType);

    // - free
    auto freeType = llvm::FunctionType::get(
      llvm::Type::getVoidTy(*context),
      {llvm::PointerType::get(llvm::Type::getInt8Ty(*context), 0)},
      false
    );
    llvm::getOrInsertLibFunc(module.get(), *ext_std_lib_info, llvm::LibFunc_free, freeType);

    // link to standard library
    // auto libc_module = std::make_unique<llvm::Module>("libc_module", *context);
    /*
    llvm::SMDiagnostic err;
    auto libc_module = parseIRFile(LIBC_PATH, err, *context);
    if (!libc_module) {
      throw std::runtime_error("Could not parse libc_module");
    }

    if (llvm::verifyModule(*libc_module, &llvm::errs())) {
      llvm::errs() << "Module verification failed after linking!\n";
      // Handle error
    }

    libc_module->print(llvm::outs(), nullptr);

    bool linkStatus = llvm::Linker::linkModules(*module, std::move(libc_module));
    // bool linkStatus = linker.linkInModule(std::move(libc_module));
    if (linkStatus) {
      throw std::runtime_error("Could not link libc_module");
    }

    module->print(llvm::outs(), nullptr);
    if (!module->getFunction("printf")) {
      throw std::runtime_error("Could not find printf() function");
    }

    // insert printf
    // std::vector<llvm::Type *> printfArgsTypes(
        // {llvm::PointerType::getInt8Ty(*context)});
    // llvm::ArrayRef<llvm::Type*> printfArgsTypes =
    //   llvm::ArrayRef<llvm::Type*>({llvm::PointerType::get(llvm::Type::getInt8Ty(*context), 0)});
    // llvm::FunctionType *printfType = llvm::FunctionType::get(llvm::Type::getInt32Ty(*context), printfArgsTypes, true);
    // llvm::Function::Create(printfType, llvm::Function::ExternalLinkage,
    // "printf", module.get()); */
  }

  // starting point of the code generation
  // cgvoid_t visitDefault(const std::shared_ptr<Entity> &node);

  cgresult_t visitDefault(const std::shared_ptr<Entity> &node);

  // #####========== EXPRESSIONS ==========#####
  cgresult_t visit(const std::shared_ptr<Expression> &expr);
  cgresult_t visit(const std::shared_ptr<IntLiteralEXP> &node);
  cgresult_t visit(const std::shared_ptr<RealLiteralEXP> &node);
  cgresult_t visit(const std::shared_ptr<StringLiteralEXP> &node);
  cgvoid_t visit(const std::shared_ptr<BoolLiteralEXP> &node);
  cgvoid_t visit(const std::shared_ptr<ArrayLiteralExpr> &node);
  cgresult_t visit(const std::shared_ptr<VarRefEXP> &node);
  cgvoid_t visit(const std::shared_ptr<FieldRefEXP> &node);
  cgvoid_t visit(const std::shared_ptr<MethodCallEXP> &node);
  cgresult_t visit(const std::shared_ptr<FuncCallEXP> &node);
  cgresult_t visit(const std::shared_ptr<ClassNameEXP> &node);
  cgvoid_t visit(const std::shared_ptr<ConstructorCallEXP> &node);
  cgvoid_t visit(const std::shared_ptr<CompoundEXP> &node);
  cgvoid_t visit(const std::shared_ptr<ThisEXP> &node);
  cgresult_t visit(const std::shared_ptr<BinaryOpEXP> &node);
  // #####========================================#####

  // #####========== DECLARATIONS ==========#####
  cgvoid_t visit(const std::shared_ptr<Decl> &node);
  cgvoid_t visit(const std::shared_ptr<ModuleDecl> &node);
  cgvoid_t visit(const std::shared_ptr<FieldDecl> &node);
  cgvoid_t visit(const std::shared_ptr<VarDecl> &node);
  cgvoid_t visit(const std::shared_ptr<ParameterDecl> &node);
  cgvoid_t visit(const std::shared_ptr<MethodDecl> &node);
  cgvoid_t visit(const std::shared_ptr<ConstrDecl> &node);
  cgvoid_t visit(const std::shared_ptr<FuncDecl> &node);
  cgvoid_t visit(const std::shared_ptr<ClassDecl> &node);
  cgvoid_t visit(const std::shared_ptr<ArrayDecl> &node);
  cgvoid_t visit(const std::shared_ptr<ListDecl> &node);
  // #####========================================#####

  // #####========== STATEMENTS ==========#####
  cgvoid_t visit(const std::shared_ptr<Statement> &node);
  cgvoid_t visit(const std::shared_ptr<AssignmentSTMT> &node);
  cgvoid_t visit(const std::shared_ptr<ReturnSTMT> &node);
  cgvoid_t visit(const std::shared_ptr<IfSTMT> &node);
  cgvoid_t visit(const std::shared_ptr<CaseSTMT> &node);
  cgvoid_t visit(const std::shared_ptr<SwitchSTMT> &node);
  cgvoid_t visit(const std::shared_ptr<WhileSTMT> &node);
  cgvoid_t visit(const std::shared_ptr<ForSTMT> &node);
  // #####========================================#####

  void dumpIR() const { module->print(llvm::outs(), nullptr); }

  void createObjFile();

private:
  llvm::Function *getFunction(std::string name);
  llvm::AllocaInst *createEntryBlockAlloca(llvm::Function *TheFunction,
                                           llvm::Type *Type,
                                           llvm::StringRef VarName);

  // entry for a symboltable (actually a tree of scopes)
  std::shared_ptr<Scope> globalScope;
  // global type table
  std::shared_ptr<GlobalTypeTable> typeTable;
  // root &node of an AST
  // std::shared_ptr<Entity> root;

  std::unique_ptr<llvm::LLVMContext> context;
  std::unique_ptr<llvm::IRBuilder<>> builder;
  std::unique_ptr<llvm::Module> module;

  /// @deprecated use GlobalSymbolTable
  // std::map<std::string, llvm::AllocaInst*> varEnv;
  // std::map<std::string, bool> varInitialized;

  std::shared_ptr<Scope> currentScope;
  size_t currDepth;

  std::string moduleName;
};

#endif
