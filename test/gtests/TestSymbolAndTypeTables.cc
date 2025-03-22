#include <gtest/gtest.h>
#include <iostream>
#include <cassert>
#include "frontend/parser/SymbolTable.h"
#include "frontend/parser/TypeTable.h"

TEST(SymbolTableTest, AddSymbol) {
    SymbolTable symTable;
    
    // Тест 1: Добавление в глобальную область
    auto varDecl = std::make_unique<VarDecl>("x", std::make_unique<TypeInt>(), nullptr);
    bool added = symTable.addSymbol("x", std::move(varDecl));
    EXPECT_TRUE(added);

    // Тест 2: Повторное объявление
    auto varDecl2 = std::make_unique<VarDecl>("x", std::make_unique<TypeReal>(), nullptr);
    added = symTable.addSymbol("x", std::move(varDecl2));
    EXPECT_FALSE(added);

    // Тест 3: Поиск символа
    Decl* found = symTable.lookup("x");
    EXPECT_NE(found, nullptr);
    
    // Тест 4: Вложенная область
    symTable.enterScope();
    auto localVar = std::make_unique<VarDecl>("y", std::make_unique<TypeBool>(), nullptr);
    added = symTable.addSymbol("y", std::move(localVar));
    EXPECT_TRUE(added);

    // Тест 5: Поиск в родительской области
    found = symTable.lookup("x");
    EXPECT_NE(found, nullptr);

    symTable.exitScope();
}

TEST(TypeTableTest, AddAndGetType) {
    TypeTable typeTable;
    
    // Тест 1: Добавление типа
    typeTable.addType("Integer", std::make_unique<TypeInt>());
    EXPECT_TRUE(typeTable.exists("Integer"));

    // Тест 2: Получение типа
    Type* intType = typeTable.getType("Integer");
    EXPECT_NE(intType, nullptr);

    // Тест 3: Несуществующий тип
    Type* badType = typeTable.getType("Unknown");
    EXPECT_EQ(badType, nullptr);
}

TEST(TypeTableTest, AddClassArrayFunction) {
    TypeTable typeTable;
    typeTable.initBuiltinTypes();

    // Добавление класса
    std::string className = "MyClass";
    EXPECT_TRUE(typeTable.addClassType(className));

    // Добавление массива
    std::string arrayName = "Array<Integer,10>";
    EXPECT_TRUE(typeTable.addArrayType(arrayName, std::make_unique<TypeInt>(), 10));

    // Добавление функции
    std::string funcName = "Func_IntInt_Bool";
    std::vector<std::unique_ptr<Type>> args;
    args.push_back(std::make_unique<TypeInt>());
    args.push_back(std::make_unique<TypeInt>());
    EXPECT_TRUE(typeTable.addFuncType(funcName, std::make_unique<TypeBool>(), std::move(args)));
}