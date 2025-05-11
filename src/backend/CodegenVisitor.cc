#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/StringRef.h"
#include "backend/CodegenVisitor.h"
#include "lld/Common/Driver.h"
#include "llvm/IR/DerivedTypes.h"
// #include "lld/Common/"

#include "util/Logger.h"

#include <llvm/Support/Chrono.h>

#define CG_ERR(path, msg)                                                   \
ERR("%s: %s\n", path, msg)

cgresult_t CodeGenVisitor::visitDefault(const std::shared_ptr<Entity> &node) {
  // Because E_Kind enum values
  // go in ORDER we can just check
  // their numeric values
  if (node->getKind() >= 0 && node->getKind() < 12) {
    return visit(std::static_pointer_cast<Decl>(node));
  }
 
  if (node->getKind() >= 12 && node->getKind() < 17) {
    // types visit
    // can it happen actually?
    // i think they cannot appear as a
    // DISTINCT node themselfs
    return cgnone;
  }
  if (node->getKind() >= 16 && node->getKind() < 37) {
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
  case E_Array_Literal: {
    auto exprArrLit = std::static_pointer_cast<ArrayLiteralExpr>(expr);
    return visit(exprArrLit);
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
  case E_Constructor_Call: {
    auto exprConstrCall = std::static_pointer_cast<ConstructorCallEXP>(expr);
    return visit(exprConstrCall);
  }
  case E_Method_Call: {
    auto exprMethodCall = std::static_pointer_cast<MethodCallEXP>(expr);
    return visit(exprMethodCall);
  }
  case E_Var_Reference: {
    auto exprVarRef = std::static_pointer_cast<VarRefEXP>(expr);
    return visit(exprVarRef);
  }
  case E_Field_Reference: {
    auto exprFieldRef = std::static_pointer_cast<FieldRefEXP>(expr);
    return visit(exprFieldRef);
  }
  case E_Assignment_Wrapper: {
    auto wrapper = std::static_pointer_cast<AssignmentWrapperEXP>(expr);
    return visit(wrapper->assignment);
  }
  default: return cgnone;
  }
}

cgresult_t CodeGenVisitor::visit(const std::shared_ptr<FieldRefEXP> &node) {
  auto [decl, alloca, isInited] = *currentScope->getSymbol(node->obj->var_name);

  // dumpIR();

  auto varDecl = std::static_pointer_cast<VarDecl>(decl);

  if (varDecl->type->kind == TYPE_ACCESS) {
    auto ptrType = std::static_pointer_cast<TypeAccess>(varDecl->type);
    auto classTypeLLVM = ptrType->to->toLLVMType(*context);
    auto ptrAlloca = builder->CreateLoad(alloca->getAllocatedType(), alloca);

    // construct GEP
    return builder->CreateStructGEP(
      classTypeLLVM,
      ptrAlloca,
      node->index,
      node->field_name
    );
  } else {
    auto classTypeLLVM = varDecl->type->toLLVMType(*context);

    // construct GEP
    return builder->CreateStructGEP(
      classTypeLLVM,
      alloca,
      node->index,
      node->field_name
    );
  }

  // return builder->CreateLoad(
  //
  // );

}

cgresult_t CodeGenVisitor::visit(const std::shared_ptr<FuncCallEXP> &node) {
  // Look up the name in the global module table.
  llvm::Function *CalleeF = getFunction(node->func_name);
  // if (!CalleeF)
  //   return cgnone;
    // return LogErrorV("Unknown function referenced");

  std::vector<llvm::Value *> ArgsV;
  auto Args = node->arguments;
  // auto FArgs = CalleeF->args();
  for (unsigned i = 0, e = Args.size(); i != e; ++i) {
    cgresult_t arg = visit(Args[i]);
    auto val = cggetval(arg);

    // load an argument
    // @TODO probably shoulnt do it here???
    // becouse we shouldnt do load:
    // - in assignment
    if (Args[i]->getKind() == E_Field_Reference) {
      auto fieldRef = std::static_pointer_cast<FieldRefEXP>(Args[i]);
      auto [decl, alloca, isInited] = *currentScope->getSymbol(fieldRef->obj->var_name);
      auto varDecl = std::static_pointer_cast<VarDecl>(decl);

      auto classTypeLLVM = varDecl->type->toLLVMType(*context);
      val = builder->CreateLoad(
        classTypeLLVM->getStructElementType(fieldRef->index),
        val
      );
    }

    // @TODO
    // if (val->getType()->isPointerTy()) {
    //   val = builder->CreateLoad(
    //     val->getType()->getStructElementType(),
    //     val
    //   );
    // }

    ArgsV.push_back(val);

    if (!ArgsV.back())
      return cgnone;
  }

  if (CalleeF->getReturnType()->isVoidTy()) {
    return builder->CreateCall(CalleeF, ArgsV);
  }
  else
    return builder->CreateCall(CalleeF, ArgsV, "calltmp");
}

cgresult_t CodeGenVisitor::visit(const std::shared_ptr<ConstructorCallEXP> &node) {
  // Look up the name in the global module table.
  auto constrName = node->left->name + "_Create";
  llvm::Function *CalleeF = getFunction(constrName);

  std::vector<llvm::Value *> ArgsV;
  auto Args = node->arguments;

  // Create a pointer to the class instance

  // @TODO ?!
  // llvm::Type *classType;
  // if (node->left->name == "Integer") {
  //   classType = typeTable->getType(moduleName, "Integer");
  // }

  llvm::Type *classType = llvm::StructType::getTypeByName(*context, node->left->name);

  if (!classType) {
    classType = typeTable->getType(moduleName, "Integer")->toLLVMType(*context);
  }

  auto objInstanceRef = builder->CreateAlloca(classType);
  ArgsV.push_back(objInstanceRef);

  for (unsigned i = 0, e = Args.size(); i != e; ++i) {
    cgresult_t arg = visit(Args[i]);
    ArgsV.push_back(cggetval(arg));

    if (!ArgsV.back())
      return cgnone;
  }

  if (CalleeF->getReturnType()->isVoidTy()) {
    builder->CreateCall(CalleeF, ArgsV);
  }
  else
    builder->CreateCall(CalleeF, ArgsV, "calltmp");

  return objInstanceRef;
}

cgresult_t CodeGenVisitor::visit(const std::shared_ptr<StringLiteralEXP> &node) {
  std::string processed;
  for (size_t i = 0; i < node->value.length(); i++) {
    if (node->value[i] == '\\' && i + 1 < node->value.length()) {
      switch (node->value[i + 1]) {
        case 'n': processed += '\n'; break;
        case 't': processed += '\t'; break;
        case 'r': processed += '\r'; break;
        case '\\': processed += '\\'; break;
        case '"': processed += '"'; break;
        default: processed += node->value[i + 1]; break;
      }
      i++; // Skip the next character since we've processed it
    } else {
      processed += node->value[i];
    }
  }
  return builder->CreateGlobalString(processed);
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

cgresult_t CodeGenVisitor::visit(const std::shared_ptr<ArrayLiteralExpr> &node) {
  auto elType = typeTable->getType(moduleName, node->el_type);
  auto arrayTypeLLVM = llvm::ArrayType::get(elType->toLLVMType(*context), node->elements.size());

  // Create constant elements
  std::vector<llvm::Constant *> elements;
  for (const auto& el: node->elements) {
    auto valToConstant = dyn_cast<llvm::Constant>(cggetval(visit(el)));
    elements.push_back(valToConstant);
  }

  // Create a global constant array
  auto constArray = llvm::ConstantArray::get(arrayTypeLLVM, elements);
  auto globalArray = new llvm::GlobalVariable(
    *module,
    arrayTypeLLVM,
    true,  // isConstant
    llvm::GlobalValue::LinkageTypes::InternalLinkage,
    constArray,
    "array_literal"  // name
  );

  return globalArray;
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

cgresult_t CodeGenVisitor::visit(const std::shared_ptr<Type> &node) {
  switch (node->kind) {
  case TYPE_INT: {
    return llvm::Type::getInt32Ty(*context);
  }
  case TYPE_BYTE: {
    return llvm::Type::getInt8Ty(*context);
  }
  case TYPE_CLASS: {
    // ->pointer ??/ @TODO
    return llvm::StructType::getTypeByName(*context, llvm::StringRef(node->name));
  }
  case TYPE_ARRAY: {
    // pointer to first el type
    auto asArr = std::static_pointer_cast<TypeArray>(node);
    return asArr->toLLVMType(*context);
  }
  default:
    return cgnone;
  }
}

// cgresult_t CodeGenVisitor::visit(const std::shared_ptr<ClassNameEXP> &node) {
//   auto resolvedType = typeTable->getType(moduleName, node->name);
//   switch (resolvedType->kind) {
//   case TYPE_INT: {
//     return llvm::Type::getInt32Ty(*context);
//   }
//   case TYPE_BYTE: {
//     return llvm::Type::getInt8Ty(*context);
//   }
//   case TYPE_CLASS: {
//     return llvm::StructType::getTypeByName(*context, llvm::StringRef(node->name))->getPointerTo();
//   }
//   default:
//     return cgnone;
//   }
// }

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
    auto fieldDecl = std::static_pointer_cast<FieldDecl>(node);
    visit(fieldDecl);
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
    auto methodDecl = std::static_pointer_cast<MethodDecl>(node);
    visit(methodDecl);
  } break;
  case E_Constructor_Decl: {
    auto constrDecl = std::static_pointer_cast<ConstrDecl>(node);
    visit(constrDecl);
  } break;
  case E_Function_Decl: case E_Main_Decl: {
    auto funcDecl = std::static_pointer_cast<FuncDecl>(node);
    visit(funcDecl);
  } break;
  case E_Class_Decl: {
    auto classDecl = std::static_pointer_cast<ClassDecl>(node);
    visit(classDecl);
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

cgvoid_t CodeGenVisitor::visit(const std::shared_ptr<ClassDecl> &node) {
  currentScope = currentScope->nextScope();

  auto classType = llvm::StructType::create(*context, llvm::StringRef(node->name));

  auto classSignature = node->type;
  std::vector<llvm::Type*> fieldTypes;
  for (auto &field : node->fields) {
    // add fields types
    fieldTypes.push_back(field->type->toLLVMType(*context));
  }

  // llvm::StructType* classType =
  //   llvm::StructType::getTypeByName(*context, node->name);
  classType->setBody(llvm::ArrayRef(fieldTypes));

  // llvm::Value* result;
  // @TODO here is the problem -> the order of methods/constructos and their scopes
  for(auto &method : node->methods) {
    visit(method);
  }

  /**
   * @deprecated
   * see note above, nowe field methods keeps both of them
   */
  // for (auto &constr : node->constructors) {
  //   visit(constr);
  // }

  currentScope = currentScope->prevScope();

  return cgnone;
}

cgvoid_t CodeGenVisitor::visit(const std::shared_ptr<ConstrDecl> &node) {
  currentScope = currentScope->nextScope();

  // CREATE PROTOTYPE OF A FUNCTION
  std::vector<llvm::Type*> argTypes;
  for (auto &arg : node->args) {
    argTypes.push_back(arg->type->toLLVMType(*this->context));
  }

  llvm::Type *returnType = llvm::Type::getVoidTy(*this->context);

  llvm::FunctionType *FT = llvm::FunctionType::get(returnType, argTypes, false);

  llvm::Function *F = llvm::Function::Create(FT, llvm::Function::InternalLinkage,
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


  builder->CreateRetVoid();
  // }

  verifyFunction(*F);

  currentScope = currentScope->prevScope();
  // varEnv.clear();

  return cgnone;
}

cgvoid_t CodeGenVisitor::visit(const std::shared_ptr<MethodDecl> &node) {
  currentScope = currentScope->nextScope();

  // CREATE PROTOTYPE OF A FUNCTION
  std::vector<llvm::Type*> argTypes;
  
  // Add all parameters including 'this' which was added by the parser
  for (auto &arg : node->args) {
    argTypes.push_back(arg->type->toLLVMType(*this->context));
  }

  llvm::Type *returnType = nullptr;
  if (!node->isVoid)
    returnType = node->signature->return_type->toLLVMType(*this->context);
  else
    returnType = llvm::Type::getVoidTy(*this->context);

  llvm::FunctionType *FT = llvm::FunctionType::get(returnType, argTypes, false);

  llvm::Function *F = llvm::Function::Create(FT, llvm::Function::InternalLinkage,
    node->name, module.get());

  // Set argument names - first one is 'this' from parser
  size_t i = 0;
  for (auto &arg : F->args()) {
    arg.setName(node->args[i]->name);
    i++;
  }

  // CREATE FUNCTION BODY
  llvm::BasicBlock *BB = llvm::BasicBlock::Create(*context, "entry", F);
  builder->SetInsertPoint(BB);

  // Record the function arguments in the scope
  for (auto &arg : F->args()) {
    llvm::AllocaInst *alloca =
      createEntryBlockAlloca(F, arg.getType(), arg.getName());
    builder->CreateStore(&arg, alloca);
    currentScope->addSymbol(std::string(arg.getName()), alloca);
  }

  // Generate method body
  auto methodBody = node->body;
  cgresult_t returnValue;
  for (auto &el : methodBody->parts) {
    returnValue = visitDefault(el);
  }

  if (node->isVoid) {
    builder->CreateRetVoid();
  }

  verifyFunction(*F);

  currentScope = currentScope->prevScope();

  return cgnone;
}

cgvoid_t CodeGenVisitor::visit(const std::shared_ptr<FieldDecl> &node) {
  return cgnone;
}

cgvoid_t CodeGenVisitor::visit(const std::shared_ptr<ModuleDecl> &node) {
  cgresult_t result;
  currentScope = currentScope->nextScope(); // global scope -> module scope

  while (currentScope->getKind() != SCOPE_MODULE) currentScope = currentScope->nextScope();

  auto children = node->children;
  for (const auto &child : children) {
    result = visitDefault(child);
  }

  // Create the main function that will call Main's constructor
  llvm::FunctionType *FT = llvm::FunctionType::get(llvm::Type::getInt32Ty(*context), false);
  llvm::Function *F = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "main", module.get());

  // Create the entry block
  llvm::BasicBlock *BB = llvm::BasicBlock::Create(*context, "entry", F);
  builder->SetInsertPoint(BB);

  // Create Main instance
  auto mainType = llvm::StructType::getTypeByName(*context, "Main");
  auto mainAlloca = builder->CreateAlloca(mainType);
  
  // Call Main's constructor
  auto mainConstr = getFunction("Main_Create");
  builder->CreateCall(mainConstr, {mainAlloca});

  // Return 0
  builder->CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), 0));

  verifyFunction(*F);

  return cgnone;
}

