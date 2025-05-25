#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include "frontend/SymbolTable.h"
#include "frontend/TypeTable.h"
#include "frontend/parser/Entity.h"
#include "frontend/parser/Expression.h"
#include "frontend/parser/Statement.h"
#include "frontend/parser/Wrappers.h"
#include "frontend/types/Decl.h"
#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>

/*
class SemanticAnalyzer {
public:
  SemanticAnalyzer(std::shared_ptr<GlobalTypeTable> globalTypeTable,
                   std::shared_ptr<SymbolTable> symbolTable)
      : globalTypeTable(globalTypeTable), symbolTable(symbolTable) {}

  bool analyze(std::shared_ptr<Entity> root);
  const std::vector<std::string> &getErrors() const { return errors; }

private:
  std::shared_ptr<GlobalTypeTable> globalTypeTable;
  std::shared_ptr<SymbolTable> symbolTable;
  std::vector<std::string> errors;
  std::shared_ptr<Scope> currentScope;

  void checkEntity(const std::shared_ptr<Entity> &entity);
  void checkBlock(const std::shared_ptr<Block> &block);
  void checkMethodCall(const std::shared_ptr<MethodCallEXP> &methodCall);
  // void checkAssignment(const std::shared_ptr<AssignmentSTMT>& assignment);
  // void checkVariableDecl(const std::shared_ptr<VarDecl>& varDecl);
  void checkFunctionDecl(const std::shared_ptr<FuncDecl> &funcDecl);
  void resolveType(const std::shared_ptr<Expression> &expr);
  void reportError(const std::string &message, const Loc &loc);
}; */

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

