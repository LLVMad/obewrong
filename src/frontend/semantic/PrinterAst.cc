#include "frontend/semantic/PrinterAst.h"
#include "frontend/parser/Expression.h"
#include <iostream>

void PrinterAst::printAST(const std::shared_ptr<Entity> &root) {
  printEntity(root, 0);
}

void PrinterAst::printEntity(const std::shared_ptr<Entity> &entity,
                             int indent) {
  switch (entity->getKind()) {
  // Declarations
  case E_Class_Decl:
    printClassDecl(std::dynamic_pointer_cast<ClassDecl>(entity), indent);
    break;
  case E_Constructor_Decl:
    printConstrDecl(std::dynamic_pointer_cast<ConstrDecl>(entity), indent);
    break;
  case E_Variable_Decl:
    printVarDecl(std::dynamic_pointer_cast<VarDecl>(entity), indent);
    break;
  case E_Parameter_Decl:
    printParamDecl(std::dynamic_pointer_cast<ParameterDecl>(entity), indent);
    break;
  case E_Field_Decl:
    printFieldDecl(std::dynamic_pointer_cast<FieldDecl>(entity), indent);
    break;
  case E_Function_Decl:
    printFuncDecl(std::dynamic_pointer_cast<FuncDecl>(entity), indent);
    break;
  case E_Method_Decl:
    printMethodDecl(std::dynamic_pointer_cast<MethodDecl>(entity), indent);
    break;
  case E_Array_Decl:
    printArrayDecl(std::dynamic_pointer_cast<ArrayDecl>(entity), indent);
    break;
  case E_Module_Decl:
    printModuleDecl(std::dynamic_pointer_cast<ModuleDecl>(entity), indent);
    break;
  case E_Enum_Decl:
    printEnumDecl(std::dynamic_pointer_cast<EnumDecl>(entity), indent);
    break;

  // Type-related entities
  case E_Integer_Literal:
    printIntLiteral(std::dynamic_pointer_cast<IntLiteralEXP>(entity), indent);
    break;
  case E_Real_Literal:
    printRealLiteral(std::dynamic_pointer_cast<RealLiteralEXP>(entity), indent);
    break;
  case E_String_Literal:
    printStringLiteral(std::dynamic_pointer_cast<StringLiteralEXP>(entity),
                       indent);
    break;
  case E_Boolean_Literal:
    printBoolLiteral(std::dynamic_pointer_cast<BoolLiteralEXP>(entity), indent);
    break;
  case E_Class_Name:
    printClassName(std::dynamic_pointer_cast<ClassNameEXP>(entity), indent);
    break;

  // Expressions
  case E_Var_Reference:
    printVarRef(std::dynamic_pointer_cast<VarRefEXP>(entity), indent);
    break;
  case E_Field_Reference:
    printFieldRef(std::dynamic_pointer_cast<FieldRefEXP>(entity), indent);
    break;
  case E_Function_Call:
    printFuncCall(std::dynamic_pointer_cast<FuncCallEXP>(entity), indent);
    break;
  case E_Method_Call:
    printMethodCall(std::dynamic_pointer_cast<MethodCallEXP>(entity), indent);
    break;
  case E_Constructor_Call:
    printConstrCall(std::dynamic_pointer_cast<ConstructorCallEXP>(entity),
                    indent);
    break;
  case E_Array_Literal:
    printArrayLiteral(std::dynamic_pointer_cast<ArrayLiteralExpr>(entity),
                      indent);
    break;
  case E_Binary_Operator:
    printBinaryOp(std::dynamic_pointer_cast<BinaryOpEXP>(entity), indent);
    break;
  case E_Unary_Operator:
    printUnaryOp(std::dynamic_pointer_cast<UnaryOpEXP>(entity), indent);
    break;
  case E_Enum_Reference:
    printEnumRef(std::dynamic_pointer_cast<EnumRefEXP>(entity), indent);
    break;

  // Statements
  case E_Assignment:
    printAssignment(std::dynamic_pointer_cast<AssignmentSTMT>(entity), indent);
    break;
  case E_For_Loop:
    printForLoop(std::dynamic_pointer_cast<ForSTMT>(entity), indent);
    break;
  case E_While_Loop:
    printWhileSTMT(std::dynamic_pointer_cast<WhileSTMT>(entity), indent);
    break;
  case E_If_Statement:
    printIfSTMT(std::dynamic_pointer_cast<IfSTMT>(entity), indent);
    break;
  case E_Switch_Statement:
    printSwitchSTMT(std::dynamic_pointer_cast<SwitchSTMT>(entity), indent);
    break;
  case E_Case_Statement:
    printCaseSTMT(std::dynamic_pointer_cast<CaseSTMT>(entity), indent);
    break;
  case E_Return_Statement:
    printReturnSTMT(std::dynamic_pointer_cast<ReturnSTMT>(entity), indent);
    break;
  case E_Block:
    printBlock(std::dynamic_pointer_cast<Block>(entity), indent);
    break;

  // Special entities
  case E_This:
    printThis(std::dynamic_pointer_cast<ThisEXP>(entity), indent);
    break;

  // Default case
  default:
    std::cout << std::string(indent, ' ')
              << "Unhandled Entity: " << entity->getKind() << "\n";
  }
}

