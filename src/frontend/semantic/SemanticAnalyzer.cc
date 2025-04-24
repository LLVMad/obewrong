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
  currentScope = symbolTable->getGlobalScope();
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
  resolveType(methodCall->left);
  auto objType = methodCall->left->resolveType(globalTypeTable->builtinTypes);
  if (!objType) {
    reportError("Undefined object in method call", methodCall->getLoc());
    return;
  }

  auto type = globalTypeTable->getType("", objType->name);
  if (!type) {
    reportError("Type '" + objType->name + "' not found", methodCall->getLoc());
    return;
  }

  auto classType = std::dynamic_pointer_cast<TypeClass>(type);
  if (!classType) {
    reportError("Type '" + objType->name + "' is not a class",
                methodCall->getLoc());
    return;
  }

  auto method = classType->getMethod(methodCall->method_name);
  if (!method) {
    reportError("Method '" + methodCall->method_name + "' not found in type '" +
                    objType->name + "'",
                methodCall->getLoc());
    return;
  }

  if (method->args.size() != methodCall->arguments.size()) {
    reportError("Argument count mismatch for method '" +
                    methodCall->method_name + "'",
                methodCall->getLoc());
    return;
  }

  for (size_t i = 0; i < methodCall->arguments.size(); ++i) {
    resolveType(methodCall->arguments[i]);
    auto argType =
        methodCall->arguments[i]->resolveType(globalTypeTable->builtinTypes);
    if (argType != method->args[i]) {
      reportError("Type mismatch in argument " + std::to_string(i + 1),
                  methodCall->arguments[i]->getLoc());
    }
  }
}

void SemanticAnalyzer::resolveType(const std::shared_ptr<Expression> &expr) {
  auto type = expr->resolveType(globalTypeTable->builtinTypes);

  if (!type) {
    reportError("Cannot resolve type of expression", expr->getLoc());
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
    resolveType(assignment->variable);
    auto varType =
        assignment->variable->resolveType(globalTypeTable->builtinTypes);
    resolveType(assignment->expression);
    auto exprType =
        assignment->expression->resolveType(globalTypeTable->builtinTypes);
    if (varType != exprType) {
      reportError("Type mismatch in assignment", assignment->getLoc());
    }
  } else if (assignment->field) {
    resolveType(assignment->field->obj);
    auto objType =
        assignment->field->obj->resolveType(globalTypeTable->builtinTypes);
    auto classType = std::dynamic_pointer_cast<TypeClass>(objType);
    if (classType) {
      auto fieldDecl = classType->getField(assignment->field->field_name);
      if (!fieldDecl) {
        reportError("Field '" + assignment->field->field_name + "' not found",
                    assignment->getLoc());
        return;
      }
      resolveType(assignment->expression);
      auto exprType =
          assignment->expression->resolveType(globalTypeTable->builtinTypes);
      if (exprType != fieldDecl) {
        reportError("Type mismatch in field assignment", assignment->getLoc());
      }
    }
  }
}

void SemanticAnalyzer::checkVarDecl(const std::shared_ptr<VarDecl> &varDecl) {
  if (!varDecl->type) {
    reportError("Variable '" + varDecl->name + "' has no type",
                varDecl->getLoc());
    return;
  }

  bool isBuiltinTypes = false;
  for (const auto &[name, type] : globalTypeTable->builtinTypes.types) {
    if (name == varDecl->type->name)
      isBuiltinTypes = true;
  }

  auto declaredType = globalTypeTable->getType("", varDecl->type->name);
  // if (!declaredType and !isBuiltinTypes) {
  //     reportError("Undefined type '" + varDecl->type->name + "'",
  //     varDecl->getLoc()); return;
  // }

  if (varDecl->initializer) {
    resolveType(varDecl->initializer);
    auto initType =
        varDecl->initializer->resolveType(globalTypeTable->builtinTypes);
    if (initType != declaredType) {
      reportError("Type mismatch in initialization of '" + varDecl->name + "'",
                  varDecl->getLoc());
    }
  }
}

