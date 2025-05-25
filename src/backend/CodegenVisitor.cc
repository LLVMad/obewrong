#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/StringRef.h"
#include "backend/CodegenVisitor.h"
#include "lld/Common/Driver.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/IntrinsicInst.h"

// #include "lld/Common/"

#include "util/Logger.h"

#include <complex>
#include <llvm/Support/Chrono.h>

#define CG_ERR(path, msg)                                                   \
ERR("%s: %s\n", path, msg)


void CodeGenVisitor::visit(Entity &node) {
  // // Because E_Kind enum values
  // // go in ORDER we can just check
  // // their numeric values
  // if (node.getKind() >= 0 && node.getKind() < 12) {
  //   return visit(std::static_pointer_cast<Decl>(node));
  // }
  //
  // if (node.getKind() >= 12 && node.getKind() < 17) {
  //   // types visit
  //   // can it happen actually?
  //   // i think they cannot appear as a
  //   // DISTINCT node themselfs
  //   return cgnone;
  // }
  // if (node.getKind() >= 16 && node.getKind() < 38) {
  //   return visit(std::static_pointer_cast<Expression>(node));
  // }
  // else {
  //   return visit(std::static_pointer_cast<Statement>(node));
  // }
  // return cgnone;
}

//#####===========================================#####
//#####=============== EXPRESSIONS ===============#####
//#####===========================================#####

void CodeGenVisitor::visit(Expression &node) {
  // ???
}

void CodeGenVisitor::visit(ConversionEXP &node) {
  auto fromExpr = node.from;

  // fromExpr->acceptAGuestVisitor(this);
  fromExpr->accept(*this);
  auto fromVal = lastValue;
  // if (!fromVal) return nul/lptr;

  auto fromType = fromExpr->resolveType(typeTable->types[moduleName], currentScope);
  auto toType = node.to;

  auto itof = fromType->kind == TYPE_INT;

  if (fromType->kind == TYPE_INT && toType->kind == TYPE_REAL) {
    lastValue = builder->CreateCast(
      llvm::CastInst::CastOps::SIToFP,
      fromVal,
      toType->toLLVMType(*context)
    );
  }
  else if (fromType->kind == TYPE_REAL && toType->kind == TYPE_INT) {
    lastValue = builder->CreateCast(
      llvm::CastInst::CastOps::FPToSI,
      fromVal,
      toType->toLLVMType(*context)
    );
  }
  else if (fromType->kind == TYPE_BYTE && toType->kind == TYPE_INT) {
    lastValue = builder->CreateSExt(fromVal, llvm::Type::getInt32Ty(*context));
  }

}

void CodeGenVisitor::visit(ElementRefEXP &node) {
  node.index->accept(*this);
  // load index value -> index is an Expresssion
  auto indexVal = lastValue;

  // convert index to i64 if it's not already
  if (indexVal->getType() != llvm::Type::getInt64Ty(*context)) {
    indexVal = builder->CreateSExt(indexVal, llvm::Type::getInt64Ty(*context));
  }

  // array type
  // @TODO: field as `arr`
  auto [arrDecl , arrAlloca, arrInited ] = *currentScope->getSymbol(node.arr->getName());

  // GEP with two indices: [0, index]
  auto zero = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), 0);
  lastValue = builder->CreateGEP(
    arrAlloca->getAllocatedType(),
    arrAlloca,
    {zero, indexVal}
  );

  // load GEP
  // auto arrType = std::static_pointer_cast<VarDecl>(arrDecl)->type;
  // auto el_type = std::static_pointer_cast<TypeArray>(arrType)->el_type;
  //
  // return builder->CreateLoad(
  //   el_type->toLLVMType(*context),
  //   gepToEl
  // );
}

void CodeGenVisitor::visit(AssignmentWrapperEXP &node) {
  node.assignment->accept(*this);
}