void PrinterAst::printBlock(const std::shared_ptr<Block> &block, int indent) {
  std::cout << std::string(indent, ' ') << "Block | "
            << blockKindToString(block->kind) << "\n";
  for (const auto &part : block->parts) {
    printEntity(part, indent + 2);
  }
}

void PrinterAst::printIfSTMT(const std::shared_ptr<IfSTMT> &ifStmt,
                             int indent) {
  if (!ifStmt) {
    std::cout << std::string(indent, ' ') << "Invalid IfStatement (nullptr)\n";
    return;
  }

  std::cout << std::string(indent, ' ') << "IfStatement\n";

  if (ifStmt->condition) {
    std::cout << std::string(indent + 2, ' ') << "Condition:\n";
    printEntity(ifStmt->condition, indent + 4);
  } else {
    std::cout << std::string(indent + 2, ' ') << "Condition: MISSING\n";
  }

  if (ifStmt->ifTrue) {
    std::cout << std::string(indent + 2, ' ') << "Then:\n";
    printEntity(ifStmt->ifTrue, indent + 4);
  }

  if (ifStmt->ifFalse) {
    std::cout << std::string(indent + 2, ' ') << "Else:\n";
    printEntity(ifStmt->ifFalse, indent + 4);
  }
}

void PrinterAst::printExpression(const std::shared_ptr<Expression> &expr,
                                 int indent) {
  if (auto methodCall = std::dynamic_pointer_cast<MethodCallEXP>(expr)) {
    std::cout << std::string(indent, ' ')
              << "MethodCall: " << methodCall->getName() << "\n";
    printExpression(methodCall->left, indent + 2);
    for (const auto &arg : methodCall->arguments) {
      printExpression(arg, indent + 2);
    }
  } else if (auto varRef = std::dynamic_pointer_cast<VarRefEXP>(expr)) {
    std::cout << std::string(indent, ' ') << "VarRef: " << varRef->getName()
              << "\n";
  }
  // ... другие типы выражений ...
}

void PrinterAst::printVarDecl(const std::shared_ptr<VarDecl> &varDecl,
                              int indent) {
  std::cout << std::string(indent, ' ') << "VarDecl | " << varDecl->getName()
            << " : " << varDecl->type->name << "\n";
  if (varDecl->initializer) {
    std::cout << std::string(indent, ' ') << "Initializer:\n";
    printEntity(varDecl->initializer, indent + 2);
  }
}

void PrinterAst::printModuleDecl(const std::shared_ptr<ModuleDecl> &moduleDecl,
                                 int indent) {
  std::cout << std::string(indent, ' ') << "ModuleDecl | \"" << moduleDecl->getName()
            << "\"\n";

  for (const auto &decl : moduleDecl->children) {
    printEntity(decl, indent + 2);
  }
}

void PrinterAst::printClassDecl(const std::shared_ptr<ClassDecl> &classDecl,
                                int indent) {
  std::cout << std::string(indent, ' ') << "ClassDecl | \"" << classDecl->getName()
            << "\"\n";

  if (classDecl->base_class) {
    // for (const auto &base : classDecl->base_classes) {
    std::cout << std::string(indent + 4, ' ') << classDecl->base_class->getName()
              << "\n";
    // }
  }

  if (!classDecl->fields.empty()) {
    for (const auto &field : classDecl->fields) {
      printEntity(field, indent + 4);
    }
  }

  if (!classDecl->methods.empty()) {
    for (const auto &method : classDecl->methods) {
      printEntity(method, indent + 4);
    }
  }
}