cgvoid_t CodeGenVisitor::visit(const std::shared_ptr<VarDecl> &node) {
  llvm::Function *function = builder->GetInsertBlock()->getParent();

  std::string var_name = node->name;
  auto varType = cggettype(visit(node->type));
  auto initializer = node->initializer;
  llvm::AllocaInst *alloca;
  cgresult_t initVal;
  if (initializer) {
    initVal = visit(initializer);
    if (initializer->getKind() != E_Constructor_Call) {
      alloca = builder->CreateAlloca(varType, nullptr, var_name);
      builder->CreateStore(cggetval(initVal), alloca);
    } else {
      // For constructor calls, we already have the allocation
      alloca = cggetalloc(initVal);
    }
  }
  else {
    alloca = builder->CreateAlloca(varType, nullptr, var_name);
  }

  currentScope->addSymbol(var_name, alloca);

  return cgnone;
}

cgvoid_t CodeGenVisitor::visit(const std::shared_ptr<FuncDecl> &node) {
  // Skip the old main function
  if (node->name == "main") {
    return cgnone;
  }

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

  llvm::Function *F = llvm::Function::Create(FT,
    (node->getKind() == E_Main_Decl) ? llvm::Function::ExternalLinkage : llvm::Function::InternalLinkage,
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
    case E_For_Loop: {
      auto forAss = std::static_pointer_cast<ForSTMT>(node);
      visit(forAss);
    }
  }

  return cgnone;
}