// a.x
void CodeGenVisitor::visit(FieldRefEXP &node) {
  std::shared_ptr<Type> varType = nullptr;
  llvm::AllocaInst* alloca = nullptr;
  bool isInherited = false;

  if (node.obj) {
    auto [decl, temp_alloca, isInited] = *currentScope->getSymbol(node.obj->getName());

    alloca = temp_alloca;
    varType = decl->resolveType(typeTable->types[moduleName], currentScope);

    // check if inherited
    if (varType->kind == TYPE_ACCESS) {
      auto className = currentScope->prevScope()->getName();
      auto fieldName = node.getName(); // get field name
      auto [classDecl, _, __] = *currentScope->getSymbol<ClassDecl>(className);
      if (!std::any_of(classDecl->fields.begin(), classDecl->fields.end(),
        [&](auto &f) { return f->getName() == fieldName; }))
      {
        // first load a copy
        isInherited = true;
      }
    }
    else {
      auto fieldName = node.getName(); // get field name
      auto [classDecl, _, __] = *currentScope->getSymbol<ClassDecl>(varType->name);
      if (!std::any_of(classDecl->fields.begin(), classDecl->fields.end(),
        [&](auto &f) { return f->getName() == fieldName; }))
      {
        // first load a copy
        isInherited = true;
      }
    }

  }
  else if (node.el) {
    auto var_ref = node.el->arr;
    auto obj_decl = std::static_pointer_cast<VarDecl>(
      currentScope->lookup(var_ref->getName()));

    node.el->accept(*this);
    // alloca = lastValue;
    varType = std::static_pointer_cast<TypeArray>(obj_decl->type)->el_type;
  }

  if (varType->kind == TYPE_ACCESS) {
    auto ptrType = std::static_pointer_cast<TypeAccess>(varType);
    auto classTypeLLVM = ptrType->to->toLLVMType(*context);
    auto ptrAlloca = builder->CreateLoad(alloca->getAllocatedType(), alloca);

    // construct GEP
    lastValue = builder->CreateStructGEP(
      classTypeLLVM,
      ptrAlloca,
      node.index,
      node.getName()
    );
  } else {
    auto classTypeLLVM = varType->toLLVMType(*context);

    if (isInherited) {
      // GEP to class copy
      lastValue = builder->CreateStructGEP(
        classTypeLLVM,
        alloca == nullptr ? lastValue : alloca,
        0,
        node.getName()
      );

      // load class copy
      // lastValue = builder->CreateLoad(
      //   classTypeLLVM->getContainedType(0),
      //   lastValue
      // );

      // gep to base field
      // lastValue = builder->CreateStructGEP(
      //   classTypeLLVM->getContainedType(0),
      //   lastValue,
      //   node.index
      // );
    }
    else {
      // construct GEP
      lastValue = builder->CreateStructGEP(
        classTypeLLVM,
        alloca == nullptr ? lastValue : alloca,
        node.index,
        node.getName()
      );
    }
  }
}

void CodeGenVisitor::visit(FuncCallEXP &node) {
  // Look up the name in the global module table.
  llvm::Function *CalleeF = getFunction(node.getName());
  // if (!CalleeF)
  //   return cgnone;
    // return LogErrorV("Unknown function referenced");

  std::vector<llvm::Value *> ArgsV;
  auto Args = node.arguments;
  // auto FArgs = CalleeF->args();
  for (unsigned i = 0, e = Args.size(); i != e; ++i) {
    // void arg = visit(Args[i]);
    Args[i]->accept(*this);
    auto val = unwrapPointerReference(Args[i].get(), lastValue);

    // load an argument
    // @TODO probably shoulnt do it here???
    // becouse we shouldnt do load:
    // - in assignment
    // val = cggetval(unwrapPointerReference(Args[i], val));
    // if (Args[i]->getKind() == E_Field_Reference) {
    //   auto fieldRef = std::static_pointer_cast<FieldRefEXP>(Args[i]);
    //   auto [decl, alloca, isInited] = *currentScope->getSymbol(fieldRef->obj->getName());
    //   auto varDecl = std::static_pointer_cast<VarDecl>(decl);
    //
    //   auto classTypeLLVM = varDecl->type->toLLVMType(*context);
    //   val = builder->CreateLoad(
    //     classTypeLLVM->getStructElementType(fieldRef->index),
    //     val
    //   );
    // }
    // // @TODO costil (needs a general case)
    // if (Args[i]->getKind() == E_Element_Reference) {
    //   auto elementRef = std::static_pointer_cast<ElementRefEXP>(Args[i]);
    //   auto arrType = std::static_pointer_cast<VarDecl>(currentScope->lookup(elementRef->arr->getName()))->type;
    //   auto el_type = std::static_pointer_cast<TypeArray>(arrType)->el_type;
    //
    //   val = builder->CreateLoad(
    //     el_type->toLLVMType(*context),
    //     val
    //   );
    // }

    // @TODO
    // if (val->getType()->isPointerTy()) {
    //   val = builder->CreateLoad(
    //     val->getType()->getStructElementType(),
    //     val
    //   );
    // }

    ArgsV.push_back(val);

    if (!ArgsV.back())
      return;
  }

  if (CalleeF->getReturnType()->isVoidTy()) {
    lastValue = builder->CreateCall(CalleeF, ArgsV);
  }
  else
    lastValue = builder->CreateCall(CalleeF, ArgsV, "calltmp");
}

void CodeGenVisitor::visit(ConstructorCallEXP &node) {
  std::vector<llvm::Value *> ArgsV;
  auto Args = node.arguments;

  // Create a pointer to the class instance

  // @TODO ?!
  // llvm::Type *classType;
  // if (node->left->getName() == "Integer") {
  //   classType = typeTable->getType(moduleName, "Integer");
  // }

  llvm::Type *classType = llvm::StructType::getTypeByName(*context, node.left->getName());

  // @TODO
  if (!classType) {
    classType = typeTable->getType(moduleName, "Integer")->toLLVMType(*context);
  }

  auto objInstanceRef = builder->CreateAlloca(classType);
  ArgsV.push_back(objInstanceRef);

  std::string typeNames;
  for (unsigned i = 0, e = Args.size(); i != e; ++i) {
    // void arg = visit(Args[i]);
    Args[i]->accept(*this);
    ArgsV.push_back(lastValue);

    typeNames += Args[i]->resolveType(typeTable->types[moduleName], currentScope)->name;

    // i wish we could just do
    // ... Args[i]->resolveType()...
    // NOW WE CAN!!!!!!!! yeeaaaahhh
    // T_T

    if (!ArgsV.back())
      return;
  }

  // Look up the name in the global module table.
  auto constrName = node.left->getName() + "_Create" + typeNames;

  llvm::Function *CalleeF = getFunction(constrName);
  if (!CalleeF)
    return;

  if (CalleeF->getReturnType()->isVoidTy()) {
    builder->CreateCall(CalleeF, ArgsV);
  }
  else
    builder->CreateCall(CalleeF, ArgsV, "calltmp");

  // return objInstanceRef;
  lastValue = objInstanceRef;
}

