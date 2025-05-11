#include "frontend/semantic/SemanticAnalyzer.h"
#include "frontend/parser/Expression.h"
#include <iostream>
#include <unordered_set>

std::string getOperatorString(OperatorKind op) {
  switch (op) {
  case OP_PLUS:
    return "+";
  case OP_MINUS:
    return "-";
  case OP_MULTIPLY:
    return "*";
  case OP_DIVIDE:
    return "/";
  default:
    return "unknown_operator";
  }
}

std::string getUnaryOperatorString(OperatorKind op) {
  switch (op) {
  case OP_UNARY_MINUS:
    return "-";
  case OP_LOGIC_NOT:
    return "!";
  case OP_BIT_NOT:
    return "~";
  default:
    return "unknown_unary_operator";
  }
}

bool SemanticAnalyzer::analyze(std::shared_ptr<Entity> root) {
  checkEntity(root);
  return errors.empty();
}

void SemanticAnalyzer::checkEntity(const std::shared_ptr<Entity> &entity) {
  // std::cout << "entity->getKind(): " << entity->getKind() << std::endl;
  switch (entity->getKind()) {
  case E_Module_Decl:
    checkModuleDecl(std::dynamic_pointer_cast<ModuleDecl>(entity));
    break;
  case E_Method_Decl:
    checkMethodDecl(std::dynamic_pointer_cast<MethodDecl>(entity));
    break;
  case E_Class_Decl:
    checkClassDecl(std::dynamic_pointer_cast<ClassDecl>(entity));
    break;
  case E_Constructor_Decl:
    checkConstrDecl(std::dynamic_pointer_cast<ConstrDecl>(entity));
    break;
  case E_Block:
    checkBlock(std::dynamic_pointer_cast<Block>(entity));
    break;
  case E_Method_Call:
    checkMethodCallEXP(std::dynamic_pointer_cast<MethodCallEXP>(entity));
    break;
  case E_Assignment:
    checkAssignmentSTMT(std::dynamic_pointer_cast<AssignmentSTMT>(entity));
    break;
  case E_Variable_Decl:
    checkVarDecl(std::dynamic_pointer_cast<VarDecl>(entity));
    break;
  case E_Parameter_Decl:
    checkParameterDecl(std::dynamic_pointer_cast<ParameterDecl>(entity));
    break;
  case E_Field_Decl:
    checkFieldDecl(std::dynamic_pointer_cast<FieldDecl>(entity));
    break;
  case E_Function_Decl:
    checkFuncDecl(std::dynamic_pointer_cast<FuncDecl>(entity));
    break;
  case E_Array_Decl:
    checkArrayDecl(std::dynamic_pointer_cast<ArrayDecl>(entity));
    break;
  case E_Enum_Decl:
    checkEnumDecl(std::dynamic_pointer_cast<EnumDecl>(entity));
    break;
  case E_Integer_Literal:
    checkIntLiteralEXP(std::dynamic_pointer_cast<IntLiteralEXP>(entity));
    break;
  case E_Real_Literal:
    checkRealLiteralEXP(std::dynamic_pointer_cast<RealLiteralEXP>(entity));
    break;
  case E_String_Literal:
    checkStringLiteralEXP(std::dynamic_pointer_cast<StringLiteralEXP>(entity));
    break;
  case E_Boolean_Literal:
    checkBoolLiteralEXP(std::dynamic_pointer_cast<BoolLiteralEXP>(entity));
    break;
  case E_Class_Name:
    checkClassNameEXP(std::dynamic_pointer_cast<ClassNameEXP>(entity));
    break;
  case E_Var_Reference:
    checkVarRefEXP(std::dynamic_pointer_cast<VarRefEXP>(entity));
    break;
  case E_Field_Reference:
    checkFieldRefEXP(std::dynamic_pointer_cast<FieldRefEXP>(entity));
    break;
  case E_Function_Call:
    checkFuncCallEXP(std::dynamic_pointer_cast<FuncCallEXP>(entity));
    break;
  case E_Constructor_Call:
    checkConstructorCallEXP(
        std::dynamic_pointer_cast<ConstructorCallEXP>(entity));
    break;
  case E_Array_Literal:
    checkArrayLiteralExpr(std::dynamic_pointer_cast<ArrayLiteralExpr>(entity));
    break;
  case E_Binary_Operator:
    checkBinaryOpEXP(std::dynamic_pointer_cast<BinaryOpEXP>(entity));
    break;
  case E_Unary_Operator:
    checkUnaryOpEXP(std::dynamic_pointer_cast<UnaryOpEXP>(entity));
    break;
  case E_Enum_Reference:
    checkEnumRefEXP(std::dynamic_pointer_cast<EnumRefEXP>(entity));
    break;
  case E_For_Loop:
    checkForSTMT(std::dynamic_pointer_cast<ForSTMT>(entity));
    break;
  case E_While_Loop:
    checkWhileSTMT(std::dynamic_pointer_cast<WhileSTMT>(entity));
    break;
  case E_If_Statement:
    checkIfSTMT(std::dynamic_pointer_cast<IfSTMT>(entity));
    break;
  case E_Switch_Statement:
    checkSwitchSTMT(std::dynamic_pointer_cast<SwitchSTMT>(entity));
    break;
  case E_Case_Statement:
    checkCaseSTMT(std::dynamic_pointer_cast<CaseSTMT>(entity));
    break;
  case E_Return_Statement:
    checkReturnSTMT(std::dynamic_pointer_cast<ReturnSTMT>(entity));
    break;
  case E_This:
    checkThisEXP(std::dynamic_pointer_cast<ThisEXP>(entity));
    break;
  case E_Main_Decl:
    checkMainDecl(std::dynamic_pointer_cast<FuncDecl>(entity));
    break;
  default:
    reportError("Unhandled entity kind: " + std::to_string(entity->getKind()),
                entity->getLoc());
  }
}