void SemanticAnalyzer::checkModuleDecl(
    const std::shared_ptr<ModuleDecl> &moduleDecl) {
  if (moduleDecl->children.empty()) {
    reportError("Module '" + moduleDecl->name + "' is empty",
                moduleDecl->getLoc());
    return;
  }

  for (const auto &child : moduleDecl->children) {
    checkEntity(child);
  }
}

void SemanticAnalyzer::checkClassDecl(
    const std::shared_ptr<ClassDecl> &classDecl) {
  std::string moduleName = symbolTable->getCurrentScope()->getName();
  // TODO

  auto existingType = globalTypeTable->getType(moduleName, classDecl->name);
  std::cout << "Checking class " << classDecl->name << " in module "
            << moduleName << std::endl;

  if (existingType) {
    reportError("Duplicate class name: " + classDecl->name,
                classDecl->getLoc());
    return;
  }

  if (!classDecl->fields.empty()) {
    for (const auto &field : classDecl->fields) {
      std::cout << "enter into classDecl->fields" << std::endl;
      checkEntity(field);
    }
  }
  if (!classDecl->methods.empty()) {
    for (const auto &method : classDecl->methods) {
      std::cout << "enter into classDecl->methods" << std::endl;
      checkEntity(method);
    }
  }
  if (!classDecl->constructors.empty()) {
    for (const auto &constr : classDecl->constructors) {
      checkEntity(constr);
    }
  }
}