void CodeGenVisitor::visit(StringLiteralEXP &node) {
  std::string processed;
  for (size_t i = 0; i < node.value.length(); i++) {
    if (node.value[i] == '\\' && i + 1 < node.value.length()) {
      switch (node.value[i + 1]) {
        case 'n': processed += '\n'; break;
        case 't': processed += '\t'; break;
        case 'r': processed += '\r'; break;
        case '\\': processed += '\\'; break;
        case '"': processed += '"'; break;
        default: processed += node.value[i + 1]; break;
      }
      i++; // skip next
    } else {
      processed += node.value[i];
    }
  }
  lastValue = builder->CreateGlobalString(processed);
}

void CodeGenVisitor::visit(IntLiteralEXP &node) {
  switch (node.getByteSize()) {
    case 8: {
      lastValue = llvm::ConstantInt::getSigned((llvm::Type::getInt8Ty(*context)),
                                    node.getValue());
      break;
    }
    case 16: {
      lastValue = llvm::ConstantInt::getSigned((llvm::Type::getInt16Ty(*context)), node.getValue());
      break;
    }
    case 32: {
      lastValue = llvm::ConstantInt::getSigned((llvm::Type::getInt32Ty(*context)), node.getValue());
      break;
    }
    case 64: {
      lastValue = llvm::ConstantInt::getSigned((llvm::Type::getInt64Ty(*context)), node.getValue());
      break;
    }
    default: {
      lastValue = llvm::ConstantInt::getSigned((llvm::Type::getInt64Ty(*context)),
                                  node.getValue());
      break;
    }
  }

}

void CodeGenVisitor::visit(RealLiteralEXP &node) {
  lastValue = llvm::ConstantFP::get(*context, llvm::APFloat(node.getValue()));
}

void CodeGenVisitor::visit(ArrayLiteralExpr &node) {
  auto elType = typeTable->getType(moduleName, node.el_type);

  if(node.el_type == TYPE_UNKNOWN) {
    // resolve type manually
    elType = node.elements[0]->resolveType(typeTable->types[moduleName], currentScope);
  
    //if(elType->kind == 

  }

  auto arrayTypeLLVM = llvm::ArrayType::get(elType->toLLVMType(*context), node.elements.size());

  // create constant elements
  std::vector<llvm::Constant *> elements;
  for (const auto& el: node.elements) {
    // auto valToConstant = dyn_cast<llvm::Constant>(cggetval(visit(el)));
    el->accept(*this);
    auto valToConstant = dyn_cast<llvm::Constant>(lastValue);
    elements.push_back(valToConstant);
  }

  // create global array
  auto constArray = llvm::ConstantArray::get(arrayTypeLLVM, elements);
  auto globalArray = new llvm::GlobalVariable(
    *module,
    arrayTypeLLVM,
    true,  // isConstant
    llvm::GlobalValue::LinkageTypes::ExternalLinkage,
    constArray
  );

  // create local array
  auto localArray = builder->CreateAlloca(arrayTypeLLVM, nullptr);

  // calc size in bytes
  auto size = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context),
    node.elements.size() * elType->toLLVMType(*context)->getPrimitiveSizeInBits() / 8);

  // cast to i8*
  auto destPtr = builder->CreateBitCast(localArray, llvm::PointerType::get(llvm::Type::getInt8Ty(*context), 0));
  auto srcPtr = builder->CreateBitCast(globalArray, llvm::PointerType::get(llvm::Type::getInt8Ty(*context), 0));

  // copy data
  std::vector<llvm::Type*> types = {
    llvm::PointerType::get(llvm::Type::getInt8Ty(*context), 0),
    llvm::PointerType::get(llvm::Type::getInt8Ty(*context), 0),
    llvm::Type::getInt64Ty(*context)
  };
  auto memcpyFn = getOrInsertDeclaration(module.get(), llvm::Intrinsic::memcpy, types);
  builder->CreateCall(memcpyFn, {destPtr, srcPtr, size, llvm::ConstantInt::getFalse(*context)});

  lastValue = localArray;
}

void CodeGenVisitor::visit(VarRefEXP &node) {
  // llvm::AllocaInst *alloca = currentScope->lookupAlloca(node.getName()); /* varEnv[node.getName()]; */
  // bool isInited = currentScope->isDeclInitialized(no);
  auto [_, alloc, isInited] = *currentScope->getSymbol(node.getName());

  // if (!isInited) {
  //   return alloc;
  // }

  lastValue = builder->CreateLoad(alloc->getAllocatedType(), alloc, node.getName().c_str());
}