void SemanticAnalyzer::checkBlock(const std::shared_ptr<Block> &block) {
  for (const auto &part : block->parts) {
    checkEntity(part);
  }
}

void SemanticAnalyzer::checkMethodCallEXP(
    const std::shared_ptr<MethodCallEXP> &methodCall) {
  checkEntity(methodCall->left);

  for (const auto &arg : methodCall->arguments) {
    checkEntity(arg);
  }
}

void SemanticAnalyzer::reportError(const std::string &message, const Loc &loc) {
  errors.push_back("[Semantic Error] " + message);
  //  " (Line: " + std::to_string(loc.line) +
  //  ", Col: " + std::to_string(loc.col) + ")");
  std::cerr << errors.back() << std::endl;
}

void SemanticAnalyzer::checkAssignmentSTMT(
    const std::shared_ptr<AssignmentSTMT> &assignment) {
  if (assignment->variable) {
    checkEntity(assignment->variable);
  }
  if (assignment->expression) {
    checkEntity(assignment->expression);
  }
}

void SemanticAnalyzer::checkVarDecl(const std::shared_ptr<VarDecl> &varDecl) {
  if (!varDecl->type) {
    reportError("Variable '" + varDecl->name + "' has no type",
                varDecl->getLoc());
  }

  std::shared_ptr<Type> declaredType;
  if (currentScope->getName() != "main") {
    declaredType = globalTypeTable->getType(
        symbolTable->getCurrentScope()->getName(), varDecl->type->name);
  } else {
    declaredType =
        globalTypeTable->getType(currentModuleName, varDecl->type->name);
  }

  bool isBuiltinTypes = false;
  for (const auto &[name, type] : globalTypeTable->builtinTypes.types) {
    if (name == varDecl->type->name)
      isBuiltinTypes = true;
  }
  if (!declaredType and !isBuiltinTypes) {
    reportError("Undefined type '" + varDecl->type->name + "'",
                varDecl->getLoc());
  }

  if (varDecl->initializer) {
    checkEntity(varDecl->initializer);
  }
}

void SemanticAnalyzer::checkModuleDecl(
    const std::shared_ptr<ModuleDecl> &moduleDecl) {
  currentScope = currentScope->nextScope();
  currentModuleName = moduleDecl->name;
  if (moduleDecl->children.empty()) {
    reportError("Module '" + moduleDecl->name + "' is empty",
                moduleDecl->getLoc());
  }

  for (const auto &child : moduleDecl->children) {
    checkEntity(child);
  }

  currentScope = currentScope->prevScope();
}