class SemanticAnalyzer : public BaseVisitor,
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
  SemanticAnalyzer(std::shared_ptr<GlobalTypeTable> globalTypeTable,
                   std::shared_ptr<SymbolTable> symbolTable)
      : globalTypeTable(globalTypeTable), symbolTable(symbolTable) {
    globalTypeTable->initBuiltinTypes();
    symbolTable->initBuiltinFunctions(globalTypeTable);
    currentScope = symbolTable->getGlobalScope();
  }

  void printAST(const std::shared_ptr<ModuleDecl> &root) {
    root->accept(*this);
  }

  // not used !
  void visit(Entity &entity) override {}

  void visit(Block &block) override {}

  void visit(EDummy &dummy) override {}

  void visit(Statement &stmt) override {}

  void reportError(const std::string &message, const Loc &loc) {
    errors.push_back(" (Line: " + std::to_string(loc.line + 1) +
                     ", Col: " + std::to_string(loc.col + 1) + ")" +
                     "[Semantic Error] " + message);
    std::cerr << errors.back() << std::endl;
  }

  void visit(AssignmentSTMT &stmt) override {
    if (stmt.variable)
      stmt.variable->accept(*this);
    if (stmt.field)
      stmt.field->accept(*this);
    if (stmt.element)
      stmt.element->accept(*this);
    if (stmt.expression)
      stmt.expression->accept(*this);
  }

  void visit(ReturnSTMT &stmt) override {
    if (!currentReturnType && !currentFuncIsVoid) {
      reportError("Return outside function context", stmt.getLoc());
      return;
    }

    if (currentFuncIsVoid) {
      if (stmt.expr) {
        reportError("Void function cannot return a value", stmt.getLoc());
      }
    } else {
      if (!stmt.expr) {
        reportError("Non-void function must return a value", stmt.getLoc());
        return;
      }

      auto returnType = stmt.expr->resolveType(
          globalTypeTable->types[currentModuleName], currentScope);
      if (!returnType || returnType->name != currentReturnType->name) {
        reportError("Return type mismatch: expected '" +
                        currentReturnType->name + "'",
                    stmt.getLoc());
        return;
      }
    }
  }

  void visit(IfSTMT &stmt) override {
    auto condType = stmt.condition->resolveType(
        globalTypeTable->types[currentModuleName], currentScope);
    if (condType->name != "Bool") {
      reportError("If condition must be boolean", stmt.condition->getLoc());
    }

    stmt.ifTrue->accept(*this);

    if (stmt.ifFalse)
      stmt.ifFalse->accept(*this);
  }

  void visit(CaseSTMT &stmt) override {
    if (stmt.condition_literal)
      stmt.condition_literal->accept(*this);
    stmt.body->accept(*this);
  }

  void visit(SwitchSTMT &stmt) override {
    auto condType = stmt.condition->resolveType(
        globalTypeTable->types[currentModuleName], currentScope);

    for (const auto &caseStmt : stmt.cases) {
      caseStmt->accept(*this);
      if (caseStmt->condition_literal) {
        auto caseType = caseStmt->condition_literal->resolveType(
            globalTypeTable->types[currentModuleName], currentScope);
        if (caseType != condType) {
          reportError("Case type mismatch with switch condition",
                      caseStmt->condition_literal->getLoc());
        }
      }
    }
  }

  void visit(WhileSTMT &stmt) override {
    auto condType = stmt.condition->resolveType(
        globalTypeTable->types[currentModuleName], currentScope);

    if (condType->name != "Bool") {
      reportError("While loop condition must be boolean",
                  stmt.condition->getLoc());
    }

    stmt.body->accept(*this);
  }

  void visit(ForSTMT &stmt) override {
    currentScope = currentScope->nextScope();
    if (stmt.varWithAss)
      stmt.varWithAss->accept(*this);

    if (stmt.condition) {
      auto condType = stmt.condition->resolveType(
          globalTypeTable->types[currentModuleName], currentScope);
      if (condType->name != "Bool") {
        reportError("For loop condition must be boolean",
                    stmt.condition->getLoc());
      }
    }

    if (stmt.post)
      stmt.post->accept(*this);

    stmt.body->accept(*this);
    currentScope = currentScope->prevScope();
  }

  void visit(Expression &expr) override {}

  void visit(IntLiteralEXP &expr) override {
    auto type = expr.resolveType(globalTypeTable->types[currentModuleName],
                                 currentScope);
    if (!type || type->name != "Integer") {
      reportError("Invalid integer literal type", expr.getLoc());
    }
  }

  void visit(RealLiteralEXP &expr) override {
    auto type = expr.resolveType(globalTypeTable->types[currentModuleName],
                                 currentScope);
    if (!type || type->name != "Real") {
      reportError("Invalid real literal type", expr.getLoc());
    }
  }

  void visit(StringLiteralEXP &expr) override {
    auto type = expr.resolveType(globalTypeTable->types[currentModuleName],
                                 currentScope);
    if (!type || type->name != "String") {
      reportError("Invalid string literal type", expr.getLoc());
    }
  }

  void visit(BoolLiteralEXP &expr) override {
    auto type = expr.resolveType(globalTypeTable->types[currentModuleName],
                                 currentScope);
    if (!type || type->name != "Bool") {
      reportError("Invalid boolean literal type", expr.getLoc());
    }
  }

  void visit(ArrayLiteralExpr &expr) override {
    if (expr.elements.empty()) {
      reportError("Array literal cannot be empty", expr.getLoc());
      return;
    }

    auto firstType = expr.elements[0]->resolveType(
        globalTypeTable->types[currentModuleName], currentScope);
    for (const auto &elem : expr.elements) {
      auto elemType = elem->resolveType(
          globalTypeTable->types[currentModuleName], currentScope);
      if (elemType != firstType) {
        reportError("All array elements must be of the same type",
                    elem->getLoc());
      }
    }
  }

  void visit(VarRefEXP &expr) override {
    auto [decl, alloc, isInited] = *currentScope->getSymbol(expr.getName());
    if (!decl) {
      reportError("Undefined variable '" + expr.getName() + "'", expr.getLoc());
    }
  }

  void visit(FieldRefEXP &expr) override {
    auto objType = expr.obj->resolveType(
        globalTypeTable->types[currentModuleName], currentScope);

    auto classType = std::dynamic_pointer_cast<TypeClass>(objType);
    if (!classType) {
      reportError("Field access on non-class type", expr.getLoc());
      return;
    }

    if (!classType->getField(expr.getName())) {
      reportError("Undefined field '" + expr.getName() + "' in class",
                  expr.getLoc());
    }
  }

  void visit(ElementRefEXP &expr) override {
    if (expr.arr)
      expr.arr->accept(*this);
    if (expr.index)
      expr.index->accept(*this);
  }

  void visit(MethodCallEXP &expr) override {
    if (expr.left)
      expr.left->accept(*this);

    for (const auto &arg : expr.arguments) {
      arg->accept(*this);
    }
  }

  void visit(FuncCallEXP &expr) override {
    if (!expr.arguments.empty()) {
      for (const auto &arg : expr.arguments) {
        arg->accept(*this);
      }
    }
  }

  void visit(ClassNameEXP &expr) override {
    auto classType = globalTypeTable->getType(
        symbolTable->getCurrentScope()->getName(), expr.getName());
    if (!classType) {
      reportError("Undefined class name: " + expr.getName(), expr.getLoc());
      return;
    }

    if (!std::dynamic_pointer_cast<TypeClass>(classType)) {
      reportError("'" + expr.getName() + "' is not a class", expr.getLoc());
    }
  }

  void visit(ConstructorCallEXP &expr) override {
    auto classType = globalTypeTable->getType("", expr.left->getName());

    bool isBuiltinTypes = false;
    for (const auto &[name, type] : globalTypeTable->builtinTypes.types) {
      if (name == expr.left->getName())
        isBuiltinTypes = true;
    }

    if (!classType and !isBuiltinTypes) {
      reportError("Undefined class '" + expr.left->getName() + "'",
                  expr.getLoc());
      return;
    }
  }

  void visit(CompoundEXP &expr) override {
    for (const auto &part : expr.parts) {
      part->accept(*this);
    }
  }

  void visit(ThisEXP &expr) override {
    // auto classScope = symbolTable->getEnclosingClassScope(currentScope);
    // if (!classScope) {
    //   reportError("'this' used outside class context", thisExp->getLoc());
    // }
  }

  void visit(ConversionEXP &expr) override { expr.from->accept(*this); }

  void visit(BinaryOpEXP &expr) override {
    auto leftType = expr.left->resolveType(
        globalTypeTable->types[currentModuleName], currentScope);
    auto rightType = expr.right->resolveType(
        globalTypeTable->types[currentModuleName], currentScope);

    if (leftType->name != rightType->name) {
      reportError("Type mismatch for operator " + getOperatorString(expr.op),
                  expr.getLoc());
    }

    if (expr.op == OP_PLUS || expr.op == OP_MINUS || expr.op == OP_MULTIPLY ||
        expr.op == OP_DIVIDE) {
      if (leftType->name != "Integer" && leftType->name != "Real") {
        reportError("Operator " + getOperatorString(expr.op) +
                        " cannot be applied to non-numeric type",
                    expr.getLoc());
      }
    }

    expr.left->accept(*this);
    expr.right->accept(*this);
  }

  void visit(UnaryOpEXP &expr) override {
    // auto operandType = resolveExprType(unaryOp->operand);

    // if (operandType == ) {
    //   reportError("Unary operator '" + getUnaryOperatorString(unaryOp->op) +
    //              "' cannot be applied to type '" + operandType->name + "'",
    //              unaryOp->getLoc());
    // }
  }

  void visit(EnumRefEXP &expr) override {
    // auto [decl, alloc, isInited] = *currentScope->getSymbol(expr.enumName);
    // if (!decl || decl->getKind() != E_Enum_Decl) {
    //   reportError("Undefined enumeration '" + expr.enumName + "'",
    //               expr.getLoc());
    //   return;
    // }

    // auto enumType = std::dynamic_pointer_cast<EnumDecl>(decl);
    // if (!enumType->items.count(expr.itemName)) {
    //   reportError("Undefined item '" + expr.itemName + "' in enumeration '" +
    //                   expr.enumName + "'",
    //               expr.getLoc());
    // }
  }

  void visit(Decl &decl) override {}

  void visit(FieldDecl &decl) override {
    auto fieldType = globalTypeTable->getType("", decl.type->name);
    if (!fieldType) {
      reportError("Undefined field type '" + decl.type->name + "'",
                  decl.getLoc());
    }

    auto [fieldDecl, alloc, isInited] =
        *currentScope->getSymbol(decl.type->name);
    if (isInited) {
      reportError("Duplicate field name: " + decl.type->name, decl.getLoc());
    }
  }

  void visit(VarDecl &decl) override {
    if (!decl.type) {
      reportError("Variable '" + decl.getName() + "' has no type",
                  decl.getLoc());
    }

    std::shared_ptr<Type> declaredType;
    if (currentScope->getName() != "main") {
      declaredType = globalTypeTable->getType(
          symbolTable->getCurrentScope()->getName(), decl.type->name);
    } else {
      declaredType =
          globalTypeTable->getType(currentModuleName, decl.type->name);
    }

    bool isBuiltinTypes = false;
    for (const auto &[name, type] : globalTypeTable->builtinTypes.types) {
      if (name == decl.type->name)
        isBuiltinTypes = true;
    }
    if (!declaredType and !isBuiltinTypes) {
      reportError("Undefined type '" + decl.type->name + "'", decl.getLoc());
    }

    if (decl.initializer) {
      decl.initializer->accept(*this);
    }
  }

  void visit(ParameterDecl &decl) override { //
    auto paramType = globalTypeTable->getType("", decl.type->name);

    bool isBuiltinTypes = false;
    for (const auto &[name, type] : globalTypeTable->builtinTypes.types) {
      if (name == decl.type->name or decl.type->name == "access")
        isBuiltinTypes = true;
    }
    if (!paramType and !isBuiltinTypes) {
      reportError("Undefined parameter type '" + decl.type->name + "'",
                  decl.getLoc());
    }

    auto [paramDecl, alloc, isInited] =
        *currentScope->getSymbol(decl.getName());
    if (isInited) {
      reportError("Duplicate parameter name: " + decl.getName(), decl.getLoc());
    }
  }

  void visit(MethodDecl &decl) override {
    currentScope = currentScope->nextScope();
    currentReturnType = decl.isVoid ? nullptr : decl.signature->return_type;
    currentFuncIsVoid = decl.isVoid;
    if (!decl.isVoid) {
      auto returnType =
          globalTypeTable->getType("", decl.signature->return_type->name);
      bool isBuiltinTypes = false;
      for (const auto &[name, type] : globalTypeTable->builtinTypes.types) {
        if (name == decl.signature->return_type->name)
          isBuiltinTypes = true;
      }
      if (!returnType and !isBuiltinTypes) {
        reportError("Undefined return type '" +
                        decl.signature->return_type->name + "'",
                    decl.getLoc());
      }
    }

    auto [methodDecl, alloc, isInited] =
        *currentScope->getSymbol(decl.getName());
    if (isInited) {
      reportError("Duplicate method name: " + decl.getName(), decl.getLoc());
    }

    for (const auto &param : decl.args) {
      param->accept(*this);
    }

    if (decl.body) {
      decl.body->accept(*this);
    }
    currentReturnType = nullptr;
    currentFuncIsVoid = false;

    currentScope = currentScope->prevScope();
  }

  void visit(ConstrDecl &decl) override {
    currentScope = currentScope->nextScope();
    auto [constrDecl, alloc, isInited] =
        *currentScope->getSymbol(decl.getName());

    if (!isInited) {
      reportError("Constructor '" + decl.getName() +
                      "' is not in a class scope",
                  decl.getLoc());
    }

    for (const auto &param : decl.args) {
      param->accept(*this);
    }

    if (decl.body) {
      decl.body->accept(*this);
    }
    currentScope = currentScope->prevScope();
  }

  void visit(FuncDecl &decl) override { //
    currentScope = currentScope->nextScope();
    currentReturnType = decl.isVoid ? nullptr : decl.signature->return_type;
    currentFuncIsVoid = decl.isVoid;
    if (!decl.isVoid) {
      auto returnType =
          globalTypeTable->getType("", decl.signature->return_type->name);
      if (!returnType) {
        reportError("Undefined return type '" +
                        decl.signature->return_type->name + "'",
                    decl.getLoc());
      }
    }

    auto [funcDecl, alloc, isInited] = *currentScope->getSymbol(decl.getName());
    if (isInited) {
      reportError("Duplicate function name: " + decl.getName(), decl.getLoc());
    }

    for (const auto &param : decl.args) {
      param->accept(*this);
    }

    if (decl.body) {
      decl.body->accept(*this);
    }

    currentReturnType = nullptr;
    currentFuncIsVoid = false;
    currentScope = currentScope->prevScope();
  }

  void visit(ClassDecl &decl) override {
    // std::string moduleName = symbolTable->getCurrentScope()->getName();

    // if (globalTypeTable->getType(currentScope->getName(), decl.getName())) {
    //   reportError("Duplicate class name: " + decl.getName(), decl.getLoc());
    // }
    currentScope = currentScope->nextScope();

    if (!decl.fields.empty()) {
      for (const auto &field : decl.fields) {
        field->accept(*this);
      }
    }
    if (!decl.methods.empty()) {
      for (const auto &method : decl.methods) {
        method->accept(*this);
      }
    }
    currentScope = currentScope->prevScope();
  }

  void visit(ArrayDecl &decl) override {
    auto elementType = globalTypeTable->getType("", decl.type->name);
    if (!elementType) {
      reportError("Undefined array element type '" + decl.type->name + "'",
                  decl.getLoc());
    }

    if (decl.size <= 0) {
      reportError("Invalid array size: " + std::to_string(decl.size),
                  decl.getLoc());
    }

    auto [arrayDecl, alloc, isInited] =
        *currentScope->getSymbol(decl.getName());
    if (isInited) {
      reportError("Duplicate array name: " + decl.getName(), decl.getLoc());
    }

    if (decl.initializer) {
      decl.initializer->accept(*this);
    }
  }

  void visit(ListDecl &decl) override {
    if (decl.initializer) {
      decl.initializer->accept(*this);
    }
  }

  void visit(ModuleDecl &decl) override {
    currentScope = currentScope->nextScope();
    currentModuleName = decl.getName();
    if (decl.children.empty()) {
      reportError("Module '" + decl.getName() + "' is empty", decl.getLoc());
    }

    for (const auto &child : decl.children) {
      child->accept(*this);
    }

    currentScope = currentScope->prevScope();
  }

  void visit(EnumDecl &decl) override {
    auto [enumDecl, alloc, isInited] = *currentScope->getSymbol(decl.getName());
    if (isInited) {
      reportError("Duplicate enum name: " + decl.getName(), decl.getLoc());
    }

    std::unordered_set<std::string> items;
    for (auto &item : decl.items) {
      if (items.count(item.first)) {
        reportError("Duplicate enum item: " + item.first, decl.getLoc());
      }
      items.insert(item.first);
    }
  }

private:
  std::shared_ptr<GlobalTypeTable> globalTypeTable;
  std::shared_ptr<SymbolTable> symbolTable;
  std::vector<std::string> errors;
  std::shared_ptr<Scope<Entity>> currentScope;

  std::shared_ptr<Type> currentReturnType;
  bool currentFuncIsVoid = false;
  std::string currentModuleName;
};

#endif