void CodeGenVisitor::visit(BinaryOpEXP &node) {
  node.left->accept(*this);
  llvm::Value *L = lastValue;

  node.right->accept(*this);
  llvm::Value *R = lastValue;
  if (!L || !R)
    return;

  switch (node.op) {
  case OP_PLUS:
    lastValue = builder->CreateAdd(L, R, "addtmp");
    break;
  case OP_MINUS:
    lastValue = builder->CreateSub(L, R, "subtmp");
    break;
  case OP_MULTIPLY:
    lastValue = builder->CreateMul(L, R, "multmp");
  case OP_LESS:
    L = builder->CreateFCmpULT(L, R, "cmptmp");
    // Convert bool 0/1 to double 0.0 or 1.0
    lastValue = builder->CreateUIToFP(L, llvm::Type::getDoubleTy(*context),
                                 "booltmp");
    break;
  default:
    return;
  }
}

//#####============================================#####
//#####=============== DECLARATIONS ===============#####
//#####============================================#####

void CodeGenVisitor::visit(Decl &node) {
  // ???
}

void CodeGenVisitor::visit(ClassDecl &node) {
  currentScope = currentScope->nextScope();

  auto classType = llvm::StructType::create(*context, llvm::StringRef(node.getName()));

  auto classSignature = node.type;
  std::vector<llvm::Type*> fieldTypes;
  for (auto &field : node.fields) {
    // add fields types
    fieldTypes.push_back(field->type->toLLVMType(*context));
  }

  classType->setBody(llvm::ArrayRef(fieldTypes));

  for(auto &method : node.methods) {
    method->accept(*this);
  }

  currentScope = currentScope->prevScope();
}

void CodeGenVisitor::visit(ConstrDecl &node) {
  currentScope = currentScope->nextScope();

  // CREATE PROTOTYPE OF A FUNCTION
  std::vector<llvm::Type*> argTypes;
  std::string typeNames;
  for (auto &arg : node.args) {
    argTypes.push_back(arg->type->toLLVMType(*this->context));

    // typeNames += arg->resolveType(typeTable->types[moduleName], currentScope)->name;
  }

  llvm::Type *returnType = llvm::Type::getVoidTy(*this->context);

  llvm::FunctionType *FT = llvm::FunctionType::get(returnType, argTypes, false);

  std::string constrName = node.getName() /* + typeNames */;
  llvm::Function *F = llvm::Function::Create(FT, llvm::Function::ExternalLinkage,
    node.getName(), module.get());

  size_t i = 0;
  for (auto &arg : F->args()) {
    arg.setName(node.args[i]->getName());
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
  auto funcBody = node.body;
  for (auto &el : funcBody->parts) {
    el->accept(*this);
  }
  builder->CreateRetVoid();

  verifyFunction(*F);

  currentScope = currentScope->prevScope();
}

void CodeGenVisitor::visit(MethodDecl &node) {
  currentScope = currentScope->nextScope();

  // CREATE PROTOTYPE OF A FUNCTION
  std::vector<llvm::Type*> argTypes;

  // 'this' which was added by the parser
  for (auto &arg : node.args) {
    argTypes.push_back(arg->type->toLLVMType(*this->context));
  }

  llvm::Type *returnType = nullptr;
  if (!node.isVoid)
    returnType = node.signature->return_type->toLLVMType(*this->context);
  else
    returnType = llvm::Type::getVoidTy(*this->context);

  llvm::FunctionType *FT = llvm::FunctionType::get(returnType, argTypes, false);

  llvm::Function *F = llvm::Function::Create(FT, llvm::Function::ExternalLinkage,
    node.getName(), module.get());

  // Set argument names - first one is 'this' from parser
  size_t i = 0;
  for (auto &arg : F->args()) {
    arg.setName(node.args[i]->getName());
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
  auto methodBody = node.body;
  for (auto &el : methodBody->parts) {
    el->accept(*this);
  }

  if (node.isVoid) {
    builder->CreateRetVoid();
  }

  verifyFunction(*F);

  currentScope = currentScope->prevScope();
}

void CodeGenVisitor::genericMethodDecl(MethodDecl &node) {
  // currentScope = currentScope->nextScope();

  // CREATE PROTOTYPE OF A FUNCTION
  std::vector<llvm::Type*> argTypes;

  // Add all parameters including 'this' which was added by the parser
  for (auto &arg : node.args) {
    argTypes.push_back(arg->type->toLLVMType(*this->context));
  }

  llvm::Type *returnType = nullptr;
  if (!node.isVoid)
    returnType = node.signature->return_type->toLLVMType(*this->context);
  else
    returnType = llvm::Type::getVoidTy(*this->context);

  llvm::FunctionType *FT = llvm::FunctionType::get(returnType, argTypes, false);

  llvm::Function *F = llvm::Function::Create(FT, llvm::Function::ExternalLinkage,
    node.getName(), module.get());

  // Set argument names - first one is 'this' from parser
  size_t i = 0;
  for (auto &arg : F->args()) {
    arg.setName(node.args[i]->getName());
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
  auto methodBody = node.body;
  for (auto &el : methodBody->parts) {
    el->accept(*this);
  }

  if (node.isVoid) {
    builder->CreateRetVoid();
  }

  verifyFunction(*F);

  currentScope = currentScope->prevScope();
}

void CodeGenVisitor::visit(FieldDecl &node) {
  // nothing to do here all the work is in CLassDel visit method
}

void CodeGenVisitor::visit(ModuleDecl &node) {
  currentScope = currentScope->nextScope(); // global scope -> module scope

  while (currentScope->getKind() != SCOPE_MODULE
    && currentScope->getName() != node.getName()) currentScope = currentScope->nextScope();

  auto children = node.children;
  for (const auto &child : children) {
    child->accept(*this);
  }

  // Create MAIN() that will call Main class constructor !
  auto mainType = llvm::StructType::getTypeByName(*context, "Main");
  if (mainType) {
    llvm::FunctionType *FT = llvm::FunctionType::get(llvm::Type::getInt32Ty(*context), false);
    llvm::Function *F = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "main", module.get());

    llvm::BasicBlock *BB = llvm::BasicBlock::Create(*context, "entry", F);
    builder->SetInsertPoint(BB);

    auto mainAlloca = builder->CreateAlloca(mainType);

    auto mainConstr = getFunction("Main_Create");
    builder->CreateCall(mainConstr, {mainAlloca});

    builder->CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), 0));

    verifyFunction(*F);

  }
  // ->> not a main module
}