void PrinterAst::printConstrDecl(const std::shared_ptr<ConstrDecl> &constrDecl,
                                 int indent) {
  std::cout << std::string(indent, ' ') << "ConstrDecl | " << constrDecl->getName()
            << "\n";

  if (!constrDecl->args.empty()) {
    for (const auto &param : constrDecl->args) {
      printEntity(param, indent + 4);
    }
  }

  if (constrDecl->body) {
    printEntity(constrDecl->body, indent + 4);
  }
}

void PrinterAst::printParamDecl(const std::shared_ptr<ParameterDecl> &paramDecl,
                                int indent) {
  std::cout << std::string(indent, ' ') << "ParamDecl | " << paramDecl->getName()
            << " : " << paramDecl->type->name << "\n";
}

void PrinterAst::printFieldDecl(const std::shared_ptr<FieldDecl> &fieldDecl,
                                int indent) {
  std::cout << std::string(indent, ' ') << "FieldDecl | " << fieldDecl->getName()
            << " : " << fieldDecl->type->name << "\n";
}

void PrinterAst::printMethodDecl(const std::shared_ptr<MethodDecl> &methodDecl,
                                 int indent) {
  std::cout << std::string(indent, ' ') << "MethodDecl | " << methodDecl->getName()
            << " : " << methodDecl->signature->name << "\n";

  if (!methodDecl->args.empty()) {
    for (const auto &param : methodDecl->args) {
      printEntity(param, indent + 4);
    }
  }

  if (methodDecl->body) {
    printEntity(methodDecl->body, indent + 4);
  }
}

void PrinterAst::printReturnSTMT(const std::shared_ptr<ReturnSTMT> &returnStmt,
                                 int indent) {
  std::cout << std::string(indent, ' ') << "ReturnSTMT\n";

  if (returnStmt->expr) {
    std::cout << std::string(indent + 2, ' ') << "Expression:\n";
    printEntity(returnStmt->expr, indent + 4);
  }
}

void PrinterAst::printAssignment(
    const std::shared_ptr<AssignmentSTMT> &assignment, int indent) {
  std::cout << std::string(indent, ' ') << "AssignmentSTMT\n";
  if (assignment->variable) {
    std::cout << std::string(indent + 2, ' ') << "LHS:\n";
    printEntity(assignment->variable, indent + 4);
  }
  if (assignment->expression) {
    std::cout << std::string(indent + 2, ' ') << "RHS:\n";
    printEntity(assignment->expression, indent + 4);
  }
}

void PrinterAst::printVarRef(const std::shared_ptr<VarRefEXP> &varRef,
                             int indent) {
  std::cout << std::string(indent, ' ') << "VarRef: " << varRef->getName()
            << "\n";
}

void PrinterAst::printMethodCall(
    const std::shared_ptr<MethodCallEXP> &methodCall, int indent) {
  std::cout << std::string(indent, ' ')
            << "MethodCall: " << methodCall->getName() << "\n";

  std::cout << std::string(indent + 2, ' ') << "Object:\n";
  printEntity(methodCall->left, indent + 4);

  std::cout << std::string(indent + 2, ' ') << "Arguments:\n";
  for (const auto &arg : methodCall->arguments) {
    printEntity(arg, indent + 4);
  }
}

void PrinterAst::printIntLiteral(
    const std::shared_ptr<IntLiteralEXP> &intLiteral, int indent) {
  std::cout << std::string(indent, ' ') << "IntLiteral: "
            << std::dynamic_pointer_cast<IntLiteralEXP>(intLiteral)->getValue()
            << "\n";
}

void PrinterAst::printFuncDecl(const std::shared_ptr<FuncDecl> &funcDecl,
                               int indent) {
  std::cout << std::string(indent, ' ') << "FuncDecl | " << funcDecl->getName()
            << "\n";

  if (!funcDecl->args.empty()) {
    std::cout << std::string(indent + 2, ' ') << "Parameters:\n";
    for (const auto &param : funcDecl->args) {
      printEntity(param, indent + 4);
    }
  }

  if (funcDecl->body) {
    std::cout << std::string(indent + 2, ' ') << "Body:\n";
    printEntity(funcDecl->body, indent + 4);
  }
}

