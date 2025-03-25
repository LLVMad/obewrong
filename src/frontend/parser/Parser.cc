#include "frontend/parser/Parser.h"

#include "frontend/parser/Expression.h"
#include "frontend/parser/Statement.h"
#include "frontend/parser/SymbolTable.h"
#include "frontend/parser/TypeTable.h"

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
  return parseExpression();
}


std::unique_ptr<Entity> Parser::parseSwitch() {
  std::unique_ptr<Token> token = peek();
  if (token == nullptr || token->kind != TOKEN_SWITCH) return nullptr;
  token = next();

  auto condition =
    std::unique_ptr<Expression>(dynamic_cast<Expression *>(parseExpression().release()));

  auto switch_st = std::make_unique<SwitchSTMT>(std::move(condition));

  token = peek();
  if (token == nullptr || token->kind != TOKEN_BBEGIN) return nullptr;
  while (token->kind != TOKEN_BEND) {
    token = next();

    // parse case stmt
    auto case_st = std::unique_ptr<CaseSTMT>(dynamic_cast<CaseSTMT*>(parseCase().release()));
    switch_st->addCase(std::move(case_st));

    token = peek();
    if (token == nullptr) return nullptr;
  }

  return switch_st;
}

std::unique_ptr<Entity> Parser::parseCase() {
  std::unique_ptr<Token> token = peek();
  if (token == nullptr || token->kind != TOKEN_CASE) return nullptr;
  token = next(); // eat 'case'

  // case literal condition
  auto cond_lit =
    std::unique_ptr<Expression>(dynamic_cast<Expression *>(parsePrimary().release()));

  token = peek();
  if (token == nullptr || token->kind != TOKEN_THEN) return nullptr;
  token = next();

  // case body
  auto body_block =
    std::unique_ptr<Block>(dynamic_cast<Block *>(parseBlock().release()));

  return std::make_unique<CaseSTMT>(std::move(cond_lit), std::move(body_block));
}

std::unique_ptr<Entity> Parser::parseIfStatement() {
  std::unique_ptr<Token> token = peek();
  if (token == nullptr || token->kind != TOKEN_ELSE) return nullptr;
  token = next(); // eat 'if'

  auto condition =
    std::unique_ptr<Expression>(dynamic_cast<Expression *>(parseExpression().release()));

  auto ifTrue =
    std::unique_ptr<Block>(dynamic_cast<Block *>(parseBlock().release()));

  if (peek()->kind == TOKEN_ELSE) {
    auto ifFalse =
      std::unique_ptr<Block>(dynamic_cast<Block *>(parseBlock().release()));

    return std::make_unique<IfSTMT>(std::move(condition), std::move(ifTrue), std::move(ifFalse));
  }

  return std::make_unique<IfSTMT>(std::move(condition), std::move(ifTrue));
}

std::unique_ptr<Entity> Parser::parseVarDecl() {
  std::unique_ptr<Token> token = peek();
  if (token == nullptr || token->kind != TOKEN_VAR_DECL) return nullptr;
  token = next(); // eat 'var' lexeme

  // read var name
  token = next();
  auto var_name = std::get<std::string>(token->value);

  // check if type specifier is present
  if (peek()->kind != TOKEN_COLON) return nullptr; // @TODO error unspecified type

  token = next(); // eat ':'

  // read type
  token = next();
  auto var_type = typeTable.getType(std::get<std::string>(token->value));

  // type not found ?
  // @TODO return some cool error
  if (var_type == nullptr)
    return nullptr;

  auto var = std::make_unique<VarDecl>(var_name, var_type);

  // read initializer
  if (peek()->kind != TOKEN_ASSIGNMENT) return var;
  token = next();
  auto initializer =
    std::unique_ptr<Expression>(dynamic_cast<Expression *>(parseExpression().release()));
  var->initializer = std::move(initializer);

  return var;
}