cgvoid_t CodeGenVisitor::visit(const std::shared_ptr<ForSTMT> &node) {
  currentScope = currentScope->nextScope();

  // Generate initial assignment
  volatile auto startCode = visit(node->varWithAss);

  // Get the loop variable
  auto iteratorVar = node->varWithAss;
  auto [iterDecl, iterAlloca, iterInited] = *currentScope->getSymbol(iteratorVar->var_name);
  auto iteratorType = std::static_pointer_cast<VarDecl>(iterDecl)->type;
  auto iterTypeLLVM = iteratorType->toLLVMType(*this->context);

  // Create the loop header block
  llvm::Function *TheFunction = builder->GetInsertBlock()->getParent();
  llvm::BasicBlock *PreheaderBB = builder->GetInsertBlock();
  llvm::BasicBlock *LoopBB = llvm::BasicBlock::Create(*context, "loop", TheFunction);
  llvm::BasicBlock *AfterBB = llvm::BasicBlock::Create(*context, "afterloop", TheFunction);

  // Load initial value before creating phi node
  auto initialValue = builder->CreateLoad(iterTypeLLVM, iterAlloca);

  // Branch to loop header
  builder->CreateBr(LoopBB);

  // Start insertion in LoopBB
  builder->SetInsertPoint(LoopBB);

  // Create the PHI node for the loop variable
  llvm::PHINode *Variable = builder->CreatePHI(iterTypeLLVM, 2, iteratorVar->var_name);
  
  // Add the initial value from the preheader
  Variable->addIncoming(initialValue, PreheaderBB);

  // Store the phi node value back to the alloca
  builder->CreateStore(Variable, iterAlloca);

  // Generate loop body
  // volatile auto bodyCode = visitDefault(node->body);
  auto forBody = node->body;
  cgresult_t returnValue;
  for (auto &el : forBody->parts) {
    returnValue = visitDefault(el);
  }

  // Generate step code
  volatile auto stepCode = visit(node->post);

  // Load the current value after the step
  auto currentValue = builder->CreateLoad(iterTypeLLVM, iterAlloca);

  // Generate condition
  auto condCode = cggetval(visit(node->condition));

  // Add the current value as incoming to phi node
  Variable->addIncoming(currentValue, builder->GetInsertBlock());

  // Create conditional branch
  builder->CreateCondBr(condCode, LoopBB, AfterBB);

  // Start insertion in AfterBB
  builder->SetInsertPoint(AfterBB);

  currentScope = currentScope->prevScope();

  return cgnone;
}