void SemanticAnalyzer::checkClassDecl(
    const std::shared_ptr<ClassDecl> &classDecl) {
  // std::string moduleName = symbolTable->getCurrentScope()->getName();

  // if (globalTypeTable->getType(currentScope->getName(), classDecl->name)) {
  //   reportError("Duplicate class name: " + classDecl->name,
  //   classDecl->getLoc());
  // }

  currentScope = currentScope->nextScope();

  if (!classDecl->fields.empty()) {
    for (const auto &field : classDecl->fields) {
      checkEntity(field);
    }
  }
  if (!classDecl->methods.empty()) {
    for (const auto &method : classDecl->methods) {
      checkEntity(method);
    }
  }
  if (!classDecl->constructors.empty()) {
    for (const auto &constr : classDecl->constructors) {
      checkEntity(constr);
    }
  }
  currentScope = currentScope->prevScope();
}

void SemanticAnalyzer::checkConstrDecl(
    const std::shared_ptr<ConstrDecl> &constrDecl) {
  auto [decl, alloc, isInited] = *currentScope->getSymbol(constrDecl->name);

  if (!isInited) {
    reportError("Constructor '" + constrDecl->name +
                    "' is not in a class scope",
                constrDecl->getLoc());
  }

  for (const auto &param : constrDecl->args) {
    checkEntity(param);
  }

  if (constrDecl->body) {
    checkEntity(constrDecl->body);
  }
}

void SemanticAnalyzer::checkParameterDecl(
    const std::shared_ptr<ParameterDecl> &paramDecl) {
  auto paramType = globalTypeTable->getType("", paramDecl->type->name);

  bool isBuiltinTypes = false;
  for (const auto &[name, type] : globalTypeTable->builtinTypes.types) {
    if (name == paramDecl->type->name)
      isBuiltinTypes = true;
  }
  if (!paramType and !isBuiltinTypes) {
    reportError("Undefined parameter type '" + paramDecl->type->name + "'",
                paramDecl->getLoc());
  }

  auto [decl, alloc, isInited] = *currentScope->getSymbol(paramDecl->name);
  if (isInited) {
    reportError("Duplicate parameter name: " + paramDecl->name,
                paramDecl->getLoc());
  }
}

void SemanticAnalyzer::checkFieldDecl(
    const std::shared_ptr<FieldDecl> &fieldDecl) {
  auto fieldType = globalTypeTable->getType("", fieldDecl->type->name);
  if (!fieldType) {
    reportError("Undefined field type '" + fieldDecl->type->name + "'",
                fieldDecl->getLoc());
  }

  auto [decl, alloc, isInited] = *currentScope->getSymbol(fieldDecl->name);
  if (isInited) {
    reportError("Duplicate field name: " + fieldDecl->name,
                fieldDecl->getLoc());
  }
}

void SemanticAnalyzer::checkFuncDecl(
    const std::shared_ptr<FuncDecl> &funcDecl) {
  currentScope = currentScope->nextScope();
  currentReturnType =
      funcDecl->isVoid ? nullptr : funcDecl->signature->return_type;
  currentFuncIsVoid = funcDecl->isVoid;
  if (!funcDecl->isVoid) {
    auto returnType =
        globalTypeTable->getType("", funcDecl->signature->return_type->name);
    if (!returnType) {
      reportError("Undefined return type '" +
                      funcDecl->signature->return_type->name + "'",
                  funcDecl->getLoc());
    }
  }

  auto [decl, alloc, isInited] = *currentScope->getSymbol(funcDecl->name);
  if (isInited) {
    reportError("Duplicate function name: " + funcDecl->name,
                funcDecl->getLoc());
  }

  for (const auto &param : funcDecl->args) {
    checkEntity(param);
  }

  if (funcDecl->body) {
    checkEntity(funcDecl->body);
  }

  currentReturnType = nullptr;
  currentFuncIsVoid = false;
  currentScope = currentScope->prevScope();
}