std::unique_ptr<Entity> Parser::parseAssignment() {
  std::unique_ptr<Token> token = peek();
  if (token == nullptr || token->kind != TOKEN_IDENTIFIER) return nullptr;

  // read lvalue var name
  token = next();
  auto var_name = std::get<std::string>(token->value);
  auto var_ref = std::make_unique<VarRefEXP>(var_name);

  // eat ':='
  if (peek()->kind != TOKEN_COLON) return nullptr;
  token = next();

  // read rvalue expression
  if (peek()->kind != TOKEN_ASSIGNMENT) return nullptr; // @TODO
  token = next();
  auto initializer =
    std::unique_ptr<Expression>(dynamic_cast<Expression *>(parseExpression().release()));
  // var->initializer = std::move(initializer);

  auto ass = std::make_unique<AssignmentSTMT>(std::move(var_ref), std::move(initializer));

  return ass;
}

std::unique_ptr<Entity> Parser::parseBlock() {
  std::unique_ptr<Token> token = peek();
  if (token == nullptr || token->kind != TOKEN_BBEGIN || token->kind != TOKEN_THEN || token->kind != TOKEN_LOOP) return nullptr;

  std::vector<std::unique_ptr<Entity>> block_body;

  while (token->kind != TOKEN_BEND) {
    std::unique_ptr<Entity> part;

    token = next();
    switch (token->kind) {
      case TOKEN_VAR_DECL: {
        part = parseVarDecl();
      } break;
      case TOKEN_IDENTIFIER: {
        // a.set(...)
        // printl(...)
        if (peek()->kind == TOKEN_DOT) part = parseExpression();

        // a := a.set(...)
        if (peek()->kind == TOKEN_ASSIGNMENT) part = parseAssignment();
      } break;
      case TOKEN_IF: {
        part = parseIfStatement();
      } break;
      case TOKEN_WHILE: {
        // part = parseWhileStatement();
      } break;
      case TOKEN_FOR: {
        // part = parseForStatement();
      } break;
      case TOKEN_RETURN: {
        part = parseReturnStatement();
      } break;
      case TOKEN_SWITCH: {
        part = parseSwitch();
      } break;
      // @TODO
      default: return nullptr;
    }

    block_body.push_back(std::move(part));

    token = peek();
    if (token == nullptr) return nullptr;
  }

  return std::make_unique<Block>(std::move(block_body));
}

std::unique_ptr<Entity> Parser::parseReturnStatement() {
  std::unique_ptr<Token> token = peek();
  if (token == nullptr || token->kind != TOKEN_RETURN) return nullptr;

  // eat 'return' lexeme
  token = next();

  std::unique_ptr<Expression> expr =
    std::unique_ptr<Expression>(dynamic_cast<Expression*>(parseExpression().release()));

  auto ret = std::make_unique<ReturnSTMT>(std::move(expr));
  return ret;
}

