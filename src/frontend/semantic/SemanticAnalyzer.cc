// #include "frontend/semantic/SemanticAnalyzer.h"
// #include "frontend/parser/Expression.h"
// #include <iostream>

/*
bool SemanticAnalyzer::analyze(std::shared_ptr<Entity> root) {
  currentScope = symbolTable->getGlobalScope();
  checkEntity(root);
  return errors.empty();
}

void SemanticAnalyzer::checkEntity(const std::shared_ptr<Entity> &entity) {
  switch (entity->getKind()) {
  case E_Block:
    checkBlock(std::dynamic_pointer_cast<Block>(entity));
    break;
  case E_Method_Call:
    checkMethodCall(std::dynamic_pointer_cast<MethodCallEXP>(entity));
    break;
  // case E_Assignment:
  //     checkAssignment(std::dynamic_pointer_cast<AssignmentSTMT>(entity));
  //     break;
  // case E_Variable_Decl:
  //     checkVariableDecl(std::dynamic_pointer_cast<VarDecl>(entity));
  //     break;
  // ...
  default:
    break;
  }
}

void SemanticAnalyzer::checkBlock(const std::shared_ptr<Block> &block) {
  for (const auto &part : block->parts) {
    checkEntity(part);
  }
}

void SemanticAnalyzer::checkMethodCall(
    const std::shared_ptr<MethodCallEXP> &methodCall) {
  // Проверка типа объекта
  resolveType(methodCall->left);
  auto objType = methodCall->left->resolveType(globalTypeTable->builtinTypes);
  if (!objType) {
    reportError("Undefined object in method call", methodCall->getLoc());
    return;
  }

  // Поиск метода в классе
  auto type = globalTypeTable->getType("", objType->name);
  if (!type) {
    reportError("Type '" + objType->name + "' not found", methodCall->getLoc());
    return;
  }

  // Приведение к TypeClass
  auto classType = std::dynamic_pointer_cast<TypeClass>(type);
  if (!classType) {
    reportError("Type '" + objType->name + "' is not a class",
                methodCall->getLoc());
    return;
  }

  // Поиск метода
  auto method = classType->getMethod(methodCall->method_name);
  if (!method) {
    reportError("Method '" + methodCall->method_name + "' not found in type '" +
                    objType->name + "'",
                methodCall->getLoc());
    return;
  }

  // Проверка аргументов
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
  errors.push_back("[Semantic Error] " + message +
                   " (Line: " + std::to_string(loc.line) +
                   ", Col: " + std::to_string(loc.col) + ")");
  std::cerr << errors.back() << std::endl;
} */

// void SemanticAnalyzer::checkAssignment(const std::shared_ptr<AssignmentSTMT>&
// assignment) {
//     // Проверка типа переменной
//     if (assignment->variable) {
//         resolveType(assignment->variable);
//         auto varType =
//         assignment->variable->resolveType(globalTypeTable->builtinTypes);
//         resolveType(assignment->expression);
//         auto exprType =
//         assignment->expression->resolveType(globalTypeTable->builtinTypes);
//         if (varType != exprType) {
//             reportError("Type mismatch in assignment", assignment->getLoc());
//         }
//     } else if (assignment->field) {
//         // Проверка поля класса
//         resolveType(assignment->field->obj);
//         auto objType =
//         assignment->field->obj->resolveType(globalTypeTable->builtinTypes);
//         auto classType = std::dynamic_pointer_cast<TypeClass>(objType);
//         if (classType) {
//             auto fieldDecl =
//             classType->getField(assignment->field->field_name); if
//             (!fieldDecl) {
//                 reportError("Field '" + assignment->field->field_name + "'
//                 not found", assignment->getLoc()); return;
//             }
//             resolveType(assignment->expression);
//             auto exprType =
//             assignment->expression->resolveType(globalTypeTable->builtinTypes);
//             if (exprType != fieldDecl->type) {
//                 reportError("Type mismatch in field assignment",
//                 assignment->getLoc());
//             }
//         }
//     }
// }

// void SemanticAnalyzer::checkVariableDecl(const std::shared_ptr<VarDecl>&
// varDecl) {
//     auto declaredType = globalTypeTable->getType("", varDecl->typeName);
//     if (!declaredType) {
//         reportError("Undefined type '" + varDecl->typeName + "'",
//         varDecl->getLoc()); return;
//     }

//     if (varDecl->initializer) {
//         resolveType(varDecl->initializer);
//         auto initType =
//         varDecl->initializer->resolveType(globalTypeTable->builtinTypes); if
//         (initType != declaredType) {
//             reportError("Variable initialization type mismatch",
//             varDecl->getLoc());
//         }
//     }
// }