void SemanticAnalyzer::checkConstrDecl(
    const std::shared_ptr<ConstrDecl> &constrDecl) {
  auto classType = std::dynamic_pointer_cast<TypeClass>(
      currentScope->lookup(constrDecl->name));
  if (!classType) {
    reportError("Constructor '" + constrDecl->name +
                    "' is not in a class scope",
                constrDecl->getLoc());
    return;
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

  if (currentScope->lookup(paramDecl->name)) {
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

  if (currentScope->lookup(fieldDecl->name)) {
    reportError("Duplicate field name: " + fieldDecl->name,
                fieldDecl->getLoc());
  }
}

void SemanticAnalyzer::checkFuncDecl(
    const std::shared_ptr<FuncDecl> &funcDecl) {
  if (!funcDecl->isVoid) {
    auto returnType =
        globalTypeTable->getType("", funcDecl->signature->return_type->name);
    if (!returnType) {
      reportError("Undefined return type '" +
                      funcDecl->signature->return_type->name + "'",
                  funcDecl->getLoc());
    }
  }

  if (currentScope->lookup(funcDecl->name)) {
    reportError("Duplicate function name: " + funcDecl->name,
                funcDecl->getLoc());
  }

  for (const auto &param : funcDecl->args) {
    checkEntity(param);
  }

  if (funcDecl->body) {
    checkEntity(funcDecl->body);
  }
}

void SemanticAnalyzer::checkMethodDecl(
    const std::shared_ptr<MethodDecl> &methodDecl) {
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

  // auto classScope = symbolTable->getEnclosingClassScope(currentScope);
  if (currentScope->lookup(methodDecl->name)) {
    reportError("Duplicate method name: " + methodDecl->name,
                methodDecl->getLoc());
  }

  for (const auto &param : methodDecl->args) {
    checkEntity(param);
  }

  if (methodDecl->body) {
    checkEntity(methodDecl->body);
  }
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

  if (currentScope->lookup(arrayDecl->name)) {
    reportError("Duplicate array name: " + arrayDecl->name,
                arrayDecl->getLoc());
  }
}

void SemanticAnalyzer::checkEnumDecl(
    const std::shared_ptr<EnumDecl> &enumDecl) {
  if (currentScope->lookup(enumDecl->name)) {
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
  auto type = lit->resolveType(globalTypeTable->builtinTypes);
  if (!type || type->name != "Integer") {
    reportError("Invalid integer literal type", lit->getLoc());
  }

  if (lit->getValue() < std::numeric_limits<int32_t>::min() ||
      lit->getValue() > std::numeric_limits<int32_t>::max()) {
    reportError("Integer literal out of range", lit->getLoc());
  }
}

void SemanticAnalyzer::checkRealLiteralEXP(
    const std::shared_ptr<RealLiteralEXP> &lit) {
  auto type = lit->resolveType(globalTypeTable->builtinTypes);
  if (!type || type->name != "Real") {
    reportError("Invalid real literal type", lit->getLoc());
  }
}

void SemanticAnalyzer::checkStringLiteralEXP(
    const std::shared_ptr<StringLiteralEXP> &lit) {
  auto type = lit->resolveType(globalTypeTable->builtinTypes);
  if (!type || type->name != "String") {
    reportError("Invalid string literal type", lit->getLoc());
  }
}

void SemanticAnalyzer::checkBoolLiteralEXP(
    const std::shared_ptr<BoolLiteralEXP> &lit) {
  auto type = lit->resolveType(globalTypeTable->builtinTypes);
  if (!type || type->name != "Bool") {
    reportError("Invalid boolean literal type", lit->getLoc());
  }
}

void SemanticAnalyzer::checkClassNameEXP(
    const std::shared_ptr<ClassNameEXP> &classRef) {
  auto classType =
      globalTypeTable->getType(currentScope->getName(), classRef->name);
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
  auto varDecl = currentScope->lookup(varRef->var_name);
  if (!varDecl) {
    reportError("Undefined variable '" + varRef->var_name + "'",
                varRef->getLoc());
    return;
  }

  auto varType = varDecl->resolveType(globalTypeTable->builtinTypes);
  if (!varType) {
    reportError("Cannot resolve type for variable '" + varRef->var_name + "'",
                varRef->getLoc());
  }
}

void SemanticAnalyzer::checkFieldRefEXP(
    const std::shared_ptr<FieldRefEXP> &fieldRef) {
  resolveType(fieldRef->obj);
  auto objType = fieldRef->obj->resolveType(globalTypeTable->builtinTypes);

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
  auto funcDecl = currentScope->lookup(funcCall->func_name);
  if (!funcDecl) {
    reportError("Undefined function '" + funcCall->func_name + "'",
                funcCall->getLoc());
    return;
  }

  auto funcType = funcDecl->resolveType(globalTypeTable->builtinTypes);
  auto funcSignature = std::dynamic_pointer_cast<TypeFunc>(funcType);

  if (funcCall->arguments.size() != funcSignature->args.size()) {
    reportError("Argument count mismatch for function '" + funcCall->func_name +
                    "'",
                funcCall->getLoc());
  }

  for (size_t i = 0; i < funcCall->arguments.size(); ++i) {
    resolveType(funcCall->arguments[i]);
    auto argType =
        funcCall->arguments[i]->resolveType(globalTypeTable->builtinTypes);
    if (argType != funcSignature->args[i]) {
      reportError("Type mismatch in argument " + std::to_string(i + 1),
                  funcCall->arguments[i]->getLoc());
    }
  }
}

void SemanticAnalyzer::checkConstructorCallEXP(
    const std::shared_ptr<ConstructorCallEXP> &constrCall) {
  // TODO
  auto classType = globalTypeTable->getType(
      symbolTable->getCurrentScope()->getName(), constrCall->left->name);

  if (!classType) {
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

  auto firstType =
      arrayLit->elements[0]->resolveType(globalTypeTable->builtinTypes);
  for (const auto &elem : arrayLit->elements) {
    auto elemType = elem->resolveType(globalTypeTable->builtinTypes);
    if (elemType != firstType) {
      reportError("All array elements must be of the same type",
                  elem->getLoc());
    }
  }
}

void SemanticAnalyzer::checkBinaryOpEXP(
    const std::shared_ptr<BinaryOpEXP> &binOp) {
  resolveType(binOp->left);
  resolveType(binOp->right);

  auto leftType = binOp->left->resolveType(globalTypeTable->builtinTypes);
  auto rightType = binOp->right->resolveType(globalTypeTable->builtinTypes);

  if (leftType != rightType) {
    reportError("Operator '" + getOperatorString(binOp->op) +
                    "' cannot be applied to types '" + leftType->name +
                    "' and '" + rightType->name + "'",
                binOp->getLoc());
  }
}

void SemanticAnalyzer::checkUnaryOpEXP(
    const std::shared_ptr<UnaryOpEXP> &unaryOp) {
  resolveType(unaryOp->operand);
  auto operandType =
      unaryOp->operand->resolveType(globalTypeTable->builtinTypes);

  // if (operandType == ) {
  //   reportError("Unary operator '" + getUnaryOperatorString(unaryOp->op) +
  //              "' cannot be applied to type '" + operandType->name + "'",
  //              unaryOp->getLoc());
  // }
}

void SemanticAnalyzer::checkEnumRefEXP(
    const std::shared_ptr<EnumRefEXP> &enumRef) {
  auto enumDecl = currentScope->lookup(enumRef->enumName);
  if (!enumDecl || enumDecl->getKind() != E_Enum_Decl) {
    reportError("Undefined enumeration '" + enumRef->enumName + "'",
                enumRef->getLoc());
    return;
  }

  auto enumType = std::dynamic_pointer_cast<EnumDecl>(enumDecl);
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
    resolveType(forStmt->condition);
    auto condType =
        forStmt->condition->resolveType(globalTypeTable->builtinTypes);
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
  resolveType(whileStmt->condition);
  auto condType =
      whileStmt->condition->resolveType(globalTypeTable->builtinTypes);
  if (condType->name != "Bool") {
    reportError("While loop condition must be boolean",
                whileStmt->condition->getLoc());
  }

  checkEntity(whileStmt->body);
}

void SemanticAnalyzer::checkIfSTMT(const std::shared_ptr<IfSTMT> &ifStmt) {
  resolveType(ifStmt->condition);
  auto condType = ifStmt->condition->resolveType(globalTypeTable->builtinTypes);
  if (condType->name != "Bool") {
    reportError("If condition must be boolean", ifStmt->condition->getLoc());
  }

  checkEntity(ifStmt->ifTrue);
  if (ifStmt->ifFalse)
    checkEntity(ifStmt->ifFalse);
}

void SemanticAnalyzer::checkSwitchSTMT(
    const std::shared_ptr<SwitchSTMT> &switchStmt) {
  resolveType(switchStmt->condition);
  auto condType =
      switchStmt->condition->resolveType(globalTypeTable->builtinTypes);

  for (const auto &caseStmt : switchStmt->cases) {
    checkEntity(caseStmt);
    if (caseStmt->condition_literal) {
      resolveType(caseStmt->condition_literal);
      auto caseType = caseStmt->condition_literal->resolveType(
          globalTypeTable->builtinTypes);
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
  // auto currentFunc = symbolTable->getEnclosingFunction(currentScope);
  // if (!currentFunc) {
  //   reportError("Return outside function", returnStmt->getLoc());
  //   return;
  // }

  // if (currentFunc->isVoid && returnStmt->expr) {
  //   reportError("Void function cannot return value", returnStmt->getLoc());
  // } else if (!currentFunc->isVoid) {
  //   resolveType(returnStmt->expr);
  //   auto returnType =
  //   returnStmt->expr->resolveType(globalTypeTable->builtinTypes); if
  //   (returnType != currentFunc->signature->returnType) {
  //     reportError("Return type mismatch", returnStmt->getLoc());
  //   }
  // }
}

void SemanticAnalyzer::checkThisEXP(const std::shared_ptr<ThisEXP> &thisExp) {
  // auto classScope = symbolTable->getEnclosingClassScope(currentScope);
  // if (!classScope) {
  //   reportError("'this' used outside class context", thisExp->getLoc());
  // }
}