std::unique_ptr<Entity> Parser::parseExpression() {
  std::unique_ptr<Entity> node = parsePrimary();

  // what it could be
  std::unique_ptr<CompoundEXP> comp;

  auto node_to_exp = std::unique_ptr<Expression>(dynamic_cast<Expression*>(node.release()));
  comp->addExpression(std::move(node_to_exp));

  // std::unique_ptr<MethodCallEXP> method_call;
  // std::unique_ptr<FieldAccessEXP> field_access;

  std::unique_ptr<Token> token = peek();
  if (token == nullptr) return node;

  if (token->kind == TOKEN_LBRACKET) {
    // its a function call or a constructor call
    token = next();

    if (peek()->kind == TOKEN_LBRACKET) {
      token = next();

      auto func_call = std::make_unique<FuncCallEXP>();
      parseArguments(func_call);

      // check if its a constructor call
      if (this->typeTable.exists(func_call->func_name)) {
        auto class_name_expr = std::make_unique<ClassNameEXP>(func_call->func_name);
        auto constr_call = std::make_unique<ConstructorCallEXP>(
          std::move(class_name_expr),
          std::move(func_call->arguments));
        return constr_call;
      }

      // auto func_call_to_exp = std::unique_ptr<Expression>(func_call.release());
      // comp->addExpression(func_call_to_exp);

      token = peek();
      if (token == nullptr) return nullptr;
      // eat closing bracket after func read
      token = next();

      return func_call;
    }
  }

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

    // should be an identifier
    std::unique_ptr<Entity> after_dot = parsePrimary();
    // @TODO throw error if after_dot is not an Identifier

    // then we decide if our identifier is a fieldaccess or a methodcall
    // - check for brackets -> MethodCall
    if (peek()->kind == TOKEN_LBRACKET) {
      token = next();

      // after_dot then is a method_name in a method call
      auto method_call = std::make_unique<MethodCallEXP>();
      parseArguments(method_call);

      auto method_call_to_exp = std::unique_ptr<Expression>(method_call.release());
      comp->addExpression(std::move(method_call_to_exp));

      token = peek();
      if (token == nullptr) return method_call;
      // eat closing bracket after method read
      if (token->kind == TOKEN_RBRACKET) { token = next(); }
    }
    // - if not its a field access, than cannot be chained mind you
    //   so we can probably just return after read ?
    else {
      token = next();

      // read identifier of a field
      // std::unique_ptr<Entity> field = parsePrimary();
      // auto left = std::unique_ptr<Expression>(dynamic_cast<Expression*>(node.release()));
      auto field_access = std::make_unique<FieldRefEXP>(std::get<std::string>(next()->value));
      auto field_access_to_exp = std::unique_ptr<Expression>(field_access.release());
      comp->addExpression(std::move(field_access_to_exp));
    }

    token = peek();
    if (token == nullptr) return node;
  }

  // number of expr delimitered by a dot
  // - 0 -> cant be, a.
  // - 1 -> methodcall or fieldaccess, a.Plus(2), Pair.first
  // - >1 -> compound
  size_t parts_num = comp->parts.size();
  if (parts_num == 0) {
    return std::unique_ptr<Entity>(comp->parts[0].release());
  };
  if (parts_num == 1) {
    switch (comp->parts[1]->getKind()) {
      case E_Method_Call: {
      auto left = std::move(comp->parts[0]);
        std::unique_ptr<MethodCallEXP> method_call =
          std::unique_ptr<MethodCallEXP>(
              dynamic_cast<MethodCallEXP *>(comp->parts[1].release()));
        return method_call;
      }
      case E_Field_Reference: {
        std::unique_ptr<FieldAccessEXP> field_access;
        auto left = std::move(comp->parts[0]);
        auto field_ref = std::unique_ptr<FieldAccessEXP>(dynamic_cast<FieldAccessEXP *>(comp->parts[1].release()));
        field_access->left = std::move(left);
        field_access->field_name = field_ref->field_name;
        return field_access;
      }
      default: return comp;
    }
  }

  return comp;
}

// void Parser::parseParameters(const std::unique_ptr<FuncDecl>& funcDecl) {
//   // @TODO
// }
//
// void Parser::parseParameters(const std::unique_ptr<MethodDecl>& funcDecl) {
//   // @TODO
// }

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

    std::unique_ptr<Entity> after_comma = parseExpression();
    auto uexpr = std::unique_ptr<Expression>(dynamic_cast<Expression*>(after_comma.release()));
    method_name->children.push_back(std::move(uexpr));

    token = peek();
    if (token == nullptr) return;
  }
}

void Parser::parseArguments(const std::unique_ptr<FuncCallEXP> &function_name) {
  auto node = parseExpression();
  auto expr = std::unique_ptr<Expression>(dynamic_cast<Expression*>(node.release()));
  function_name->arguments.push_back(std::move(expr));

  std::unique_ptr<Token> token = peek();
  if (token == nullptr) return;

  // arg, arg, arg
  // ',' is a delimiter
  while (token->kind == TOKEN_COMMA) {
    token = next();

    std::unique_ptr<Entity> after_comma = parseExpression();
    auto uexpr = std::unique_ptr<Expression>(dynamic_cast<Expression*>(after_comma.release()));
    function_name->children.push_back(std::move(uexpr));

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
    return std::make_unique<StringLiteralEXP>(
        std::get<std::string>(token->value));
  }
  case TOKEN_SELFREF: {
    return std::make_unique<ThisEXP>();
  }
  case TOKEN_IDENTIFIER: {
    auto var = symbolTable.lookup(std::get<std::string>(token->value));
    if (!var)
      throw std::runtime_error("Undefined variable: " +
                               std::get<std::string>(token->value));
    switch (var->getKind()) {
      case E_Field_Decl: {
        return std::make_unique<FieldRefEXP>(std::get<std::string>(token->value));
      }
      case E_Variable_Decl: {
        return std::make_unique<VarRefEXP>(std::get<std::string>(token->value));
      }
      default: return nullptr;
    }

  }
  default:
    return nullptr;
  }
}