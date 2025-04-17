#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/StringRef.h"
#include "backend/CodegenVisitor.h"
#include "lld/Common/Driver.h"
// #include "lld/Common/"

#include "util/Logger.h"

#define CG_ERR(path, msg)                                                   \
ERR("%s: %s\n", path, msg)

cgresult_t CodeGenVisitor::visitDefault(const std::shared_ptr<Entity> &node) {
  // Because E_Kind enum values
  // go in ORDER we can just check
  // their numeric values
  if (node->getKind() >= 0 && node->getKind() < 11) {
    return visit(std::static_pointer_cast<Decl>(node));
  }
  if (node->getKind() >= 11 && node->getKind() < 16) {
    // types visit
    // can it happen actually?
    // i think they cannot appear as a
    // DISTINCT node themselfs
    return cgnone;
  }
  if (node->getKind() >= 16 && node->getKind() < 36) {
    return visit(std::static_pointer_cast<Expression>(node));
  }
  else {
    return visit(std::static_pointer_cast<Statement>(node));
  }
  return cgnone;
}

//#####===========================================#####
//#####=============== EXPRESSIONS ===============#####
//#####===========================================#####

cgresult_t CodeGenVisitor::visit(const std::shared_ptr<Expression> &expr) {
  switch (expr->getKind()) {
  case E_Integer_Literal: {
    auto exprIntLit = std::static_pointer_cast<IntLiteralEXP>(expr);
    return visit(exprIntLit);
  }
  case E_Real_Literal: {
    auto exprRealLit = std::static_pointer_cast<RealLiteralEXP>(expr);
    return visit(exprRealLit);
  }
  case E_Binary_Operator: {
    auto exprBinOp = std::static_pointer_cast<BinaryOpEXP>(expr);
    return visit(exprBinOp);
  }
  case E_String_Literal: {
    auto exprStringLit = std::static_pointer_cast<StringLiteralEXP>(expr);
    return visit(exprStringLit);
  }
  case E_Function_Call: {
    auto exprFuncCall = std::static_pointer_cast<FuncCallEXP>(expr);
    return visit(exprFuncCall);
  }
  case E_Var_Reference: {
    auto exprVarRef = std::static_pointer_cast<VarRefEXP>(expr);
    return visit(exprVarRef);
  }
  default: return cgnone;
  }
}

cgresult_t CodeGenVisitor::visit(const std::shared_ptr<FuncCallEXP> &node) {
  // Look up the name in the global module table.
  llvm::Function *CalleeF = getFunction(node->func_name);
  // if (!CalleeF)
  //   return cgnone;
    // return LogErrorV("Unknown function referenced");

  // If argument mismatch error.
  // if (CalleeF->arg_size() != llvm::Args.size())
    // return LogErrorV("Incorrect # arguments passed");

  std::vector<llvm::Value *> ArgsV;
  auto Args = node->arguments;
  // auto FArgs = CalleeF->args();
  for (unsigned i = 0, e = Args.size(); i != e; ++i) {
    cgresult_t arg = visit(Args[i]);
    ArgsV.push_back(cggetval(arg));

    // if (ArgsV[i]->getType() )

    // if (ArgsV[i]->getType() != CalleeF->getFunctionType()->getParamType(i)) {
    //
    // }

    if (!ArgsV.back())
      return cgnone;
  }

  if (CalleeF->getReturnType()->isVoidTy()) {
    return builder->CreateCall(CalleeF, ArgsV);
    // return cgnone;
  }
  else
    return builder->CreateCall(CalleeF, ArgsV, "calltmp");
}

cgresult_t CodeGenVisitor::visit(const std::shared_ptr<StringLiteralEXP> &node) {
  return builder->CreateGlobalString(node->value);
}

