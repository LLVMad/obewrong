#include "frontend/semantic/SemanticVisitor.h"
#include <stdexcept>

void SemanticVisitor::visit(std::shared_ptr<VarDecl> node) {
  // Проверка типа
  if (!typeTable->getType("", node->type->name)) {
    throw std::runtime_error("Undefined type: " + node->type->name);
  }

  // Попытка добавления в текущую область
  if (!symbolTable->getCurrentScope()->addSymbol(node->name, node)) {
    throw std::runtime_error("Duplicate variable: " + node->name);
  }
}

void SemanticVisitor::visit(std::shared_ptr<MethodDecl> node) {
  symbolTable->enterScope(SCOPE_METHOD, node->name);

  for (const auto &param : node->args) {
    if (!symbolTable->getCurrentScope()->addSymbol(param->name, param)) {
      throw std::runtime_error("Duplicate parameter: " + param->name);
    }
  }

  for (const auto &stmt : node->body->parts) {
    if (auto varDecl = std::dynamic_pointer_cast<VarDecl>(stmt)) {
      visit(varDecl);
    } else if (auto methodCall =
                   std::dynamic_pointer_cast<MethodCallEXP>(stmt)) {
      visit(methodCall);
    }
    // Тут делай добавление других типы узлов по аналогии
  }
  symbolTable->exitScope();
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
  return symbolTable->getCurrentScope()->lookup(name);
}

void SemanticVisitor::visit(std::shared_ptr<MethodCallEXP> node) {
  // 1. Поиск метода в текущей области видимости
  //   std::string parentScope = scopeStack.empty() ? "Global" :
  //   scopeStack.back(); auto methodSymbol =
  //       symbolTable->lookup(currentModule, parentScope, node->method_name);

  //   if (!methodSymbol) {
  //     throw std::runtime_error("Method '" + node->method_name + "' not
  //     declared");
  //   }

  // 2. Проверка, что символ действительно является методом
  auto methodDecl = std::dynamic_pointer_cast<MethodDecl>(
      symbolTable->getCurrentScope()->lookup(node->method_name));

  if (!methodDecl) {
    throw std::runtime_error("Method '" + node->method_name + "' not found");
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