cgvoid_t CodeGenVisitor::visit(const std::shared_ptr<AssignmentSTMT> &node) {
  llvm::Value *var = nullptr;
  std::string name;
  switch (node->assKind) {
  case VAR_ASS: {
    auto [_, alloc, isInited] = *currentScope->getSymbol(node->variable->var_name);
    var = alloc;
    name = node->variable->var_name;
    currentScope->markInitialized(name);
    break;
  }
  case FIELD_ASS:
    var = cggetval(visit(node->field));
    name = node->field->field_name;
    break;
  case EL_ASS:
    var = cggetval(visit(node->element));
    // name = node->element->; @TODO
    break;
  }

  llvm::Value *assignment = cggetval(visit(node->expression));

  builder->CreateStore(assignment, var);
  return cgnone;
}

cgvoid_t CodeGenVisitor::visit(const std::shared_ptr<ReturnSTMT> &node) {
  // @TODO: className + ...
  auto methodName = currentScope->getName();
  auto methodDecl =
    std::static_pointer_cast<MethodDecl>(currentScope->lookup(methodName));

  llvm::Value *retVal = cggetval(visit(node->expr));

  auto returnType = methodDecl->signature->return_type->toLLVMType(*context);
  if (retVal && retVal->getType()->isPointerTy()) {
    if (!returnType->isPointerTy()) {
      // Load the value from the pointer
      retVal = builder->CreateLoad(returnType, retVal);
    }
  }

  // if (retVal) {
  //   builder->CreateRet(retVal);
  // }

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

  if (llvm::verifyModule(*module, &llvm::errs())) {
    llvm::errs() << "Module verification failed!\n";
    exit(-1);
    // Handle error
  }

  pass.run(*module);
  dest.flush();

  llvm::outs() << "Wrote " << Filename << "\n";

  // clang linking
  std::string command = "clang -o a.out "+ Filename + " ";
  int status = system(command.c_str());
  if (status == -1) {
    CG_ERR("", "Linking failed");
  }
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

// llvm::StructType* CodeGenVisitor::getStructType(const std::string &name) {
//   auto allStructs = module->getIdentifiedStructTypes();
//   auto namedStruct = std::find_if(
//     allStructs.begin(),
//     allStructs.end(),
//     [&](const llvm::StructType *T) {T->getName() == name;}
//     );
//   if (namedStruct == allStructs.end()) return nullptr;
//   return *namedStruct;
// }

//#####=========================================#####
//#####=========================================#####
//#####=========================================#####

cgresult_t CodeGenVisitor::handleBuiltinMethodCall(
    const std::shared_ptr<MethodCallEXP> &node,
    std::string methodName) {
  
  // get left operand
  llvm::Value *L = cggetval(visit(node->left));
  if (!L)
    return cgnone;

  // only one arg for builtin methods
  if (node->arguments.size() != 1) {
    return cgnone;
  }

  // get right operand
  llvm::Value *R = cggetval(visit(node->arguments[0]));
  if (!R)
    return cgnone;

  // handle different methods
  if (methodName == "Plus") {
    return builder->CreateAdd(L, R, "addtmp");
  }
  else if (methodName == "Minus") {
    return builder->CreateSub(L, R, "subtmp");
  }
  else if (methodName == "Mult") {
    return builder->CreateMul(L, R, "multmp");
  }
  else if (methodName == "Div") {
    return builder->CreateSDiv(L, R, "divtmp");
  }
  else if (methodName == "Rem") {
    return builder->CreateSRem(L, R, "remtmp");
  }
  else if (methodName == "Less") {
    return builder->CreateICmpSLT(L, R, "cmptmp");
  }
  else if (methodName == "Greater") {
    return builder->CreateICmpSGT(L, R, "cmptmp");
  }
  else if (methodName == "Equal") {
    return builder->CreateICmpEQ(L, R, "cmptmp");
  }

  return cgnone;
}

cgresult_t CodeGenVisitor::visit(const std::shared_ptr<MethodCallEXP> &node) {
  // check if this is a built-in method call
  std::string className;
  if (node->left->getKind() == E_Var_Reference) {
    auto varRefLeft = std::static_pointer_cast<VarRefEXP>(node->left);
    auto leftDecl = std::static_pointer_cast<VarDecl>(currentScope->lookup(varRefLeft->var_name));
    className = leftDecl->type->name;
  }
  else if (node->left->getKind() == E_Integer_Literal) {
    auto intRefLeft = std::static_pointer_cast<IntLiteralEXP>(node->left);
    auto leftDecl = std::static_pointer_cast<VarDecl>(currentScope->lookup(
      std::to_string(intRefLeft->getValue())));
    className = leftDecl->type->name;
  }

  // @TODO optimize
  std::shared_ptr<Decl> decl;
  for (auto child : globalScope->getChildren()) {
    if (child->getKind() == SCOPE_MODULE_BUILTIN) {
      auto classBTScope = child->getChildren()[0]; // ? single module - single class : Integer - Integer
      decl = classBTScope->lookup(node->method_name);
    }
  }

  if (decl && decl->getKind() == E_Method_Decl) {
    auto methodDecl = std::static_pointer_cast<MethodDecl>(decl);
    if (methodDecl->isBuiltin) {
      return handleBuiltinMethodCall(node, node->method_name);
    }
  }

  // Regular method call handling
  auto varRef = std::static_pointer_cast<VarRefEXP>(node->left);
  auto [_, alloc, isInited] = *currentScope->getSymbol(varRef->var_name);
  std::vector<llvm::Value *> ArgsV;
  ArgsV.push_back(alloc); // Pass the pointer directly as 'this'

  for (unsigned i = 0, e = node->arguments.size(); i != e; ++i) {
    cgresult_t arg = visit(node->arguments[i]);
    ArgsV.push_back(cggetval(arg));

    if (!ArgsV.back())
      return cgnone;
  }

  llvm::Function *CalleeF = getFunction(node->method_name);
  if (!CalleeF)
    return cgnone;

  if (CalleeF->getReturnType()->isVoidTy()) {
    return builder->CreateCall(CalleeF, ArgsV);
  }
  else {
    return builder->CreateCall(CalleeF, ArgsV, "calltmp");
  }
}