cgresult_t CodeGenVisitor::visit(const std::shared_ptr<IntLiteralEXP> &node) {
  switch (node->getByteSize()) {
    case 8: {
      return llvm::ConstantInt::getSigned((llvm::Type::getInt8Ty(*context)),
                                    node->getValue());
    }
    case 16: {
      return llvm::ConstantInt::getSigned((llvm::Type::getInt16Ty(*context)), node->getValue());
    }
    case 32: {
      return llvm::ConstantInt::getSigned((llvm::Type::getInt32Ty(*context)), node->getValue());
    }
    case 64: {
      return llvm::ConstantInt::getSigned((llvm::Type::getInt64Ty(*context)), node->getValue());
    }
    default: {
      return llvm::ConstantInt::getSigned((llvm::Type::getInt64Ty(*context)),
                                  node->getValue());
    }
  }

}

cgresult_t 
CodeGenVisitor::visit(const std::shared_ptr<RealLiteralEXP> &node) {
  return llvm::ConstantFP::get(*context, llvm::APFloat(node->getValue()));
}

cgresult_t CodeGenVisitor::visit(const std::shared_ptr<VarRefEXP> &node) {
  // llvm::AllocaInst *alloca = currentScope->lookupAlloca(node->var_name); /* varEnv[node->var_name]; */
  // bool isInited = currentScope->isDeclInitialized(no);
  auto [_, alloc, isInited] = *currentScope->getSymbol(node->var_name);

  // if (!isInited) {
  //   return alloc;
  // }

  return builder->CreateLoad(alloc->getAllocatedType(), alloc, node->var_name.c_str());
}

cgresult_t CodeGenVisitor::visit(const std::shared_ptr<ClassNameEXP> &node) {
  auto resolvedType = typeTable->getType(moduleName, node->name);
  switch (resolvedType->kind) {
  case TYPE_INT: {
    return llvm::Type::getInt32Ty(*context);
  }
  case TYPE_BYTE: {
    return llvm::Type::getInt8Ty(*context);
  }
  default:
    return cgnone;
  }
}

cgresult_t CodeGenVisitor::visit(const std::shared_ptr<BinaryOpEXP> &node) {
  llvm::Value *L = cggetval(visit(node->left));
  llvm::Value *R = cggetval(visit(node->right));
  if (!L || !R)
    return cgnone;

  switch (node->op) {
  case OP_PLUS:
    return builder->CreateAdd(L, R, "addtmp");
  case OP_MINUS:
    return builder->CreateSub(L, R, "subtmp");
  case OP_MULTIPLY:
    return builder->CreateMul(L, R, "multmp");
  case OP_LESS:
    L = builder->CreateFCmpULT(L, R, "cmptmp");
    // Convert bool 0/1 to double 0.0 or 1.0
    return builder->CreateUIToFP(L, llvm::Type::getDoubleTy(*context),
                                 "booltmp");
  default:
    return cgnone;
  }
}

//#####============================================#####
//#####=============== DECLARATIONS ===============#####
//#####============================================#####

cgvoid_t CodeGenVisitor::visit(const std::shared_ptr<Decl> &node) {
  switch (node->getKind()) {
  case E_Field_Decl: {
    // auto fieldDecl = std::static_pointer_cast<FieldDecl>(node);
    // visit(fieldDecl);
  } break;
  case E_Variable_Decl: {
    auto varDecl = std::static_pointer_cast<VarDecl>(node);
    visit(varDecl);
  }
  case E_Parameter_Decl: {
    // auto paramDecl = std::static_pointer_cast<ParameterDecl>(node);
    // visit(paramDecl);
  } break;
  case E_Method_Decl: {
    // auto methodDecl = std::static_pointer_cast<MethodDecl>(node);
    // visit(methodDecl);
  } break;
  case E_Constructor_Decl: {
    // auto constrDecl = std::static_pointer_cast<ConstrDecl>(node);
    // visit(constrDecl);
  } break;
  case E_Function_Decl: {
    auto funcDecl = std::static_pointer_cast<FuncDecl>(node);
    visit(funcDecl);
  } break;
  case E_Class_Decl: {
    // auto classDecl = std::static_pointer_cast<ClassDecl>(node);
    // visit(classDecl);
  } break;
  // case E_Array_Decl: {
  //
  // }
  case E_Module_Decl: {
    auto moduleDecl = std::static_pointer_cast<ModuleDecl>(node);
    visit(moduleDecl);
  } break;
  case E_Enum_Decl: {
    // auto enumDecl = std::static_pointer_cast<EnumDecl>(node);
    // visit(enumDecl);
  } break;
  }

  return cgnone;
}