void SemanticAnalyzer::checkMethodDecl(
    const std::shared_ptr<MethodDecl> &methodDecl) {
  currentScope = currentScope->nextScope();
  currentReturnType =
      methodDecl->isVoid ? nullptr : methodDecl->signature->return_type;
  currentFuncIsVoid = methodDecl->isVoid;
  if (!methodDecl->isVoid) {
    auto returnType =
        globalTypeTable->getType("", methodDecl->signature->return_type->name);
    bool isBuiltinTypes = false;
    for (const auto &[name, type] : globalTypeTable->builtinTypes.types) {
      if (name == methodDecl->signature->return_type->name)
        isBuiltinTypes = true;
    }
    if (!returnType and !isBuiltinTypes) {
      reportError("Undefined return type '" +
                      methodDecl->signature->return_type->name + "'",
                  methodDecl->getLoc());
    }
  }

  auto [decl, alloc, isInited] = *currentScope->getSymbol(methodDecl->name);
  if (isInited) {
    reportError("Duplicate method name: " + methodDecl->name,
                methodDecl->getLoc());
  }

  for (const auto &param : methodDecl->args) {
    checkEntity(param);
  }

  if (methodDecl->body) {
    checkEntity(methodDecl->body);
  }
  currentReturnType = nullptr;
  currentFuncIsVoid = false;

  currentScope = currentScope->prevScope();
}

void SemanticAnalyzer::checkArrayDecl(
    const std::shared_ptr<ArrayDecl> &arrayDecl) {
  auto elementType = globalTypeTable->getType("", arrayDecl->type->name);
  if (!elementType) {
    reportError("Undefined array element type '" + arrayDecl->type->name + "'",
                arrayDecl->getLoc());
  }

  if (arrayDecl->size <= 0) {
    reportError("Invalid array size: " + std::to_string(arrayDecl->size),
                arrayDecl->getLoc());
  }

  auto [decl, alloc, isInited] = *currentScope->getSymbol(arrayDecl->name);
  if (isInited) {
    reportError("Duplicate array name: " + arrayDecl->name,
                arrayDecl->getLoc());
  }
}

void SemanticAnalyzer::checkEnumDecl(
    const std::shared_ptr<EnumDecl> &enumDecl) {
  auto [decl, alloc, isInited] = *currentScope->getSymbol(enumDecl->name);
  if (isInited) {
    reportError("Duplicate enum name: " + enumDecl->name, enumDecl->getLoc());
  }

  std::unordered_set<std::string> items;
  for (auto &item : enumDecl->items) {
    if (items.count(item.first)) {
      reportError("Duplicate enum item: " + item.first, enumDecl->getLoc());
    }
    items.insert(item.first);
  }
}

void SemanticAnalyzer::checkIntLiteralEXP(
    const std::shared_ptr<IntLiteralEXP> &lit) {

  if (lit->getValue() < std::numeric_limits<int32_t>::min() ||
      lit->getValue() > std::numeric_limits<int32_t>::max()) {
    reportError("Integer literal out of range", lit->getLoc());
  }
}

void SemanticAnalyzer::checkRealLiteralEXP(
    const std::shared_ptr<RealLiteralEXP> &lit) {
  auto type = resolveExprType(lit);
  if (!type || type->name != "Real") {
    reportError("Invalid real literal type", lit->getLoc());
  }
}

void SemanticAnalyzer::checkStringLiteralEXP(
    const std::shared_ptr<StringLiteralEXP> &lit) {
  auto type = resolveExprType(lit);
  if (!type || type->name != "String") {
    reportError("Invalid string literal type", lit->getLoc());
  }
}

void SemanticAnalyzer::checkBoolLiteralEXP(
    const std::shared_ptr<BoolLiteralEXP> &lit) {
  auto type = resolveExprType(lit);
  if (!type || type->name != "Bool") {
    reportError("Invalid boolean literal type", lit->getLoc());
  }
}

void SemanticAnalyzer::checkClassNameEXP(
    const std::shared_ptr<ClassNameEXP> &classRef) {
  auto classType = globalTypeTable->getType(
      symbolTable->getCurrentScope()->getName(), classRef->name);
  if (!classType) {
    reportError("Undefined class name: " + classRef->name, classRef->getLoc());
    return;
  }

  if (!std::dynamic_pointer_cast<TypeClass>(classType)) {
    reportError("'" + classRef->name + "' is not a class", classRef->getLoc());
  }
}

void SemanticAnalyzer::checkVarRefEXP(
    const std::shared_ptr<VarRefEXP> &varRef) {
  auto [decl, alloc, isInited] = *currentScope->getSymbol(varRef->var_name);
  if (!decl) {
    reportError("Undefined variable '" + varRef->var_name + "'",
                varRef->getLoc());
  }
}

