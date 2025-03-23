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

std::unique_ptr<Entity> Parser::parseExpression() {
  std::unique_ptr<Entity> node = parsePrimary();

  std::unique_ptr<Token> token = peek();
  if (token == nullptr) return node;

  // '.' is a delimeter between entitys here
  // a.k.a. a terminal char
  while (token->kind == TOKEN_DOT) {
    // if we are here
    // it means that we expect:
    // or a `.Identifier(..)` => MethodCallExpr
    // or without a `(`, just ident => FieldAccessExpr
    // or chains of them => CompoundExpr
    //
    // here we construct a tree
    // of expression(-s)

    token = next();

    std::unique_ptr<Entity> after_dot = parsePrimary();
    // @TODO throw error if after_dot is not an Identifier

    // then we decide if our identifier is a fieldaccess or a methodcall
    // - check for brackets -> MethodCall
    if (peek()->kind == TOKEN_LBRACKET) {
      token = next();

      // after_dot then is a method_name in a method call
      auto method_call = std::make_unique<MethodCallEXP>();
      parseArguments(method_call);

      token = peek();
      if (token == nullptr) return method_call;
    }

    token = peek();
    if (token == nullptr) return node;
  }
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