// #include <stdio.h>
// #include <iostream>

// #include "frontend/Lexer.h"
// #include "frontend/SourceManager.h"

// int main(int argc, char *argv[]) {
//   if (argc != 3) {}

//   SourceManager sm;
//   auto buff = std::make_shared<SourceBuffer>(sm.readSource(argv[1]));

//   Lexer lexer(buff);

//   int times = std::stoi(argv[2]);
//   for (int i = 0; i < times; i++) {
//     std::unique_ptr<Token> next_tok = lexer.next();
//     if (next_tok) {
//       std::cout << "[ "
//                 << next_tok->line << ":"
//                 << next_tok->column << " "
//                 << Lexer::getTokenTypeName(next_tok->type);

//       // if (!next_tok->.empty()) std::cout << ",\t" << next_tok->lexeme;
//       // if (next_tok->value.identId) std::cout << ",\t" << next_tok->value.identId;
//       switch (next_tok->value.index()) {
//         case 1: {
//           std::cout << ", " << std::get<int>(next_tok->value);
//         } break;
//         case 2: {
//           std::cout << ", " << std::get<double>(next_tok->value);
//         } break;
//         case 3: {
//           std::cout << ", " << std::get<std::string>(next_tok->value);
//         } break;
//         default: break;
//       }

//       std::cout << " ]" << std::endl;
//     }
//     else std::cout << "nullptr" << std::endl;
//   }
// }

#include <iostream>
#include <cassert>
#include "frontend/parser/SymbolTable.h"
#include "frontend/parser/TypeTable.h"

// Тест SymbolTable
void testSymbolTable() {
  SymbolTable symTable;
  
  // Тест 1: Добавление в глобальную область
  auto varDecl = std::make_unique<VarDecl>("x", std::make_unique<TypeInt>(), nullptr);
  bool added = symTable.addSymbol("x", std::move(varDecl));
  std::cout << "Test 1: " << (added ? "PASSED" : "FAILED") << "\n";

  // Тест 2: Повторное объявление
  auto varDecl2 = std::make_unique<VarDecl>("x", std::make_unique<TypeReal>(), nullptr);
  added = symTable.addSymbol("x", std::move(varDecl2));
  std::cout << "Test 2: " << (!added ? "PASSED" : "FAILED") << "\n";

  // Тест 3: Поиск символа
  Decl* found = symTable.lookup("x");
  std::cout << "Test 3: " << (found ? "PASSED" : "FAILED") << "\n";
  
  // Тест 4: Вложенная область
  symTable.enterScope();
  auto localVar = std::make_unique<VarDecl>("y", std::make_unique<TypeBool>(), nullptr);
  added = symTable.addSymbol("y", std::move(localVar));
  std::cout << "Test 4: " << (added ? "PASSED" : "FAILED") << "\n";

  // Тест 5: Поиск в родительской области
  found = symTable.lookup("x");
  std::cout << "Test 5: " << (found ? "PASSED" : "FAILED") << "\n";

  symTable.exitScope();
}

// Тест TypeTable
void testTypeTable() {
    TypeTable typeTable;
    
    // Тест 1: Добавление типа
    typeTable.addType("Integer", std::make_unique<TypeInt>());
    std::cout << "Test 1: " << (typeTable.exists("Integer") ? "PASSED" : "FAILED") << "\n";

    // Тест 2: Получение типа
    Type* intType = typeTable.getType("Integer");
    std::cout << "Test 2: " << (intType ? "PASSED" : "FAILED") << "\n";

    // Тест 3: Несуществующий тип
    Type* badType = typeTable.getType("Unknown");
    std::cout << "Test 3: " << (!badType ? "PASSED" : "FAILED") << "\n";
}

void testTypeTable2() {
  TypeTable typeTable;
  typeTable.initBuiltinTypes();

  // Добавление класса
  std::string className = "MyClass";
  if (!typeTable.addClassType(className)) {
      throw std::runtime_error("Class " + className + " already exists");
  }
  std::cout << "Test 1 (Add class): PASSED\n";

  // Добавление массива
  // Type* intType = typeTable.getType("Integer");
  std::string arrayName = "Array<Integer,10>";
  if (!typeTable.addArrayType(arrayName, std::make_unique<TypeInt>(), 10)) {
      throw std::runtime_error("Array " + arrayName + " already exists");
  }
  std::cout << "Test 2 (Add array): PASSED\n";

  // Добавление функции
  std::string funcName = "Func_IntInt_Bool";
  std::vector<std::unique_ptr<Type>> args;
  args.push_back(std::make_unique<TypeInt>());
  args.push_back(std::make_unique<TypeInt>());
  if (!typeTable.addFuncType(funcName, std::make_unique<TypeBool>(), std::move(args))) {
      throw std::runtime_error("Function " + funcName + " already exists");
  }
  std::cout << "Test 3 (Add function): PASSED\n";
}

int main() {
    std::cout << "=== Testing SymbolTable ===\n";
    testSymbolTable();
    
    std::cout << "\n=== Testing TypeTable ===\n";
    testTypeTable();

    std::cout << "\n=== Testing TypeTable2 ===\n";
    testTypeTable2();
    
    return 0;
}