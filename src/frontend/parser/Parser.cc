#include "frontend/parser/Parser.h"

#include "frontend/parser/Expression.h"

std::unique_ptr<Token> Parser::next() {
  if (tokens[tokenPos + 1]->kind != TOKEN_EOF) {
    return std::make_unique<Token>(*tokens[++tokenPos]);
  }

  return nullptr;
}

std::unique_ptr<Token> Parser::peek() {
  if (tokens[tokenPos + 1]->kind != TOKEN_EOF) {
    return std::make_unique<Token>(*tokens[tokenPos + 1]);
  }

  return nullptr;
}

std::unique_ptr<Entity> Parser::parseProgram() {
  auto program = std::make_unique<Block>(std::vector<std::unique_ptr<Expression>>());
  
  // Парсим выражения и добавляем их в блок
  while (peek() != nullptr) {
      auto expr = parseExpression();
      if (expr) {
          if (auto expression = dynamic_cast<Expression*>(expr.get())) {
              program->body.push_back(std::unique_ptr<Expression>(expression));
              expr.release(); // Освобождаем владение у expr, чтобы избежать двойного удаления
          } else {
              throw std::runtime_error("Expected Expression type in parseProgram");
          }
      }
  }

  return program;
}

std::unique_ptr<Entity> Parser::parseExpression() {
  std::unique_ptr<Entity> node = parsePrimary();

  std::unique_ptr<Token> token = peek();
  if (token == nullptr) return node;

  // Обработка точек (FieldAccess или MethodCall)
  while (token->kind == TOKEN_DOT) {
      token = next();

      std::unique_ptr<Entity> after_dot = parsePrimary();
      if (!after_dot) {
          throw std::runtime_error("Expected identifier after dot");
      }

      if (peek()->kind == TOKEN_LBRACKET) {
          token = next();

          // Создаем MethodCall
          auto method_call = std::make_unique<MethodCallEXP>();
          parseArguments(method_call);

          return method_call;
      } else {
          auto field_access = std::make_unique<FieldAccessEXP>();

          if (auto expr = dynamic_cast<Expression*>(node.get())) {
              field_access->left.reset(expr);
              node.release(); // Освобождаем владение у node, чтобы избежать двойного удаления
          } else {
              throw std::runtime_error("Expected Expression type for left side of FieldAccess");
          }

          field_access->field_name = std::get<std::string>(token->value);
          node = std::move(field_access);
      }

      token = peek();
      if (token == nullptr) return node;
  }

  return node;
}


void Parser::parseArguments(const std::unique_ptr<MethodCallEXP> &method_name) {
  auto node = parseExpression();
  auto expr = std::unique_ptr<Expression>(dynamic_cast<Expression*>(node.release()));
  method_name->arguments.push_back(std::move(expr));

  std::unique_ptr<Token> token = peek();
  if (token == nullptr) return;

  // arg, arg, arg
  // ',' is a delimiter
  while (token->kind == TOKEN_COMMA) {
    token = next();

    // add a node to a fake root
    std::unique_ptr<Entity> after_comma = parseExpression();
    auto uexpr = std::unique_ptr<Expression>(dynamic_cast<Expression*>(after_comma.release()));
    method_name->children.push_back(std::move(uexpr));

    token = peek();
    if (token == nullptr) return;
  }
}

std::unique_ptr<Entity> Parser::parsePrimary() {
  std::unique_ptr<Token> token = peek();
  if (token == nullptr) return nullptr;

  token = next();
  switch (token->kind) {
    case TOKEN_INT_NUMBER: {
      return std::make_unique<IntLiteralEXP>(std::get<int>(token->value));
    }
    case TOKEN_REAL_NUMBER: {
      return std::make_unique<RealLiteralEXP>(std::get<double>(token->value));
    }
    case TOKEN_BOOL_TRUE: {
      return std::make_unique<BoolLiteralEXP>(true);
    }
    case TOKEN_BOOL_FALSE: {
      return std::make_unique<BoolLiteralEXP>(false);
    }
    case TOKEN_STRING: {
      return std::make_unique<StringLiteralEXP>(std::get<std::string>(token->value));
    }
    // should be translated to something meaningful in upper steps
    case TOKEN_IDENTIFIER: {
      return std::make_unique<VarRefEXP>(std::get<std::string>(token->value));
    }
    default: return nullptr;
  }
}