void CodeGenVisitor::visit(VarDecl &node) {
  llvm::Function *function = builder->GetInsertBlock()->getParent();

  std::string var_name = node.getName();
  auto varType = node.type->toLLVMType(*this->context);
  auto initializer = node.initializer;
  llvm::AllocaInst *alloca;
  llvm::Value* initVal;
  if (initializer) {
    initializer->accept(*this);
    initVal = lastValue;
    if (initializer->getKind() != E_Constructor_Call && initializer->getKind() != E_Array_Literal) {

      // if initVal is a pointer (EL_REF, GEP)
      llvm::Value* initValUnwrap = unwrapPointerReference(initializer.get(), initVal);

      alloca = builder->CreateAlloca(varType, nullptr, var_name);
      builder->CreateStore(initValUnwrap, alloca);
    } else {
      // For constructor calls, we already have the allocation
      alloca = dyn_cast<llvm::AllocaInst>(initVal); // @TODO look into ALLOCAJUMP
    }
  }
  // @TODO: if no initialzer, check if thats allowed
  else {
    alloca = builder->CreateAlloca(varType, nullptr, var_name);
  }

  currentScope->addSymbol(var_name, alloca);
}

void CodeGenVisitor::visit(FuncDecl &node) {
  currentScope = currentScope->nextScope();

  // CREATE PROTOTYPE OF A FUNCTION
  std::vector<llvm::Type*> argTypes;
  for (auto &arg : node.args) {
    argTypes.push_back(arg->type->toLLVMType(*this->context));
  }

  llvm::Type *returnType = nullptr;
  if (!node.isVoid)
    returnType = node.signature->return_type->toLLVMType(*this->context);
  else
    returnType = llvm::Type::getVoidTy(*this->context);

  llvm::FunctionType *FT = llvm::FunctionType::get(returnType, argTypes, false);

  llvm::Function *F = llvm::Function::Create(FT,
    (node.getKind() == E_Main_Decl) ? llvm::Function::ExternalLinkage : llvm::Function::ExternalLinkage,
    node.getName(), module.get());

  size_t i = 0;
  for (auto &arg : F->args()) {
    arg.setName(node.args[i]->getName());
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
  auto funcBody = node.body;
  for (auto &el : funcBody->parts) {
    el->accept(*this);
  }

  if (node.isVoid) {
    builder->CreateRetVoid();
  }

  verifyFunction(*F);

  currentScope = currentScope->prevScope();
};


//#####===========================================#####
//#####=============== STATEMENTS ================#####
//#####===========================================#####

void CodeGenVisitor::visit(Statement &node) {
  // ???
}

void CodeGenVisitor::visit(IfSTMT &node) {
  currentScope = currentScope->nextScope();

  // gen condition first startCode
  node.condition->accept(*this);
  auto startCode = lastValue;
  auto type = node.condition->resolveType(typeTable->types[moduleName], currentScope);

  llvm::Function *TheFunction = builder->GetInsertBlock()->getParent();

  // Create blocks for the then and else cases.  Insert the 'then' block at the
  // end of the function.
  llvm::BasicBlock *ThenBB =
      llvm::BasicBlock::Create(*context, "then", TheFunction);
  llvm::BasicBlock *ElseBB = llvm::BasicBlock::Create(*context, "else");
  llvm::BasicBlock *MergeBB = llvm::BasicBlock::Create(*context, "ifcont");
  builder->CreateCondBr(startCode, ThenBB, ElseBB);

  builder->SetInsertPoint(ThenBB);

  // if True body
  // volatile auto thenCode = visit(node->ifTrue);
  auto trueBody = node.ifTrue;
  for (auto &part : trueBody->parts) {
    part->accept(*this);
  }

  builder->CreateBr(MergeBB);

  ThenBB = builder->GetInsertBlock();

  // Emit else block.
  TheFunction->insert(TheFunction->end(), ElseBB);
  builder->SetInsertPoint(ElseBB);

  // gen Else
  llvm::Value *elsB;
  if (node.ifFalse) {
    node.ifFalse->accept(*this);
    // auto blockFalse = std::static_pointer_cast<Block>(node.ifFalse);
    // for (auto &part : blockFalse->parts) {
      // part->accept(*this);
    // }
  }

  builder->CreateBr(MergeBB);
  // codegen of 'Else' can change the current block, update ElseBB for the PHI.
  ElseBB = builder->GetInsertBlock();

  // Emit merge block.
  TheFunction->insert(TheFunction->end(), MergeBB);
  builder->SetInsertPoint(MergeBB);
}

void CodeGenVisitor::visit(ForSTMT &node) {
  currentScope = currentScope->nextScope();

  // initial assignment startCode
  node.varWithAss->accept(*this);

  // get loop variable
  auto iteratorVar = node.varWithAss;
  auto [iterDecl, iterAlloca, iterInited] = *currentScope->getSymbol<VarDecl>(iteratorVar->getName());
  auto iteratorType = iterDecl->type;
  auto iterTypeLLVM = iteratorType->toLLVMType(*this->context);

  // loop header block
  llvm::Function *TheFunction = builder->GetInsertBlock()->getParent();
  llvm::BasicBlock *PreheaderBB = builder->GetInsertBlock();
  llvm::BasicBlock *LoopBB = llvm::BasicBlock::Create(*context, "loop", TheFunction);
  llvm::BasicBlock *AfterBB = llvm::BasicBlock::Create(*context, "afterloop", TheFunction);

  // load value for phi node
  auto initialValue = builder->CreateLoad(iterTypeLLVM, iterAlloca);

  // Branch _> header
  builder->CreateBr(LoopBB);

  // start insertion -> LoopBB
  builder->SetInsertPoint(LoopBB);

  // Create the PHI node <= the loop variable
  llvm::PHINode *Variable = builder->CreatePHI(iterTypeLLVM, 2, iteratorVar->getName());

  // add the initial value from the
  Variable->addIncoming(initialValue, PreheaderBB);

  // store the phi node value back to the alloca
  builder->CreateStore(Variable, iterAlloca);

  // generate loop body
  // volatile auto bodyCode = visitDefault(node->body);
  auto forBody = node.body;
  for (auto &el : forBody->parts) {
    el->accept(*this);
  }

  // Generate step code stepCode
  node.post->accept(*this);

  // Load the current value after the step
  auto currentValue = builder->CreateLoad(iterTypeLLVM, iterAlloca);

  // Generate condition condCode
  node.condition->accept(*this);
  auto condCode = lastValue;

  // Add the current value as incoming to phi node
  Variable->addIncoming(currentValue, builder->GetInsertBlock());

  // Create conditional branch
  builder->CreateCondBr(condCode, LoopBB, AfterBB);

  // Start insertion in AfterBB
  builder->SetInsertPoint(AfterBB);

  currentScope = currentScope->prevScope();
}

void CodeGenVisitor::visit(AssignmentSTMT &node) {
  llvm::Value *var = nullptr;
  std::string name;
  switch (node.assKind) {
  case VAR_ASS: {
    auto [_, alloc, isInited] = *currentScope->getSymbol(node.variable->getName());
    var = alloc;
    name = node.variable->getName();
    currentScope->markInitialized(name);
    break;
  }
  case FIELD_ASS:
    // var = cggetval(visit(node->field));
    node.field->accept(*this);
    var = lastValue;
    name = node.field->getName();
    break;
  case EL_ASS:
    // var = cggetval(visit(node->element));
    node.element->accept(*this);
    var = lastValue;
    // name = node->element->; @TODO
    break;
  }

  // llvm::Value *assignment = cggetval(visit(node->expression));
  node.expression->accept(*this);
  // llvm::Value* assignment = lastValue;

  auto assignment = unwrapPointerReference(node.expression.get(), lastValue);

  builder->CreateStore(assignment, var);
}

void CodeGenVisitor::visit(ReturnSTMT &node) {
  // @TODO: className + ...
  auto methodName = currentScope->getName();
  auto methodDecl =currentScope->lookup<MethodDecl>(methodName);

  node.expr->accept(*this);
  llvm::Value *retVal = lastValue;

  auto returnType = methodDecl->signature->return_type->toLLVMType(*context);
  if (retVal && retVal->getType()->isPointerTy()) {
    if (!returnType->isPointerTy()) {
      // Load the value from the pointer
      retVal = builder->CreateLoad(returnType, retVal);
    }
  }

  builder->CreateRet(retVal);
}

//#####=========================================#####
//#####=============== COMPILING ===============#####
//#####=========================================#####

void CodeGenVisitor::createObjFile() {
  // LINK MODULES

  // ============

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
  std::string command = "clang -o "+ Filename + "out " +  Filename + " ";
  int status = system(command.c_str());
  if (status == -1) {
    CG_ERR("", "Linking failed");
  }

  // Debug logging to verify module transfer
  llvm::outs() << "Transferring module to SourceManager...\n";
  sm.addCompiledModule(*buff, std::move(module));
  llvm::outs() << "Module transferred successfully\n";
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

void CodeGenVisitor::handleBuiltinMethodCall(
    MethodCallEXP &node,
    std::string methodName) {

  // get left operand
  node.left->accept(*this);
  llvm::Value *L = unwrapPointerReference(node.left.get(), lastValue);
  if (!L)
    return;

  // only one arg for builtin methods
  // if (node->arguments.size() != 1) {
  //   return cgnone;
  // }

  // get right operand (can be void) ?
  llvm::Value *R;
  if (!node.arguments.empty()) {
    node.arguments[0]->accept(*this);
    R = unwrapPointerReference(node.arguments[0].get(), lastValue);
    // if (!R)
    //   return cgnone;
  }

  auto leftType = node.left->resolveType(typeTable->types[moduleName], currentScope);
  auto className = leftType->name;

  // handle different methods
  if (methodName == "Plus") {
    if (className == "Integer") {
      lastValue = builder->CreateAdd(L, R, "addtmp");
    } else if (className == "Real") {
      lastValue = builder->CreateFAdd(L, R, "faddtmp");
    }
  }
  else if (methodName == "Minus") {
    if (className == "Integer") {
      lastValue = builder->CreateSub(L, R, "subtmp");
    } else if (className == "Real") {
      lastValue = builder->CreateFSub(L, R, "fsubtmp");
    }
  }
  else if (methodName == "Mult") {
    if (className == "Integer") {
      lastValue = builder->CreateMul(L, R, "multmp");
    } else if (className == "Real") {
      lastValue = builder->CreateFMul(L, R, "fmultmp");
    }
  }
  else if (methodName == "Div") {
    if (className == "Integer") {
      lastValue = builder->CreateSDiv(L, R, "divtmp");
    } else if (className == "Real") {
      lastValue = builder->CreateFDiv(L, R, "fdivtmp");
    }
  }
  else if (methodName == "Rem") {
    if (className == "Integer") {
      lastValue = builder->CreateSRem(L, R, "remtmp");
    } else if (className == "Real") {
      lastValue = builder->CreateFRem(L, R, "fremtmp");
    }
  }
  else if (methodName == "Less") {
    if (className == "Integer") {
      lastValue = builder->CreateICmpSLT(L, R, "cmptmp");
    } else if (className == "Real") {
      lastValue = builder->CreateFCmpOLT(L, R, "fcmptmp");
    }
  }
  else if (methodName == "Greater") {
    if (className == "Integer") {
      lastValue = builder->CreateICmpSGT(L, R, "cmptmp");
    } else if (className == "Real") {
      lastValue = builder->CreateFCmpOGT(L, R, "fcmptmp");
    }
  }
  else if (methodName == "Equal") {
    if (className == "Integer") {
      lastValue = builder->CreateICmpEQ(L, R, "cmptmp");
    } else if (className == "Real") {
      lastValue = builder->CreateFCmpOEQ(L, R, "fcmptmp");
    }
  }
  else if (methodName == "UnaryMinus") {
    if (className == "Integer") {
      auto one = llvm::ConstantInt::getSigned((llvm::Type::getInt32Ty(*context)), -1);
      lastValue = builder->CreateMul(L, one, "uminus");
    } else if (className == "Real") {
      auto negOne = llvm::ConstantFP::get(llvm::Type::getFloatTy(*context), -1.0);
      lastValue = builder->CreateFMul(L, negOne, "fuminus");
    }
  }
}

void CodeGenVisitor::visit(MethodCallEXP &node) {
  // check if this is a built-in method call
  std::string className;
  if (node.left->getKind() == E_Var_Reference) {
    auto leftDecl = currentScope->lookup<VarDecl>(node.left->getName());
    className = leftDecl->type->name;
  }
  else if (node.left->getKind() == E_Integer_Literal) {
    auto intRefLeft = std::static_pointer_cast<IntLiteralEXP>(node.left);
    auto leftDecl = currentScope->lookup<VarDecl>(
      std::to_string(intRefLeft->getValue()));
    className = leftDecl->type->name;
  }

  // @TODO optimize
  std::shared_ptr<MethodDecl> decl;
  for (auto child : globalScope->getChildren()) {
    if (child->getKind() == SCOPE_MODULE_BUILTIN) {
      auto classBTScope = child->getChildren()[0]; // ? single module - single class : Integer - Integer
      decl =
        classBTScope->lookup<MethodDecl>(node.getName());
    }
  }

  if (decl && decl->getKind() == E_Method_Decl) {
    if (decl->isBuiltin) {
      return handleBuiltinMethodCall(node, node.getName());
    }
  }

  auto varRef = std::static_pointer_cast<VarRefEXP>(node.left);
  auto [_, alloc, isInited] = *currentScope->getSymbol(varRef->getName());
  std::vector<llvm::Value *> ArgsV;
  ArgsV.push_back(alloc); // Pass the pointer directly as 'this'

  for (unsigned i = 0, e = node.arguments.size(); i != e; ++i) {
    node.arguments[i]->accept(*this);
    auto arg = lastValue;
    ArgsV.push_back(arg);

    if (!ArgsV.back())
      return;
  }

  llvm::Function *CalleeF = getFunction(className + "_" + node.getName());
  if (!CalleeF)
    return;

  if (CalleeF->getReturnType()->isVoidTy()) {
    lastValue = builder->CreateCall(CalleeF, ArgsV);
  }
  else {
    lastValue = builder->CreateCall(CalleeF, ArgsV, "calltmp");
  }
}

llvm::Value*
CodeGenVisitor::unwrapPointerReference(Expression *node, llvm::Value *val) {
  if (!val->getType()->isPointerTy()) return val;

  switch (node->getKind()) {
    case E_Element_Reference: {
      auto elementRef = static_cast<ElementRefEXP*>(node);
      auto arrType = currentScope->lookup<VarDecl>(elementRef->arr->getName())->type;

      std::shared_ptr<Type> el_type;
      switch (arrType->kind) {
        case TYPE_ARRAY: {
          el_type = std::static_pointer_cast<TypeArray>(arrType)->el_type;
          break;
        }
        case TYPE_ACCESS: {
          el_type = std::static_pointer_cast<TypeAccess>(arrType)->to;
          break;
        }
      }

      val = builder->CreateLoad(
        el_type->toLLVMType(*context),
        val
      );
    } break;
    case E_Var_Reference: {

    } break;
    case E_Field_Reference: {
      std::shared_ptr<Type> type = nullptr;
      llvm::AllocaInst* alloca = nullptr;

      auto fieldRef = static_cast<FieldRefEXP*>(node);
      if (fieldRef->obj) {
        auto [varDecl, temp_alloca, isInited] = *currentScope->getSymbol(fieldRef->obj->getName());
        alloca = temp_alloca;
        type = varDecl->resolveType(typeTable->types[moduleName], currentScope);
      }
      else if (fieldRef->el) {
        auto var_ref = fieldRef->el->arr;
        auto obj_decl = std::static_pointer_cast<VarDecl>(
          currentScope->lookup(var_ref->getName()));

        // fieldRef->el->accept(*this);
        // alloca = lastValue;
        type = std::static_pointer_cast<TypeArray>(obj_decl->type)->el_type;
      }

      // auto [varDecl, alloca, isInited] = *currentScope->getSymbol<VarDecl>(fieldRef->obj->getName());

      // type = varDecl->resolveType(typeTable->types[moduleName], currentScope);
      if (type->kind == TYPE_ACCESS) {
        type = std::static_pointer_cast<TypeAccess>(type)->to;
      }

      auto classTypeLLVM = type->toLLVMType(*context);
      size_t field_index = fieldRef->index;

      // if inherited
      auto base = std::dynamic_pointer_cast<TypeClass>(type)->base_class;
      if (base) {
        auto baseDecl = currentScope->lookup<ClassDecl>(base->name);

        if (std::ranges::any_of(
          baseDecl->fields,
          [&](auto f) { return f->getName() == fieldRef->getName(); })
          ) {
          // its an inherited
          // @TODO sem check overriding existing fields
          classTypeLLVM = base->toLLVMType(*context);
        }

      }

      val = builder->CreateLoad(
        classTypeLLVM->getStructElementType(field_index),
        val
      );
    } break;
    case E_Constructor_Call: {
      auto obj_ref = static_cast<ConstructorCallEXP*>(node);
      auto className = obj_ref->left->getName();

      auto classTypeLLVM = 
        currentScope->lookup(className)->resolveType(typeTable->types[moduleName], currentScope)->toLLVMType(*context);
      

      val = builder->CreateLoad(
        classTypeLLVM,
        val
      );
    } break;
  }

  return val;
}

// UNUSED

// void CodeGenVisitor::visit(Block& block) {
//   // ???
// }

void CodeGenVisitor::visit(BoolLiteralEXP &node) {
  lastValue = llvm::ConstantInt::get(llvm::Type::getInt1Ty(*context), node.getValue());
}

void CodeGenVisitor::visit(ClassNameEXP &node) {
  // ??? 
  lastValue = nullptr;
}

void CodeGenVisitor::visit(CompoundEXP &node) {
  for (auto &expr : node.parts) {
    expr->accept(*this);
  }
}

void CodeGenVisitor::visit(ThisEXP &node) {
  // auto func = builder->GetInsertBlock()->getParent();
  // lastValue = func->getArg(0); // First argument is 'this'
}

void CodeGenVisitor::visit(ParameterDecl &node) {
  lastValue = nullptr;
}

void CodeGenVisitor::visit(ArrayDecl &node) {
  lastValue = nullptr;
}

void CodeGenVisitor::visit(ListDecl &node) {
  lastValue = nullptr;
}

void CodeGenVisitor::visit(CaseSTMT &node) {

}

void CodeGenVisitor::visit(SwitchSTMT &node) {

}

void CodeGenVisitor::visit(WhileSTMT &node) {
  
}

// ======== GENERICS =========
void CodeGenVisitor::createOpaqueStruct() {
  llvm::StructType::create(*context, "obw.opaque");
}