void SemanticAnalyzer::checkFieldRefEXP(
    const std::shared_ptr<FieldRefEXP> &fieldRef) {
  auto objType = resolveExprType(fieldRef->obj);

  auto classType = std::dynamic_pointer_cast<TypeClass>(objType);
  if (!classType) {
    reportError("Field access on non-class type", fieldRef->getLoc());
    return;
  }

  if (!classType->getField(fieldRef->field_name)) {
    reportError("Undefined field '" + fieldRef->field_name + "' in class",
                fieldRef->getLoc());
  }
}

void SemanticAnalyzer::checkFuncCallEXP(
    const std::shared_ptr<FuncCallEXP> &funcCall) {
  if (!funcCall->arguments.empty()) {
    for (const auto &arg : funcCall->arguments) {
      checkEntity(arg);
    }
  }
}

void SemanticAnalyzer::checkConstructorCallEXP(
    const std::shared_ptr<ConstructorCallEXP> &constrCall) {
  auto classType = globalTypeTable->getType("", constrCall->left->name);

  bool isBuiltinTypes = false;
  for (const auto &[name, type] : globalTypeTable->builtinTypes.types) {
    if (name == constrCall->left->name)
      isBuiltinTypes = true;
  }

  if (!classType and !isBuiltinTypes) {
    reportError("Undefined class '" + constrCall->left->name + "'",
                constrCall->getLoc());
    return;
  }
}

void SemanticAnalyzer::checkArrayLiteralExpr(
    const std::shared_ptr<ArrayLiteralExpr> &arrayLit) {
  if (arrayLit->elements.empty()) {
    reportError("Array literal cannot be empty", arrayLit->getLoc());
    return;
  }

  auto firstType = resolveExprType(arrayLit->elements[0]);
  for (const auto &elem : arrayLit->elements) {
    auto elemType = resolveExprType(elem);
    if (elemType != firstType) {
      reportError("All array elements must be of the same type",
                  elem->getLoc());
    }
  }
}

void SemanticAnalyzer::checkBinaryOpEXP(
    const std::shared_ptr<BinaryOpEXP> &binOp) {
  auto leftType = resolveExprType(binOp->left);
  auto rightType = resolveExprType(binOp->right);

  if (leftType->name != rightType->name) {
    reportError("Type mismatch for operator " + getOperatorString(binOp->op),
                binOp->getLoc());
  }

  if (binOp->op == OP_PLUS || binOp->op == OP_MINUS ||
      binOp->op == OP_MULTIPLY || binOp->op == OP_DIVIDE) {
    if (leftType->name != "Integer" && leftType->name != "Real") {
      reportError("Operator " + getOperatorString(binOp->op) +
                      " cannot be applied to non-numeric type",
                  binOp->getLoc());
    }
  }

  checkEntity(binOp->left);
  checkEntity(binOp->right);
}

void SemanticAnalyzer::checkUnaryOpEXP(
    const std::shared_ptr<UnaryOpEXP> &unaryOp) {
  auto operandType = resolveExprType(unaryOp->operand);

  // if (operandType == ) {
  //   reportError("Unary operator '" + getUnaryOperatorString(unaryOp->op) +
  //              "' cannot be applied to type '" + operandType->name + "'",
  //              unaryOp->getLoc());
  // }
}

void SemanticAnalyzer::checkEnumRefEXP(
    const std::shared_ptr<EnumRefEXP> &enumRef) {
  auto [decl, alloc, isInited] = *currentScope->getSymbol(enumRef->enumName);
  if (!decl || decl->getKind() != E_Enum_Decl) {
    reportError("Undefined enumeration '" + enumRef->enumName + "'",
                enumRef->getLoc());
    return;
  }

  auto enumType = std::dynamic_pointer_cast<EnumDecl>(decl);
  if (!enumType->items.count(enumRef->itemName)) {
    reportError("Undefined item '" + enumRef->itemName + "' in enumeration '" +
                    enumRef->enumName + "'",
                enumRef->getLoc());
  }
}

void SemanticAnalyzer::checkForSTMT(const std::shared_ptr<ForSTMT> &forStmt) {
  if (forStmt->varWithAss)
    checkEntity(forStmt->varWithAss);

  if (forStmt->condition) {
    auto condType = resolveExprType(forStmt->condition);
    if (condType->name != "Bool") {
      reportError("For loop condition must be boolean",
                  forStmt->condition->getLoc());
    }
  }

  if (forStmt->post)
    checkEntity(forStmt->post);

  checkEntity(forStmt->body);
}