cgvoid_t CodeGenVisitor::visit(const std::shared_ptr<ModuleDecl> &node) {
  cgresult_t result;
  currentScope = currentScope->nextScope(); // global scope -> module scope
  auto children = node->children;
  for (const auto &child : children) {
    result = visitDefault(child);
  }

  return cgnone;
}

cgvoid_t CodeGenVisitor::visit(const std::shared_ptr<VarDecl> &node) {
  // std::vector<llvm::AllocaInst *> oldBindings;

  llvm::Function *function = builder->GetInsertBlock()->getParent();

  std::string var_name = node->name;
  auto varType = node->type->toLLVMType(*context);
  auto initializer = node->initializer;
  llvm::AllocaInst *alloca;
  llvm::Value *initVal;
  if (initializer) {
    initVal = cggetval(visit(initializer));
    alloca = builder->CreateAlloca(varType, initVal, var_name);
    builder->CreateStore(initVal, alloca);
  }
  else {
    // @TODO
    alloca = builder->CreateAlloca(varType, nullptr, var_name);
  }

  // oldBindings.push_back(varEnv[var_name]);
  // varEnv[var_name] = alloca;
  currentScope->addSymbol(var_name, alloca);

  return cgnone;
}

cgvoid_t CodeGenVisitor::visit(const std::shared_ptr<FuncDecl> &node) {
  currentScope = currentScope->nextScope();

  // CREATE PROTOTYPE OF A FUNCTION
  std::vector<llvm::Type*> argTypes;
  for (auto &arg : node->args) {
    argTypes.push_back(arg->type->toLLVMType(*this->context));
  }

  llvm::Type *returnType = nullptr;
  if (!node->isVoid)
    returnType = node->signature->return_type->toLLVMType(*this->context);
  else
    returnType = llvm::Type::getVoidTy(*this->context);

  llvm::FunctionType *FT = llvm::FunctionType::get(returnType, argTypes, false);

  llvm::Function *F = llvm::Function::Create(FT, llvm::Function::CommonLinkage,
    node->name, module.get());

  size_t i = 0;
  for (auto &arg : F->args()) {
    arg.setName(node->args[i]->name);
    i++;
  }

  // CREATE FUNCTION BODY
  // - Create a new basic block to start insertion into.
  llvm::BasicBlock *BB = llvm::BasicBlock::Create(*context, "entry", F);
  builder->SetInsertPoint(BB);

  // - Record the function arguments in the NamedValues map.
  // varEnv.clear();
  for (auto &arg : F->args()) {
    llvm::AllocaInst *alloca =
      createEntryBlockAlloca(F, arg.getType(), arg.getName());
    builder->CreateStore(&arg, alloca);
    // varEnv[std::string(arg.getName())] = alloca;
    currentScope->addSymbol(std::string(arg.getName()), alloca);
  }

  // - Generate func body
  auto funcBody = node->body;
  cgresult_t returnValue;
  for (auto &el : funcBody->parts) {
    returnValue = visitDefault(el);
  }

  if (node->isVoid) {
    builder->CreateRetVoid();
  }

  verifyFunction(*F);

  currentScope = currentScope->prevScope();
  // varEnv.clear();

  return cgnone;
};


//#####===========================================#####
//#####=============== STATEMENTS ================#####
//#####===========================================#####

