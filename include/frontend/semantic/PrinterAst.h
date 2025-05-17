#ifndef PRINTER_AST_H
#define PRINTER_AST_H

#include "frontend/SymbolTable.h"
#include "frontend/TypeTable.h"
#include "frontend/parser/Entity.h"
#include "frontend/parser/Expression.h"
#include "frontend/parser/Statement.h"
#include "frontend/parser/Wrappers.h"
#include "frontend/types/Decl.h"

#include <iostream>
#include <string>
#include <vector>

inline std::string blockKindToString(BlockKind kind) {
  switch (kind) {
  case BLOCK_IN_CLASS:
    return "BLOCK_IN_CLASS";
  case BLOCK_IN_METHOD:
    return "BLOCK_IN_METHOD";
  case BLOCK_IN_FUNCTION:
    return "BLOCK_IN_FUNCTION";
  case BLOCK_IN_WHILE:
    return "BLOCK_IN_WHILE";
  case BLOCK_IN_IF:
    return "BLOCK_IN_IF";
  case BLOCK_IN_FOR:
    return "BLOCK_IN_FOR";
  case BLOCK_IN_SWITCH:
    return "BLOCK_IN_SWITCH";
  default:
    return "UNKNOWN_BLOCK";
  }
}

class PrinterAst : public VisitorBase,
                  public Visitor<Entity>,
                  public Visitor<Block>,
                  public Visitor<EDummy>,
                  public Visitor<Statement>,
                  public Visitor<AssignmentSTMT>,
                  public Visitor<ReturnSTMT>,
                  public Visitor<IfSTMT>,
                  public Visitor<CaseSTMT>,
                  public Visitor<SwitchSTMT>,
                  public Visitor<WhileSTMT>,
                  public Visitor<ForSTMT>,
                  public Visitor<Expression>,
                  public Visitor<IntLiteralEXP>,
                  public Visitor<RealLiteralEXP>,
                  public Visitor<StringLiteralEXP>,
                  public Visitor<BoolLiteralEXP>,
                  public Visitor<ArrayLiteralExpr>,
                  public Visitor<VarRefEXP>,
                  public Visitor<FieldRefEXP>,
                  public Visitor<ElementRefEXP>,
                  public Visitor<MethodCallEXP>,
                  public Visitor<FuncCallEXP>,
                  public Visitor<ClassNameEXP>,
                  public Visitor<ConstructorCallEXP>,
                  public Visitor<CompoundEXP>,
                  public Visitor<ThisEXP>,
                  public Visitor<ConversionEXP>,
                  public Visitor<BinaryOpEXP>,
                  public Visitor<UnaryOpEXP>,
                  public Visitor<EnumRefEXP>,
                  public Visitor<Decl>,
                  public Visitor<FieldDecl>,
                  public Visitor<VarDecl>,
                  public Visitor<ParameterDecl>,
                  public Visitor<MethodDecl>,
                  public Visitor<ConstrDecl>,
                  public Visitor<FuncDecl>,
                  public Visitor<ClassDecl>,
                  public Visitor<ArrayDecl>,
                  public Visitor<ListDecl>,
                  public Visitor<ModuleDecl>,
                  public Visitor<EnumDecl> {
public:
  PrinterAst(std::shared_ptr<GlobalTypeTable> globalTypeTable,
             std::shared_ptr<SymbolTable> symbolTable)
      : globalTypeTable(globalTypeTable), symbolTable(symbolTable), indent(0) {}

  void printAST(const std::shared_ptr<ModuleDecl> &root) {
    root->acceptAGuestVisitor(this);
  }

  // Entity visit methods
  void visit(Entity& entity) override {
    printIndent();
    std::cout << "Entity: " << entity.getName() << std::endl;
  }

  void visit(Block& block) override {
    printIndent();
    std::cout << "Block: " << blockKindToString(block.kind) << std::endl;
    indent += 2;
    for (auto& part : block.parts) {
      DERIVED(part, this);
    }
    indent -= 2;
  }

  void visit(EDummy& dummy) override {
    printIndent();
    std::cout << "Dummy" << std::endl;
  }

  // Statement visit methods
  void visit(Statement& stmt) override {
    printIndent();
    std::cout << "Statement" << std::endl;
  }

  void visit(AssignmentSTMT& stmt) override {
    printIndent();
    std::cout << "Assignment" << std::endl;
    indent += 2;
    if (stmt.variable) stmt.variable->acceptAGuestVisitor(this);
    if (stmt.field) stmt.field->acceptAGuestVisitor(this);
    if (stmt.element) stmt.element->acceptAGuestVisitor(this);
    if (stmt.expression) stmt.expression->acceptAGuestVisitor(this);
    indent -= 2;
  }

  void visit(ReturnSTMT& stmt) override {
    printIndent();
    std::cout << "Return" << std::endl;
    if (stmt.expr) {
      indent += 2;
      stmt.expr->acceptAGuestVisitor(this);
      indent -= 2;
    }
  }

  void visit(IfSTMT& stmt) override {
    printIndent();
    std::cout << "If" << std::endl;
    indent += 2;
    stmt.condition->acceptAGuestVisitor(this);
    stmt.ifTrue->acceptAGuestVisitor(this);
    if (stmt.ifFalse) {
      DERIVED(stmt.ifFalse, this)
      // stmt.ifFalse->acceptAGuestVisitor(this);
    }
    indent -= 2;
  }

  void visit(CaseSTMT& stmt) override {
    printIndent();
    std::cout << "Case" << std::endl;
    indent += 2;
    if (stmt.condition_literal) stmt.condition_literal->acceptAGuestVisitor(this);
    stmt.body->acceptAGuestVisitor(this);
    indent -= 2;
  }

  void visit(SwitchSTMT& stmt) override {
    printIndent();
    std::cout << "Switch" << std::endl;
    indent += 2;
    stmt.condition->acceptAGuestVisitor(this);
    for (const auto& case_stmt : stmt.cases) {
      case_stmt->acceptAGuestVisitor(this);
    }
    indent -= 2;
  }

  void visit(WhileSTMT& stmt) override {
    printIndent();
    std::cout << "While" << std::endl;
    indent += 2;
    stmt.condition->acceptAGuestVisitor(this);
    stmt.body->acceptAGuestVisitor(this);
    indent -= 2;
  }

  void visit(ForSTMT& stmt) override {
    printIndent();
    std::cout << "For" << std::endl;
    indent += 2;
    stmt.varWithAss->acceptAGuestVisitor(this);
    stmt.condition->acceptAGuestVisitor(this);
    stmt.post->acceptAGuestVisitor(this);
    stmt.body->acceptAGuestVisitor(this);
    indent -= 2;
  }

  // Expression visit methods
  void visit(Expression& expr) override {
    printIndent();
    std::cout << "Expression" << std::endl;
  }

  void visit(IntLiteralEXP& expr) override {
    printIndent();
    std::cout << "IntLiteral: " << expr.getValue() << std::endl;
  }

  void visit(RealLiteralEXP& expr) override {
    printIndent();
    std::cout << "RealLiteral: " << expr.getValue() << std::endl;
  }

  void visit(StringLiteralEXP& expr) override {
    printIndent();
    std::cout << "StringLiteral: " << expr.value << std::endl;
  }

  void visit(BoolLiteralEXP& expr) override {
    printIndent();
    std::cout << "BoolLiteral: " << expr.getValue() << std::endl;
  }

  void visit(ArrayLiteralExpr& expr) override {
    printIndent();
    std::cout << "ArrayLiteral" << std::endl;
    indent += 2;
    for (const auto& element : expr.elements) {
      element->acceptAGuestVisitor(this);
    }
    indent -= 2;
  }

  void visit(VarRefEXP& expr) override {
    printIndent();
    std::cout << "VarRef: " << expr.getName() << std::endl;
  }

  void visit(FieldRefEXP& expr) override {
    printIndent();
    std::cout << "FieldRef: " << expr.getName() << std::endl;
    if (expr.obj) {
      indent += 2;
      expr.obj->acceptAGuestVisitor(this);
      indent -= 2;
    }
  }

  void visit(ElementRefEXP& expr) override {
    printIndent();
    std::cout << "ElementRef" << std::endl;
    indent += 2;
    if (expr.arr) expr.arr->acceptAGuestVisitor(this);
    if (expr.index) expr.index->acceptAGuestVisitor(this);
    indent -= 2;
  }

  void visit(MethodCallEXP& expr) override {
    printIndent();
    std::cout << "MethodCall: " << expr.getName() << std::endl;
    indent += 2;
    if (expr.left) expr.left->acceptAGuestVisitor(this);
    for (const auto& arg : expr.arguments) {
      arg->acceptAGuestVisitor(this);
    }
    indent -= 2;
  }

  void visit(FuncCallEXP& expr) override {
    printIndent();
    std::cout << "FuncCall: " << expr.getName() << std::endl;
    indent += 2;
    for (const auto& arg : expr.arguments) {
      arg->acceptAGuestVisitor(this);
    }
    indent -= 2;
  }

  void visit(ClassNameEXP& expr) override {
    printIndent();
    std::cout << "ClassName: " << expr.getName() << std::endl;
  }

  void visit(ConstructorCallEXP& expr) override {
    printIndent();
    std::cout << "ConstructorCall" << std::endl;
    indent += 2;
    expr.left->acceptAGuestVisitor(this);
    for (const auto& arg : expr.arguments) {
      arg->acceptAGuestVisitor(this);
    }
    indent -= 2;
  }

  void visit(CompoundEXP& expr) override {
    printIndent();
    std::cout << "Compound" << std::endl;
    indent += 2;
    for (const auto& part : expr.parts) {
      part->acceptAGuestVisitor(this);
    }
    indent -= 2;
  }

  void visit(ThisEXP& expr) override {
    printIndent();
    std::cout << "This" << std::endl;
  }

  void visit(ConversionEXP& expr) override {
    printIndent();
    std::cout << "Conversion" << std::endl;
    indent += 2;
    expr.from->acceptAGuestVisitor(this);
    indent -= 2;
  }

  void visit(BinaryOpEXP& expr) override {
    printIndent();
    std::cout << "BinaryOp" << std::endl;
    indent += 2;
    expr.left->acceptAGuestVisitor(this);
    expr.right->acceptAGuestVisitor(this);
    indent -= 2;
  }

  void visit(UnaryOpEXP& expr) override {
    printIndent();
    std::cout << "UnaryOp" << std::endl;
    indent += 2;
    expr.operand->acceptAGuestVisitor(this);
    indent -= 2;
  }

  void visit(EnumRefEXP& expr) override {
    printIndent();
    std::cout << "EnumRef: " << expr.enumName << "." << expr.itemName << std::endl;
  }

  // Declaration visit methods
  void visit(Decl& decl) override {
    printIndent();
    std::cout << "Decl: " << decl.getName() << std::endl;
  }

  void visit(FieldDecl& decl) override {
    printIndent();
    std::cout << "FieldDecl: " << decl.getName() << std::endl;
  }

  void visit(VarDecl& decl) override {
    printIndent();
    std::cout << "VarDecl: " << decl.getName() << std::endl;
    if (decl.initializer) {
      indent += 2;
      decl.initializer->acceptAGuestVisitor(this);
      indent -= 2;
    }
  }

  void visit(ParameterDecl& decl) override {
    printIndent();
    std::cout << "ParameterDecl: " << decl.getName() << std::endl;
  }

  void visit(MethodDecl& decl) override {
    printIndent();
    std::cout << "MethodDecl: " << decl.getName() << std::endl;
    indent += 2;
    for (const auto& arg : decl.args) {
      arg->acceptAGuestVisitor(this);
    }
    if (decl.body) decl.body->acceptAGuestVisitor(this);
    indent -= 2;
  }

  void visit(ConstrDecl& decl) override {
    printIndent();
    std::cout << "ConstrDecl: " << decl.getName() << std::endl;
    indent += 2;
    for (const auto& arg : decl.args) {
      arg->acceptAGuestVisitor(this);
    }
    if (decl.body) decl.body->acceptAGuestVisitor(this);
    indent -= 2;
  }

  void visit(FuncDecl& decl) override {
    printIndent();
    std::cout << "FuncDecl: " << decl.getName() << std::endl;
    indent += 2;
    for (const auto& arg : decl.args) {
      arg->acceptAGuestVisitor(this);
    }
    if (decl.body) decl.body->acceptAGuestVisitor(this);
    indent -= 2;
  }

  void visit(ClassDecl& decl) override {
    printIndent();
    std::cout << "ClassDecl: " << decl.getName() << std::endl;
    indent += 2;
    for (const auto& field : decl.fields) {
      field->acceptAGuestVisitor(this);
    }
    for (auto& method : decl.methods) {
      auto m = std::static_pointer_cast<Entity>(method);
      DERIVED(m, this)
      // method->acceptAGuestVisitor(this);
    }
    indent -= 2;
  }

  void visit(ArrayDecl& decl) override {
    printIndent();
    std::cout << "ArrayDecl: " << decl.getName() << std::endl;
    if (decl.initializer) {
      indent += 2;
      decl.initializer->acceptAGuestVisitor(this);
      indent -= 2;
    }
  }

  void visit(ListDecl& decl) override {
    printIndent();
    std::cout << "ListDecl: " << decl.getName() << std::endl;
    if (decl.initializer) {
      indent += 2;
      decl.initializer->acceptAGuestVisitor(this);
      indent -= 2;
    }
  }

  void visit(ModuleDecl& decl) override {
    printIndent();
    std::cout << "ModuleDecl: " << decl.getName() << std::endl;
    indent += 2;
    for (auto& child : decl.children) {
      DERIVED(child, this)
      // child->acceptAGuestVisitor(this);
    }
    indent -= 2;
  }

  void visit(EnumDecl& decl) override {
    printIndent();
    std::cout << "EnumDecl: " << decl.getName() << std::endl;
    indent += 2;
    for (const auto& [name, value] : decl.items) {
      printIndent();
      std::cout << name << " = " << value << std::endl;
    }
    indent -= 2;
  }

private:
  std::shared_ptr<GlobalTypeTable> globalTypeTable;
  std::shared_ptr<SymbolTable> symbolTable;
  int indent;

  void printIndent() {
    for (int i = 0; i < indent; i++) {
      std::cout << " ";
    }
  }
};

#endif