void SemanticAnalyzer::checkWhileSTMT(
    const std::shared_ptr<WhileSTMT> &whileStmt) {
  auto condType = resolveExprType(whileStmt->condition);
  if (condType->name != "Bool") {
    reportError("While loop condition must be boolean",
                whileStmt->condition->getLoc());
  }

  checkEntity(whileStmt->body);
}

void SemanticAnalyzer::checkIfSTMT(const std::shared_ptr<IfSTMT> &ifStmt) {
  auto condType = resolveExprType(ifStmt->condition);
  if (condType->name != "Bool") {
    reportError("If condition must be boolean", ifStmt->condition->getLoc());
  }

  checkEntity(ifStmt->ifTrue);
  if (ifStmt->ifFalse)
    checkEntity(ifStmt->ifFalse);
}

void SemanticAnalyzer::checkSwitchSTMT(
    const std::shared_ptr<SwitchSTMT> &switchStmt) {
  auto condType = resolveExprType(switchStmt->condition);

  for (const auto &caseStmt : switchStmt->cases) {
    checkEntity(caseStmt);
    if (caseStmt->condition_literal) {
      auto caseType = resolveExprType(caseStmt->condition_literal);
      if (caseType != condType) {
        reportError("Case type mismatch with switch condition",
                    caseStmt->condition_literal->getLoc());
      }
    }
  }
}

void SemanticAnalyzer::checkCaseSTMT(
    const std::shared_ptr<CaseSTMT> &caseStmt) {
  if (caseStmt->condition_literal)
    checkEntity(caseStmt->condition_literal);
  checkEntity(caseStmt->body);
}

void SemanticAnalyzer::checkReturnSTMT(
    const std::shared_ptr<ReturnSTMT> &returnStmt) {
  if (!currentReturnType && !currentFuncIsVoid) {
    reportError("Return outside function context", returnStmt->getLoc());
    return;
  }

  if (currentFuncIsVoid) {
    if (returnStmt->expr) {
      reportError("Void function cannot return a value", returnStmt->getLoc());
    }
  } else {
    if (!returnStmt->expr) {
      reportError("Non-void function must return a value",
                  returnStmt->getLoc());
      return;
    }

    auto returnType = resolveExprType(returnStmt->expr);
    if (!returnType || returnType->name != currentReturnType->name) {
      reportError("Return type mismatch: expected '" + currentReturnType->name +
                      "'",
                  returnStmt->getLoc());
    }
  }
}

void SemanticAnalyzer::checkThisEXP(const std::shared_ptr<ThisEXP> &thisExp) {
  // auto classScope = symbolTable->getEnclosingClassScope(currentScope);
  // if (!classScope) {
  //   reportError("'this' used outside class context", thisExp->getLoc());
  // }
}

void SemanticAnalyzer::checkMainDecl(
    const std::shared_ptr<FuncDecl> &funcDecl) {
  if (symbolTable->getCurrentScope()->lookup("main")) {
    reportError("Duplicate main declaration", funcDecl->getLoc());
  }

  if (!funcDecl->isVoid) {
    if (funcDecl->signature->return_type->name != "Integer") {
      reportError("main must return Integer or be void", funcDecl->getLoc());
    }
  }

  currentScope = currentScope->nextScope();
  for (const auto &param : funcDecl->args) {
    checkEntity(param);
  }
  if (funcDecl->body) {
    checkEntity(funcDecl->body);
  }
  currentScope = currentScope->prevScope();
}

std::shared_ptr<Type> SemanticAnalyzer::getTypeInTypeTable(std::string str) {
  for (const auto &[name, type] : globalTypeTable->builtinTypes.types) {
    if (name == str) {
      return type;
    }
  }
  return nullptr;
}

