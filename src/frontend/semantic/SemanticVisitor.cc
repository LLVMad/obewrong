#include "frontend/semantic/SemanticVisitor.h"
#include <stdexcept>

void SemanticVisitor::visit(std::shared_ptr<VarDecl> node) {
  // Проверка существования типа
  if (!node->type) {
    throw std::runtime_error("Variable '" + node->name + "' has no type");
  }
  auto type = typeTable->getType("", node->type->name);
  if (!type) {
    throw std::runtime_error("Undefined type: " + node->type->name);
  }

  // Поиск переменной в текущей области видимости
  std::string parentScope = scopeStack.empty() ? "Global" : scopeStack.back();
  auto symbol = symbolTable->lookup(currentModule, parentScope, node->name);

  if (!symbol) {
    throw std::runtime_error("Variable '" + node->name + "' not declared");
  }
}

void SemanticVisitor::visit(std::shared_ptr<MethodDecl> node) {
  scopeStack.push_back(node->name);

  // Проверка параметров
  for (const auto &param : node->args) {
    if (!symbolTable->lookup("Global", node->name, param->name)) {
      throw std::runtime_error("Parameter '" + param->name + "' not found");
    }
  }

  // Проверка тела метода
  for (const auto &stmt : node->body->parts) {
    if (auto varDecl = std::dynamic_pointer_cast<VarDecl>(stmt)) {
      visit(varDecl);
    }
    // @тудуду обработка других типов узлов
  }

  scopeStack.pop_back();
}

std::shared_ptr<Type>
SemanticVisitor::getExpressionType(std::shared_ptr<Expression> expr) {
  if (auto varRef = std::dynamic_pointer_cast<VarRefEXP>(expr)) {
    auto decl = lookupInScopes(varRef->var_name); // Используем новый метод
    if (!decl) {
      throw std::runtime_error("Undefined variable: " + varRef->var_name);
    }
    auto varDecl = std::dynamic_pointer_cast<VarDecl>(decl);
    if (!varDecl) {
      throw std::runtime_error("Symbol is not a variable: " + varRef->var_name);
    }
    return varDecl->type;
  } else if (auto intLit = std::dynamic_pointer_cast<IntLiteralEXP>(expr)) {
    return typeTable->builtinTypes.getType("Integer");
  }
  // @TODO: добавить обработку других типов выражений

  throw std::runtime_error("Unsupported expression type");
}

bool SemanticVisitor::areTypesCompatible(std::shared_ptr<Type> argType,
                                         std::shared_ptr<Type> paramType) {
  if (argType->name == paramType->name)
    return true;

  // Проверка наследования классов
  if (auto paramClass = std::dynamic_pointer_cast<TypeClass>(paramType)) {
    for (const auto &base : paramClass->base_class) {
      if (areTypesCompatible(argType, base))
        return true;
    }
  }

  // Дополнительные проверки (например, числовые типы)
  if (paramType->name == "Real" && argType->name == "Integer")
    return true;

  return false;
}

std::shared_ptr<Decl> SemanticVisitor::lookupInScopes(const std::string &name) {
  // Поиск в текущих локальных областях (например, параметры метода)
  for (auto it = scopeStack.rbegin(); it != scopeStack.rend(); ++it) {
    auto &scopeName = *it;
    if (localScopes.count(scopeName) && localScopes[scopeName].count(name)) {
      return localScopes[scopeName][name];
    }
  }

  // Поиск через SymbolTable (без изменений в SymbolTable)
  std::string currentScope = scopeStack.empty() ? "Global" : scopeStack.back();
  return symbolTable->lookup(currentModule, currentScope, name);
}

void SemanticVisitor::visit(std::shared_ptr<MethodCallEXP> node) {
  // 1. Поиск метода в текущей области видимости
  std::string parentScope = scopeStack.empty() ? "Global" : scopeStack.back();
  auto methodSymbol =
      symbolTable->lookup(currentModule, parentScope, node->method_name);

  if (!methodSymbol) {
    throw std::runtime_error("Method '" + node->method_name + "' not declared");
  }

  // 2. Проверка, что символ действительно является методом
  auto methodDecl = std::dynamic_pointer_cast<MethodDecl>(methodSymbol);
  if (!methodDecl) {
    throw std::runtime_error("'" + node->method_name + "' is not a method");
  }

  // 3. Проверка количества аргументов
  if (methodDecl->args.size() != node->arguments.size()) {
    throw std::runtime_error(
        "Expected " + std::to_string(methodDecl->args.size()) +
        " arguments for '" + node->method_name + "', got " +
        std::to_string(node->arguments.size()));
  }

  // 4. Проверка типов аргументов
  for (size_t i = 0; i < node->arguments.size(); ++i) {
    auto argType = getExpressionType(node->arguments[i]);

    auto paramDecl =
        std::dynamic_pointer_cast<ParameterDecl>(methodDecl->args[i]);
    if (!paramDecl) {
      throw std::runtime_error("Invalid parameter declaration for method '" +
                               node->method_name + "'");
    }
    auto paramType = typeTable->getType("", paramDecl->type->name);

    if (!areTypesCompatible(argType, paramType)) {
      throw std::runtime_error(
          "Type mismatch for argument " + std::to_string(i + 1) + " in '" +
          node->method_name + "'. Expected: " + paramType->name +
          ", got: " + argType->name);
    }
  }
}