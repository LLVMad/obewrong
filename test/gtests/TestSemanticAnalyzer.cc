#include "frontend/SymbolTable.h"
#include "frontend/TypeTable.h"
#include "frontend/semantic/SemanticVisitor.h"
#include "frontend/types/Decl.h"
#include <gtest/gtest.h>
#include <memory>

class SemanticTest : public ::testing::Test {
protected:
  void SetUp() override {
    typeTable = std::make_shared<GlobalTypeTable>();
    symbolTable = std::make_shared<GlobalSymbolTable>();

    typeTable->initBuiltinTypes();
    symbolTable->initBuiltinFuncs(typeTable);
  }

  std::shared_ptr<GlobalTypeTable> typeTable;
  std::shared_ptr<GlobalSymbolTable> symbolTable;
};

// Тест 1: Корректное объявление переменной
TEST_F(SemanticTest, ValidVarDeclaration) {
  auto varDecl = std::make_shared<VarDecl>(
      "x", typeTable->builtinTypes.getType("Integer"));
  symbolTable->addToGlobalScope("Global", "Global", varDecl);

  SemanticVisitor visitor(symbolTable, typeTable);
  EXPECT_NO_THROW(visitor.visit(varDecl));
}

// Тест 2: Объявление переменной с неизвестным типом
TEST_F(SemanticTest, InvalidVarType) {
  auto invalidType = std::make_shared<TypeInt>();
  invalidType->name = "Undefined";

  auto varDecl = std::make_shared<VarDecl>("y", invalidType);
  symbolTable->addToGlobalScope("Global", "Global", varDecl);

  SemanticVisitor visitor(symbolTable, typeTable);
  EXPECT_THROW(visitor.visit(varDecl), std::runtime_error);
}

// Тест 3: Вызов существующего метода
TEST_F(SemanticTest, ValidMethodCall) {
  auto methodDecl = std::make_shared<MethodDecl>(
      "Plus",
      std::make_shared<TypeFunc>(
          typeTable->builtinTypes.getType("Integer"),
          std::vector<std::shared_ptr<Type>>{
              typeTable->builtinTypes.getType("Integer")}),
      std::vector<std::shared_ptr<Decl>>{}, true);
  symbolTable->addToGlobalScope("Global", "Global", methodDecl);

  auto leftExpr = std::make_shared<VarRefEXP>("calculator");

  auto methodCall = std::make_shared<MethodCallEXP>(
      "Plus", leftExpr, std::vector<std::shared_ptr<Expression>>{});

  SemanticVisitor visitor(symbolTable, typeTable);
  EXPECT_NO_THROW(visitor.visit(methodCall));
}