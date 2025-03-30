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
    symbolTable = std::make_shared<SymbolTable>();

    typeTable->initBuiltinTypes();
    symbolTable->initBuiltinFunctions(typeTable);
  }

  std::shared_ptr<GlobalTypeTable> typeTable;
  std::shared_ptr<SymbolTable> symbolTable;
};

// Тест 1: Корректное объявление переменной
TEST_F(SemanticTest, ValidVarDeclaration) {
  // Проверяем, что тип "Integer" инициализирован
  auto integerType = typeTable->getType("", "Integer");
  ASSERT_NE(integerType, nullptr)
      << "Тип 'Integer' не найден в GlobalTypeTable";

  auto varDecl = std::make_shared<VarDecl>("x", integerType);

  SemanticVisitor visitor(symbolTable, typeTable);
  EXPECT_NO_THROW(visitor.visit(varDecl));
}

// Тест 2: Объявление переменной с неизвестным типом
TEST_F(SemanticTest, InvalidVarType) {
  auto invalidType = std::make_shared<TypeInt>();
  invalidType->name = "UndefinedType";

  auto varDecl = std::make_shared<VarDecl>("y", invalidType);

  SemanticVisitor visitor(symbolTable, typeTable);
  EXPECT_THROW(visitor.visit(varDecl), std::runtime_error);
}

// Тест 3: Вызов существующего метода
TEST_F(SemanticTest, ValidMethodCall) {
  // Создаем параметр метода
  auto param =
      std::make_shared<ParameterDecl>("x", typeTable->getType("", "Integer"));

  // Создаем метод Plus с одним параметром
  auto methodDecl = std::make_shared<MethodDecl>(
      "Plus",
      std::make_shared<TypeFunc>(typeTable->getType("", "Integer"),
                                 std::vector<std::shared_ptr<Type>>{
                                     typeTable->getType("", "Integer")}),
      std::vector<std::shared_ptr<Decl>>{param}, // Добавляем параметр
      true);

  // Добавляем метод в глобальную область видимости
  symbolTable->getGlobalScope()->addSymbol("Plus", methodDecl);

  // Создаем аргумент для вызова метода
  auto arg =
      std::make_shared<IntLiteralEXP>(42); // Пример аргумента типа Integer
  std::vector<std::shared_ptr<Expression>> args = {arg};

  // Создаем вызов метода с корректным количеством аргументов
  auto leftExpr = std::make_shared<VarRefEXP>("calculator");
  auto methodCall =
      std::make_shared<MethodCallEXP>("Plus", leftExpr,
                                      args // Передаем один аргумент
      );

  SemanticVisitor visitor(symbolTable, typeTable);
  EXPECT_NO_THROW(visitor.visit(methodCall));
}