void PrinterAst::printFuncCall(const std::shared_ptr<FuncCallEXP> &funcCall,
                               int indent) {
  std::cout << std::string(indent, ' ') << "FuncCall: " << funcCall->getName()
            << "\n";

  if (!funcCall->arguments.empty()) {
    std::cout << std::string(indent + 2, ' ') << "Arguments:\n";
    for (const auto &arg : funcCall->arguments) {
      printEntity(arg, indent + 4);
    }
  }
}

void PrinterAst::printFieldRef(const std::shared_ptr<FieldRefEXP> &fieldRef,
                               int indent) {
  std::cout << std::string(indent, ' ') << "FieldRef: " << fieldRef->getName()
            << "\n";
  if (fieldRef->obj) {
    std::cout << std::string(indent + 2, ' ') << "Object:\n";
    printEntity(fieldRef->obj, indent + 4);
  }
}

void PrinterAst::printStringLiteral(
    const std::shared_ptr<StringLiteralEXP> &strLit, int indent) {
  std::cout << std::string(indent, ' ') << "StringLiteral: " << strLit->value
            << "\n";
}

void PrinterAst::printThis(const std::shared_ptr<ThisEXP> &thisExp,
                           int indent) {
  std::cout << std::string(indent, ' ') << "This\n";
}

void PrinterAst::printForLoop(const std::shared_ptr<ForSTMT> &forLoop,
                              int indent) {
  std::cout << std::string(indent, ' ') << "ForLoop:\n";

  std::cout << std::string(indent + 2, ' ') << "Initializer:\n";
  printEntity(forLoop->varWithAss, indent + 4);

  std::cout << std::string(indent + 2, ' ') << "Condition:\n";
  printEntity(forLoop->condition, indent + 4);

  std::cout << std::string(indent + 2, ' ') << "Update:\n";
  printEntity(forLoop->post, indent + 4);

  std::cout << std::string(indent + 2, ' ') << "Body:\n";
  printEntity(forLoop->body, indent + 4);
}

void PrinterAst::printBinaryOp(const std::shared_ptr<BinaryOpEXP> &op,
                               int indent) {
  static std::unordered_map<OperatorKind, std::string> op_map = {
      {OP_PLUS, "+"},        {OP_MINUS, "-"},       {OP_MULTIPLY, "*"},
      {OP_DIVIDE, "/"},      {OP_MODULUS, "%"},     {OP_EQUAL, "=="},
      {OP_NOT_EQUAL, "!="},  {OP_MORE, ">"},        {OP_LESS, "<"},
      {OP_MORE_EQUAL, ">="}, {OP_LESS_EQUAL, "<="}, {OP_LOGIC_AND, "&&"},
      {OP_LOGIC_OR, "||"},   {OP_BIT_AND, "&"},     {OP_BIT_OR, "|"},
      {OP_BIT_XOR, "^"},     {OP_BIT_LSHIFT, "<<"}, {OP_BIT_RSHIFT, ">>"}};

  auto it = op_map.find(op->op);
  std::string op_str =
      (it != op_map.end())
          ? it->second
          : "UnknownOp(" + std::to_string(static_cast<int>(op->op)) + ")";

  std::cout << std::string(indent, ' ') << "BinaryOp: " << op_str << "\n";

  printEntity(op->left, indent + 2);
  printEntity(op->right, indent + 2);
}

void PrinterAst::printClassName(
    const std::shared_ptr<ClassNameEXP> &classNameExp, int indent) {
  std::cout << std::string(indent, ' ') << "ClassName: " << classNameExp->getName()
            << "\n";
}

void PrinterAst::printConstrCall(
    const std::shared_ptr<ConstructorCallEXP> &constrCall, int indent) {
  std::cout << std::string(indent, ' ') << "ConstrCall: ";
  printEntity(constrCall->left, 0);

  if (!constrCall->arguments.empty()) {
    std::cout << std::string(indent + 2, ' ') << "Arguments:\n";
    for (const auto &arg : constrCall->arguments) {
      printEntity(arg, indent + 4);
    }
  }
}

