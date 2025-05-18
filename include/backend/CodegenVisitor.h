#ifndef OBW_CODEGENVISITOR_H
#define OBW_CODEGENVISITOR_H

#include "frontend/SourceManager.h"
#include "frontend/SymbolTable.h"
#include "frontend/parser/Statement.h"
#include "frontend/parser/Visitor.h"
#include "frontend/parser/Wrappers.h"

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
#include <queue>
#include <variant>


class CodeGenVisitor : public BaseVisitor,
                  public Visitor<Entity, void>,
                  public Visitor<Block, void>,
                  public Visitor<EDummy, void>,
                  public Visitor<Statement, void>,
                  public Visitor<AssignmentSTMT, void>,
                  public Visitor<ReturnSTMT, void>,
                  public Visitor<IfSTMT, void>,
                  public Visitor<CaseSTMT, void>,
                  public Visitor<SwitchSTMT, void>,
                  public Visitor<WhileSTMT, void>,
                  public Visitor<ForSTMT, void>,
                  public Visitor<Expression, void>,
                  public Visitor<IntLiteralEXP, void>,
                  public Visitor<RealLiteralEXP, void>,
                  public Visitor<StringLiteralEXP, void>,
                  public Visitor<BoolLiteralEXP, void>,
                  public Visitor<ArrayLiteralExpr, void>,
                  public Visitor<VarRefEXP, void>,
                  public Visitor<FieldRefEXP, void>,
                  public Visitor<ElementRefEXP, void>,
                  public Visitor<MethodCallEXP, void>,
                  public Visitor<FuncCallEXP, void>,
                  public Visitor<ClassNameEXP, void>,
                  public Visitor<ConstructorCallEXP, void>,
                  public Visitor<CompoundEXP, void>,
                  public Visitor<ThisEXP, void>,
                  public Visitor<ConversionEXP, void>,
                  public Visitor<BinaryOpEXP, void>,
                  public Visitor<UnaryOpEXP, void>,
                  public Visitor<EnumRefEXP, void>,
                  public Visitor<Decl, void>,
                  public Visitor<FieldDecl, void>,
                  public Visitor<VarDecl, void>,
                  public Visitor<ParameterDecl, void>,
                  public Visitor<MethodDecl, void>,
                  public Visitor<ConstrDecl, void>,
                  public Visitor<FuncDecl, void>,
                  public Visitor<ClassDecl, void>,
                  public Visitor<ArrayDecl, void>,
                  public Visitor<ListDecl, void>,
                  public Visitor<ModuleDecl, void>,
                  public Visitor<EnumDecl, void> {
public:
  CodeGenVisitor(
    SourceManager &sm, std::shared_ptr<SourceBuffer> buff,
    const std::shared_ptr<Scope<Entity>> &globalScope,
                 const std::shared_ptr<GlobalTypeTable> &typeTable,
                 std::shared_ptr<llvm::LLVMContext> context)
      : globalScope(globalScope), typeTable(typeTable), context(context), sm(sm), buff(buff) {
    // context = std::make_unique<llvm::LLVMContext>();
    builder = std::make_unique<llvm::IRBuilder<>>(*context);

    auto children = globalScope->getChildren();
    for (auto child : children) {
      if (child->getKind() == SCOPE_MODULE) moduleName = child->getName();
    }

    // moduleName = globalScope->getChildren()[0]->getName();
    module = std::make_unique<llvm::Module>(
      moduleName,
      *context
      );

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

    // LINK MODULES
    auto included = sm.getIncluded(*buff);
    // sm.linkWithIncludedModules(*buff, );
    std::ranges::for_each(
      included,
      [&](auto &file) {
        if (llvm::Linker::linkModules(*module, std::move(file->module)))
          throw std::runtime_error("Failed to link modules");
        assert(module && "Module is null after linking!");
      }
    );

    // if(llvm::verifyModule(*module, &llvm::outs())) {
    //   throw std::runtime_error("Linker failed");
    // }
    // ============

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
  // void visitDefault(Entity &node) override;

  void visit(Entity &node) override;

  // #####========== EXPRESSIONS ==========#####
  void visit(Expression &node) override;
  void visit(IntLiteralEXP &node) override;
  void visit(RealLiteralEXP &node) override;
  void visit(StringLiteralEXP &node) override;
  void visit(BoolLiteralEXP &node) override;
  void visit(ArrayLiteralExpr &node) override;
  void visit(VarRefEXP &node) override;
  void visit(FieldRefEXP &node) override;
  void visit(MethodCallEXP &node) override;
  void visit(FuncCallEXP &node) override;
  void visit(ClassNameEXP &node) override;
  void visit(ConversionEXP &node) override;
  void visit(ConstructorCallEXP &node) override;
  void visit(CompoundEXP &node) override;
  void visit(ThisEXP &node) override;
  void visit(BinaryOpEXP &node) override;
  void visit(ElementRefEXP &node) override;
  // #####========================================#####

  // #####========== DECLARATIONS ==========#####
  void visit(Decl &node) override;
  void visit(ModuleDecl &node) override;
  void visit(FieldDecl &node) override;
  void visit(VarDecl &node) override;
  void visit(ParameterDecl &node) override; // outsorced its work to another function
  void visit(MethodDecl &node) override;
  void visit(ConstrDecl &node) override;
  void visit(FuncDecl &node) override;
  void visit(ClassDecl &node) override;
  void visit(ArrayDecl &node) override; // probably should be deleted
  void visit(ListDecl &node) override; // same
  // #####========================================#####

  // #####========== STATEMENTS ==========#####
  void visit(Statement &node) override;
  void visit(AssignmentSTMT &node) override;
  void visit(ReturnSTMT &node) override;
  void visit(IfSTMT &node) override;
  void visit(CaseSTMT &node) override;
  void visit(SwitchSTMT &node) override;
  void visit(WhileSTMT &node) override;
  void visit(ForSTMT &node) override;
  // #####========================================#####

  // UNUSED
  void visit(Block& block) override {};
  void visit(EDummy& dummy) override {}
  void visit(EnumDecl& node) override {}
  void visit(EnumRefEXP& node) override {}
  void visit(UnaryOpEXP &node) override {}

  // void visit(Type &node) override;

  void handleBuiltinMethodCall(MethodCallEXP &node,
                                     std::string methodName);

  void dumpIR() const { module->print(llvm::outs(), nullptr); }

  void createObjFile();

private:
  // #####========== UTILLITY ==========#####
  llvm::Function *getFunction(std::string name);
  llvm::AllocaInst *createEntryBlockAlloca(llvm::Function *TheFunction,
                                           llvm::Type *Type,
                                           llvm::StringRef VarName);

  // creates load instruction to load a pointer type value
  llvm::Value* unwrapPointerReference(Expression *node, llvm::Value *val);
  // #####========================================#####

  // entry for a symboltable (actually a tree of scopes)
  std::shared_ptr<Scope<Entity>> globalScope;
  // global type table
  std::shared_ptr<GlobalTypeTable> typeTable;
  // root &node of an AST
  // std::shared_ptr<Entity> root;

  std::shared_ptr<llvm::LLVMContext> context;
  std::unique_ptr<llvm::IRBuilder<>> builder;
  std::unique_ptr<llvm::Module> module;

  /// @deprecated use GlobalSymbolTable
  // std::map<std::string, llvm::AllocaInst*> varEnv;
  // std::map<std::string, bool> varInitialized;

  std::queue<llvm::Value*> values;
  llvm::Value* lastValue;
  std::shared_ptr<Scope<Entity>> currentScope;
  size_t currDepth;

  std::string moduleName;

  SourceManager &sm;
  std::shared_ptr<SourceBuffer> buff;
};

#endif