std::shared_ptr<Type>
SemanticAnalyzer::resolveExprType(const std::shared_ptr<Expression> &expr) {
  if (auto intLit = std::dynamic_pointer_cast<IntLiteralEXP>(expr)) {
    return getTypeInTypeTable("Integer");
  }
  if (auto realLit = std::dynamic_pointer_cast<RealLiteralEXP>(expr)) {
    return getTypeInTypeTable("Real");
  }
  if (auto strLit = std::dynamic_pointer_cast<StringLiteralEXP>(expr)) {
    return getTypeInTypeTable("String");
  }
  if (auto boolLit = std::dynamic_pointer_cast<BoolLiteralEXP>(expr)) {
    return getTypeInTypeTable("Bool");
  }

  if (auto varRef = std::dynamic_pointer_cast<VarRefEXP>(expr)) {
    if (auto decl = currentScope->lookup(varRef->var_name)) {
      if (auto varDecl = std::dynamic_pointer_cast<VarDecl>(decl))
        return varDecl->type;
      if (auto paramDecl = std::dynamic_pointer_cast<ParameterDecl>(decl))
        return paramDecl->type;
      if (auto fieldDecl = std::dynamic_pointer_cast<FieldDecl>(decl))
        return fieldDecl->type;
      reportError("Unexpected declaration type for variable", expr->getLoc());
    }
    return nullptr;
  }

  if (auto fieldRef = std::dynamic_pointer_cast<FieldRefEXP>(expr)) {
    auto objType = resolveExprType(fieldRef->obj);
    if (auto classType = std::dynamic_pointer_cast<TypeClass>(objType)) {
      if (auto fieldType = classType->getField(fieldRef->field_name)) {
        return fieldType;
      }
      reportError("Undefined field: " + fieldRef->field_name, expr->getLoc());
    } else {
      reportError("Field access on non-class type", expr->getLoc());
    }
    return nullptr;
  }

  if (auto methodCall = std::dynamic_pointer_cast<MethodCallEXP>(expr)) {
    auto classType =
        std::dynamic_pointer_cast<TypeClass>(resolveExprType(methodCall->left));
    if (!classType) {
      reportError("Method call on non-class type", expr->getLoc());
      return nullptr;
    }
    if (auto methodType = classType->getMethod(methodCall->method_name)) {
      return methodType;
    }
    reportError("Undefined method: " + methodCall->method_name, expr->getLoc());
    return nullptr;
  }

  if (auto constrCall = std::dynamic_pointer_cast<ConstructorCallEXP>(expr)) {
    return globalTypeTable->getType("", constrCall->left->name);
  }

  if (auto binOp = std::dynamic_pointer_cast<BinaryOpEXP>(expr)) {
    auto leftType = resolveExprType(binOp->left);
    auto rightType = resolveExprType(binOp->right);

    if (binOp->op == OP_EQUAL || binOp->op == OP_NOT_EQUAL ||
        binOp->op == OP_LESS || binOp->op == OP_LESS_EQUAL ||
        binOp->op == OP_MORE || binOp->op == OP_MORE_EQUAL) {
      return getTypeInTypeTable("Bool");
    }

    if (binOp->op == OP_LOGIC_AND || binOp->op == OP_LOGIC_OR) {
      if (leftType->name != "Bool" || rightType->name != "Bool") {
        reportError("Logical operators require boolean operands",
                    binOp->getLoc());
      }
    }

    return getTypeInTypeTable("Bool");
  }

  if (auto unaryOp = std::dynamic_pointer_cast<UnaryOpEXP>(expr)) {
    auto operandType = resolveExprType(unaryOp->operand);

    if (unaryOp->op == OP_LOGIC_NOT) {
      if (operandType->name != "Bool") {
        reportError("'!' operator requires boolean operand", unaryOp->getLoc());
      }
      return getTypeInTypeTable("Bool");
    }

    return operandType;
  }

  if (auto className = std::dynamic_pointer_cast<ClassNameEXP>(expr)) {
    return globalTypeTable->getType("", className->name);
  }

  if (auto enumRef = std::dynamic_pointer_cast<EnumRefEXP>(expr)) {
    return globalTypeTable->getType("", enumRef->enumName);
  }

  if (auto arrayLit = std::dynamic_pointer_cast<ArrayLiteralExpr>(expr)) {
    if (arrayLit->elements.empty())
      return nullptr;

    auto firstType = resolveExprType(arrayLit->elements[0]);
    for (const auto &elem : arrayLit->elements) {
      if (resolveExprType(elem) != firstType) {
        reportError("Inconsistent array element types", elem->getLoc());
        return nullptr;
      }
    }
    return std::make_shared<TypeArray>(arrayLit->elements.size(), firstType);
  }

  reportError("Unresolved expression type", expr->getLoc());
  return nullptr;
}