void PrinterAst::printUnaryOp(const std::shared_ptr<UnaryOpEXP> &op,
                              int indent) {
  static std::unordered_map<OperatorKind, std::string> op_map = {
      {OP_LOGIC_NOT, "!"},
      {OP_UNARY_MINUS, "-"},
      {OP_BIT_NOT, "~"},
      {OP_INCREMENT, "++"},
      {OP_DECREMENT, "--"}};

  auto it = op_map.find(op->op);
  std::string op_str =
      (it != op_map.end())
          ? it->second
          : "UnknownUnaryOp(" + std::to_string(static_cast<int>(op->op)) + ")";

  std::cout << std::string(indent, ' ') << "UnaryOp: " << op_str << "\n";

  if (op->operand) {
    printEntity(op->operand, indent + 2);
  } else {
    std::cout << std::string(indent + 2, ' ') << "Missing operand!\n";
  }
}

void PrinterAst::printSwitchSTMT(const std::shared_ptr<SwitchSTMT> &switchStmt,
                                 int indent) {
  std::cout << std::string(indent, ' ') << "SwitchStatement\n";

  if (switchStmt->condition) {
    std::cout << std::string(indent + 2, ' ') << "Condition:\n";
    printEntity(switchStmt->condition, indent + 4);
  }

  if (!switchStmt->cases.empty()) {
    std::cout << std::string(indent + 2, ' ') << "Cases:\n";
    for (const auto &caseStmt : switchStmt->cases) {
      printEntity(caseStmt, indent + 4);
    }
  }
}

void PrinterAst::printCaseSTMT(const std::shared_ptr<CaseSTMT> &caseStmt,
                               int indent) {
  if (caseStmt->condition_literal) {
    std::cout << std::string(indent + 2, ' ') << "Condition:\n";
    printEntity(caseStmt->condition_literal, indent + 4);
  } else {
    std::cout << std::string(indent + 2, ' ') << "Default:\n";
  }

  if (caseStmt->body) {
    std::cout << std::string(indent + 2, ' ') << "Body:\n";
    printEntity(caseStmt->body, indent + 4);
  }
}

void PrinterAst::printWhileSTMT(const std::shared_ptr<WhileSTMT> &whileStmt,
                                int indent) {
  std::cout << std::string(indent, ' ') << "WhileLoop\n";

  if (whileStmt->condition) {
    std::cout << std::string(indent + 2, ' ') << "Condition:\n";
    printEntity(whileStmt->condition, indent + 4);
  }

  if (whileStmt->body) {
    std::cout << std::string(indent + 2, ' ') << "Body:\n";
    printEntity(whileStmt->body, indent + 4);
  }
}

void PrinterAst::printEnumDecl(const std::shared_ptr<EnumDecl> &enumDecl,
                               int indent) {
  std::cout << std::string(indent, ' ') << "EnumDecl | " << enumDecl->getName()
            << ")\n";

  if (!enumDecl->items.empty()) {
    std::cout << std::string(indent + 2, ' ') << "Items:\n";
    for (const auto &[name, value] : enumDecl->items) {
      std::cout << std::string(indent + 4, ' ') << name << " = " << value
                << "\n";
    }
  }
}

void PrinterAst::printEnumRef(const std::shared_ptr<EnumRefEXP> &enumRef,
                              int indent) {
  std::cout << std::string(indent, ' ') << "EnumRef: " << enumRef->enumName
            << "::" << enumRef->itemName << "\n";
}

void PrinterAst::printArrayDecl(const std::shared_ptr<ArrayDecl> &arrDecl,
                                int indent) {
  std::cout << std::string(indent, ' ') << "ArrayDecl | " << arrDecl->getName()
            << " : " << arrDecl->type->name << "\n";

  if (arrDecl->initializer) {
    std::cout << std::string(indent + 2, ' ') << "Initializer:\n";
    printEntity(arrDecl->initializer, indent + 4);
  }
}

void PrinterAst::printRealLiteral(
    const std::shared_ptr<RealLiteralEXP> &realLit, int indent) {
  std::cout << std::string(indent, ' ')
            << "RealLiteral: " << realLit->getValue() << "\n";
}

void PrinterAst::printBoolLiteral(
    const std::shared_ptr<BoolLiteralEXP> &boolLit, int indent) {
  std::cout << std::string(indent, ' ')
            << "BoolLiteral: " << (boolLit->getValue() ? "true" : "false")
            << "\n";
}

void PrinterAst::printArrayLiteral(
    const std::shared_ptr<ArrayLiteralExpr> &arrLit, int indent) {
  std::cout << std::string(indent, ' ') << "ArrayLiteral ["
            << arrLit->elements.size() << " elements]\n";

  for (const auto &element : arrLit->elements) {
    printEntity(element, indent + 2);
  }
}