cgvoid_t CodeGenVisitor::visit(const std::shared_ptr<Statement> &node) {
  switch (node->getKind()) {
    case E_Assignment: {
      auto stAss = std::static_pointer_cast<AssignmentSTMT>(node);
      visit(stAss);
    } break;
    case E_Return_Statement: {
      auto rtAss = std::static_pointer_cast<ReturnSTMT>(node);
      visit(rtAss);
    } break;
  }

  return cgnone;
}

cgvoid_t CodeGenVisitor::visit(const std::shared_ptr<AssignmentSTMT> &node) {
  llvm::Value *var = cggetval(visit(node->variable));
  llvm::Value *assignment = cggetval(visit(node->expression));

  // varInitialized[node->variable->var_name] = true;
  currentScope->markInitialized(node->variable->var_name);
  builder->CreateStore(assignment, var);
  return cgnone;
}

cgvoid_t CodeGenVisitor::visit(const std::shared_ptr<ReturnSTMT> &node) {
  llvm::Value *retVal = cggetval(visit(node->expr));

  builder->CreateRet(retVal);
  return cgnone;
}

//#####=========================================#####
//#####=============== COMPILING ===============#####
//#####=========================================#####

void CodeGenVisitor::createObjFile() {
  llvm::InitializeAllTargetInfos();
  llvm::InitializeAllTargets();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmParsers();
  llvm::InitializeAllAsmPrinters();

  auto TargetTriple = llvm::sys::getDefaultTargetTriple();
  module->setTargetTriple(
    llvm::Triple(TargetTriple).str()
    );

  std::string Error;
  auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple, Error);

  // Print an error and exit if we couldn't find the requested target.
  // This generally occurs if we've forgotten to initialise the
  // TargetRegistry or we have a bogus target triple.
  if (!Target) {
    llvm::errs() << Error;
    return;
  }

  auto CPU = "generic";
  auto Features = "";

  llvm::TargetOptions opt;
  auto TheTargetMachine = Target->createTargetMachine(
      llvm::Triple(TargetTriple).str(), CPU, Features, opt, llvm::Reloc::PIC_);

  module->setDataLayout(TheTargetMachine->createDataLayout());

  auto Filename = moduleName + ".o";
  std::error_code EC;
  llvm::raw_fd_ostream dest(Filename, EC, llvm::sys::fs::OF_None);

  if (EC) {
    llvm::errs() << "Could not open file: " << EC.message();
    return;
  }

  llvm::legacy::PassManager pass;
  auto FileType = llvm::CodeGenFileType::ObjectFile;

  if (TheTargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType)) {
    llvm::errs() << "TheTargetMachine can't emit a file of this type";
    return;
  }

  pass.run(*module);
  dest.flush();

  llvm::outs() << "Wrote " << Filename << "\n";

  // clang linking
  std::string command = "clang " + Filename + " ";
}

//#####=========================================#####
//#####=============== UTILITARY ===============#####
//#####=========================================#####

llvm::Function *CodeGenVisitor::getFunction(std::string name) {
  // First, see if the function has already been added to the current module.
  if (auto *F = module->getFunction(name))
    return F;

  // If not, check whether we can codegen the declaration from some existing
  // prototype.
  // auto FI = llvm::FunctionProtos.find(name);
  // if (FI != FunctionProtos.end())
  // return FI->second->codegen();

  // If no existing prototype exists, return null.
  return nullptr;
}

llvm::AllocaInst *CodeGenVisitor::createEntryBlockAlloca(llvm::Function *TheFunction,
                                          llvm::Type* Type,
                                          llvm::StringRef VarName) {
  llvm::IRBuilder<> TmpB(&TheFunction->getEntryBlock(),
                 TheFunction->getEntryBlock().begin());
  return TmpB.CreateAlloca(Type, nullptr,
                           VarName);
}

//#####=========================================#####
//#####=========================================#####
//#####=========================================#####