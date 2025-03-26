#include "frontend/parser/Parser.h"

#include "../../../include/frontend/SymbolTable.h"
#include "../../../include/frontend/TypeTable.h"
#include "frontend/parser/Expression.h"
#include "frontend/parser/Statement.h"

bool isTypeName(TokenKind kind) {
  switch (kind) {
  case TOKEN_TYPE_STRING:
  case TOKEN_TYPE_INT32:
  case TOKEN_TYPE_INT64:
  case TOKEN_TYPE_INT16:
  case TOKEN_TYPE_U32:
  case TOKEN_TYPE_U16:
  case TOKEN_TYPE_U64:
  case TOKEN_TYPE_REAL:
  case TOKEN_TYPE_F64:
  case TOKEN_TYPE_BOOL:
  case TOKEN_TYPE_LIST:
  case TOKEN_TYPE_ARRAY:
  case TOKEN_TYPE_ANYVAL:
  case TOKEN_TYPE_ANYREF:
  case TOKEN_TYPE_TYPE:
      return true;
  default:
    return false;
  }
}

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

std::shared_ptr<Entity> Parser::parseProgram() {
  // return parseExpression();
  std::unique_ptr<Token> token = peek();
  if (token == nullptr || token->kind != TOKEN_MODULE_DECL) return nullptr;
  token = next(); // eat 'module'

  // read module name
  token = peek();
  if (token == nullptr || token->kind != TOKEN_IDENTIFIER) return nullptr;
  token = next();

  std::shared_ptr<Entity> root = std::make_shared<ModuleDecl>(std::get<std::string>(token->value));
  moduleName = std::get<std::string>(token->value);
  lastDeclaredScopeParent.emplace("Global");

  // auto printl = std::make_shared<FuncDecl>()
  // globalSymbolTable.addToGlobalScope(moduleName, "Global", )

  token = peek();
  while (token->kind != TOKEN_EOF) {
    // in global scope we can wait for class or function
    switch (token->kind) {
      case TOKEN_CLASS: {
        // token = next();
        root->next = parseClassDecl();
      } break;
      // case TOKEN_FUNC: {
      //   root->next = parseFunctionDecl();
      // }
      default: break;
    }

    token = peek();
  }

  return root;
}

std::shared_ptr<Entity> Parser::parseMethodDecl() {
  std::unique_ptr<Token> token = peek();
  if (token == nullptr || token->kind != TOKEN_METHOD) return nullptr;
  token = next(); // eat 'method'

  // get method name
  token = peek();
  if (token == nullptr || token->kind != TOKEN_IDENTIFIER) return nullptr;
  token = next();
  auto method_name = std::get<std::string>(token->value);

  // new our scope is this method
  lastDeclaredScopeParent.emplace(method_name);

  auto method =
    std::make_shared<MethodDecl>(method_name);

  // get method parameters
  parseParameters(method);

  std::vector<std::shared_ptr<Type>> args_types;
  if (!method->isVoided) {
    for (const auto &param : method->args) {
      auto param_decl =
        std::dynamic_pointer_cast<ParameterDecl>(param);
      args_types.push_back(param_decl->type);
    }
  }

  // get return type
  token = peek();
  if (isTypeName(token->kind)) {
    // no return type
    // build signature
    auto signature = std::make_shared<TypeFunc>(args_types);
    method->isVoid= true;

    // get body of method
    auto body_block =
      std::dynamic_pointer_cast<Block>(parseBlock(BLOCK_IN_CLASS));

    method->body = body_block;

    // quit scope
    lastDeclaredScopeParent.pop();

    return method;
  };

  token = next();
  auto return_type =
    globalTypeTable.getType(moduleName, std::get<std::string>(token->value));

  // build signature
  auto signature =
    method->isVoided ?
      std::make_shared<TypeFunc>(return_type)
      :
      std::make_shared<TypeFunc>(return_type, args_types);

  // get body of method
  auto body_block =
    std::dynamic_pointer_cast<Block>(parseBlock(BLOCK_IN_CLASS));

  method->body = body_block;

  // quit scope
  lastDeclaredScopeParent.pop();

  return method;
}

std::shared_ptr<Entity> Parser::parseSwitch() {
  std::unique_ptr<Token> token = peek();
  if (token == nullptr || token->kind != TOKEN_SWITCH) return nullptr;
  token = next();

  auto condition =
    std::shared_ptr<Expression>(dynamic_cast<Expression *>(parseExpression().get()));

  auto switch_st = std::make_shared<SwitchSTMT>(std::move(condition));

  token = peek();
  if (token == nullptr || token->kind != TOKEN_BBEGIN) return nullptr;
  while (token->kind != TOKEN_BEND) {
    token = next();

    // parse case stmt
    auto case_st = std::shared_ptr<CaseSTMT>(dynamic_cast<CaseSTMT*>(parseCase().get()));
    switch_st->addCase(std::move(case_st));

    token = peek();
    if (token == nullptr) return nullptr;
  }

  return switch_st;
}

std::shared_ptr<Entity> Parser::parseCase() {
  std::unique_ptr<Token> token = peek();
  if (token == nullptr || token->kind != TOKEN_CASE) return nullptr;
  token = next(); // eat 'case'

  // case literal condition
  auto cond_lit =
    std::shared_ptr<Expression>(dynamic_cast<Expression *>(parsePrimary().get()));

  token = peek();
  if (token == nullptr || token->kind != TOKEN_THEN) return nullptr;
  token = next();

  // case body
  auto body_block =
    std::shared_ptr<Block>(dynamic_cast<Block *>(parseBlock(BLOCK_IN_SWITCH).get()));

  return std::make_shared<CaseSTMT>(std::move(cond_lit), std::move(body_block));
}

std::shared_ptr<Entity> Parser::parseIfStatement() {
  std::unique_ptr<Token> token = peek();
  if (token == nullptr || token->kind != TOKEN_ELSE) return nullptr;
  token = next(); // eat 'if'

  auto condition =
    std::shared_ptr<Expression>(dynamic_cast<Expression *>(parseExpression().get()));

  auto ifTrue =
    std::shared_ptr<Block>(dynamic_cast<Block *>(parseBlock(BLOCK_IN_IF).get()));

  if (peek()->kind == TOKEN_ELSE) {
    auto ifFalse =
      std::shared_ptr<Block>(dynamic_cast<Block *>(parseBlock(BLOCK_IN_IF).get()));

    return std::make_shared<IfSTMT>(std::move(condition), std::move(ifTrue), std::move(ifFalse));
  }

  return std::make_shared<IfSTMT>(std::move(condition), std::move(ifTrue));
}

std::shared_ptr<Entity> Parser::parseVarDecl() {
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
  auto var_type = globalTypeTable.getType(moduleName, std::get<std::string>(token->value));

  // type not found ?
  // @TODO return some cool error
  if (var_type == nullptr)
    return nullptr;

  auto var = std::make_shared<VarDecl>(var_name, var_type);

  // read initializer
  if (peek()->kind != TOKEN_ASSIGNMENT) return var;
  token = next();
  auto initializer =
    std::shared_ptr<Expression>(dynamic_cast<Expression *>(parseExpression().get()));
  var->initializer = std::move(initializer);

  this->globalSymbolTable.addToGlobalScope(moduleName, lastDeclaredScopeParent.top(), var.get());

  return var;
}

std::shared_ptr<Entity> Parser::parseAssignment() {
  std::unique_ptr<Token> token = peek();
  if (token == nullptr || token->kind != TOKEN_IDENTIFIER) return nullptr;

  // read lvalue var name
  token = next();
  auto var_name = std::get<std::string>(token->value);
  auto var_ref = std::make_shared<VarRefEXP>(var_name);

  // eat ':='
  if (peek()->kind != TOKEN_COLON) return nullptr;
  token = next();

  // read rvalue expression
  if (peek()->kind != TOKEN_ASSIGNMENT) return nullptr; // @TODO
  token = next();
  auto initializer =
    std::shared_ptr<Expression>(dynamic_cast<Expression *>(parseExpression().get()));
  // var->initializer = std::move(initializer);

  auto ass = std::make_shared<AssignmentSTMT>(std::move(var_ref), std::move(initializer));

  return ass;
}

std::shared_ptr<Entity> Parser::parseBlock(BlockKind blockKind) {
  std::unique_ptr<Token> token = peek();
  if (token == nullptr || (token->kind != TOKEN_BBEGIN && token->kind != TOKEN_THEN && token->kind != TOKEN_LOOP)) return nullptr;
  token = next();
  std::vector<std::shared_ptr<Entity>> block_body;

  while (token->kind != TOKEN_BEND) {
    std::shared_ptr<Entity> part;

    token = peek();
    switch (token->kind) {
      case TOKEN_VAR_DECL: {
        if (blockKind > 0)
          part = parseVarDecl();
        else // in class
          part = parseFieldDecl();
      } break;
      case TOKEN_IDENTIFIER: {
        // a.set(...)
        // printl(...)
        if (peek()->kind == TOKEN_DOT) part = parseExpression();

        // a := a.set(...)
        else if (peek()->kind == TOKEN_ASSIGNMENT) part = parseAssignment();

        else part = parseExpression();
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
      case TOKEN_SELFREF: {
        // @TODO this() or this.var
        switch (blockKind) {
          case BLOCK_IN_CLASS: {
            part = parseConstructorDecl();
          } break;
          // case BLOCK_IN_METHOD: {
          //   part =
          // }
          default: break;
        }
      }
      // @TODO
      default: return nullptr;
    }

    block_body.push_back(std::move(part));

    token = peek();
    if (token == nullptr) return nullptr;
  }

  // eat 'end'
  token = next();

  return std::make_shared<Block>(std::move(block_body), blockKind);
}

std::shared_ptr<Entity> Parser::parseConstructorDecl() {
  std::unique_ptr<Token> token = peek();
  if (token == nullptr || token->kind != TOKEN_SELFREF) return nullptr;
  token = next(); // eat 'this'

  // ????
  // lastDeclaredScopeParent.emplace());

  // read parameters
  // @TODO name constructor
  auto constr = std::make_shared<ConstrDecl>("This");

  // read params
  parseParameters(constr);

  std::vector<std::shared_ptr<Type>> args_types;
  if (!constr->isDefault) {
    for (const auto &param : constr->args) {
      auto param_decl =
        std::dynamic_pointer_cast<ParameterDecl>(param);
      args_types.push_back(param_decl->type);
    }
  }

  // get return type ???? no return type in constructorts lol
  // token = next();
  // auto return_type =
  //   globalTypeTable.getType(moduleName, std::get<std::string>(token->value));

  // build signature
  auto signature =
    constr->isDefault ?
      std::make_shared<TypeFunc>()
      :
      std::make_shared<TypeFunc>(args_types);

  // read constr body
  auto body_block
    = std::dynamic_pointer_cast<Block>(parseBlock(BLOCK_IN_METHOD));

  constr->body = body_block;

  return constr;
}

std::shared_ptr<Entity> Parser::parseClassDecl() {
  std::unique_ptr<Token> token = peek();
  if (token == nullptr || token->kind != TOKEN_CLASS) return nullptr;
  token = next(); // eat 'class'

  // read classname
  if (peek()->kind != TOKEN_IDENTIFIER) return nullptr;
  token = next();
  auto class_name = std::get<std::string>(token->value);

  // now our scope is this class
  lastDeclaredScopeParent.emplace(class_name);

  // see if there is extends
  if (peek()->kind == TOKEN_EXTENDS) {
    token = next();

    token = next();
    auto base_class =
      globalSymbolTable.lookup(moduleName, "Global", std::get<std::string>(token->value));

    auto base_class_type =
      globalTypeTable.getType(moduleName, std::get<std::string>(token->value));
  }

  // read body of class
  auto body_block =
    std::shared_ptr<Block>(dynamic_cast<Block *>(parseBlock(BLOCK_IN_CLASS).get()));

  std::vector<std::shared_ptr<Type>> fieldTypes;
  std::vector<std::shared_ptr<TypeFunc>> methodTypes;
  std::vector<std::shared_ptr<FieldDecl>> fields;
  std::vector<std::shared_ptr<MethodDecl>> methods;
  std::vector<std::shared_ptr<MethodDecl>> constructors;

  for (auto &ent : body_block->parts) {
    switch (ent->getKind()) {
      case E_Field_Decl: {
        auto field = std::dynamic_pointer_cast<FieldDecl>(ent);
        fields.push_back(std::move(field));
        fieldTypes.push_back(std::move(field->type));
      } break;
      case E_Method_Decl: {
        auto method = std::dynamic_pointer_cast<MethodDecl>(ent);
        methods.push_back(std::move(method));
        methodTypes.push_back(std::move(method->signature));
      } break;
      case E_Constructor_Decl: {
        // @TODO
      } break;
      default: break;
    }
  }

  lastDeclaredScopeParent.pop();

  auto class_new_type =
    std::make_shared<TypeClass>(class_name, fieldTypes, methodTypes);
  // add new type (class) to LE SYMBOOOOOL TAABLEEEEESS si si papa
  globalTypeTable.addType(moduleName, class_name, std::move(class_new_type));

  auto class_stmt =
    std::make_shared<ClassDecl>(class_name, class_new_type, fields, methods, constructors);
  globalSymbolTable.addToGlobalScope(moduleName, "Global", class_stmt.get());

  return class_stmt;
}

std::shared_ptr<Entity> Parser::parseFieldDecl() {
  std::unique_ptr<Token> token = peek();
  if (token == nullptr || token->kind != TOKEN_VAR_DECL) return nullptr;
  token = next();

  // read lvalue var name
  token = next();
  auto var_name = std::get<std::string>(token->value);
  // auto var_ref = std::make_shared<VarRefEXP>(var_name);

  // eat ':'
  if (peek()->kind != TOKEN_COLON) return nullptr;
  token = next();

  // read type
  if (peek()->kind != TOKEN_IDENTIFIER) return nullptr;
  token = next();
  auto var_type = globalTypeTable.types[moduleName].getType(std::get<std::string>(token->value));

  auto ass = std::make_shared<FieldDecl>(std::move(var_name), var_type);

  this->globalSymbolTable
    .addToGlobalScope(this->moduleName, this->lastDeclaredClass, ass.get());
  // this->symbolTable.addSymbol()

  return ass;
}

std::shared_ptr<Entity> Parser::parseWhileStatement() {
  std::unique_ptr<Token> token = peek();
  if (token == nullptr || token->kind != TOKEN_WHILE) return nullptr;
  token = next(); // eat 'while'

  auto condition =
    std::shared_ptr<Expression>(dynamic_cast<Expression *>(parseExpression().get()));

  auto block_body =
    std::shared_ptr<Block>(dynamic_cast<Block *>(parseBlock(BLOCK_IN_WHILE).get()));

  return std::make_shared<WhileSTMT>(std::move(condition), std::move(block_body));
}

std::shared_ptr<Entity> Parser::parseForStatement() {
  std::unique_ptr<Token> token = peek();
  if (token == nullptr || token->kind != TOKEN_FOR) return nullptr;
  token = next(); // eat 'for'

  // assignment
  token = peek();
  if (token->kind != TOKEN_IDENTIFIER) return nullptr;
  auto varRef =
    std::shared_ptr<AssignmentSTMT>(dynamic_cast<AssignmentSTMT *>(parseAssignment().get()));

  // eat ','
  token = peek();
  if (token->kind != TOKEN_COMMA) return nullptr;
  token = next();

  // condition
  auto cond =
    std::shared_ptr<Expression>(dynamic_cast<Expression *>(parseExpression().get()));

  // eat ','
  token = peek();
  if (token->kind != TOKEN_COMMA) return nullptr;
  token = next();

  // step after i.e. "i++"
  auto post =
    std::shared_ptr<Expression>(dynamic_cast<Expression *>(parseExpression().get()));

  auto block_body =
    std::shared_ptr<Block>(dynamic_cast<Block *>(parseBlock(BLOCK_IN_FOR).get()));

  return std::make_shared<ForSTMT>(std::move(varRef), std::move(cond), std::move(post), std::move(block_body));

  // auto varAssign = std::make_shared<AssignmentSTMT>(std::move(varRef), )
}

std::shared_ptr<Entity> Parser::parseReturnStatement() {
  std::unique_ptr<Token> token = peek();
  if (token == nullptr || token->kind != TOKEN_RETURN) return nullptr;

  // eat 'return' lexeme
  token = next();

  std::shared_ptr<Expression> expr =
    std::shared_ptr<Expression>(dynamic_cast<Expression*>(parseExpression().get()));

  auto ret = std::make_shared<ReturnSTMT>(std::move(expr));
  return ret;
}

std::shared_ptr<Entity> Parser::parseExpression() {
  std::shared_ptr<Entity> node = parsePrimary();

  // what it could be
  std::shared_ptr<CompoundEXP> comp;

  auto node_to_exp = std::shared_ptr<Expression>(dynamic_cast<Expression*>(node.get()));
  comp->addExpression(std::move(node_to_exp));

  // std::shared_ptr<MethodCallEXP> method_call;
  // std::shared_ptr<FieldAccessEXP> field_access;

  std::unique_ptr<Token> token = peek();
  if (token == nullptr) return node;

  if (token->kind == TOKEN_LBRACKET) {
    // its a function call or a constructor call
    token = next();

    if (peek()->kind == TOKEN_LBRACKET) {
      token = next();

      auto func_call = std::make_shared<FuncCallEXP>();
      parseArguments(func_call);

      // check if its a constructor call
      if (this->globalTypeTable.types[moduleName].exists(func_call->func_name)) {
        auto class_name_expr = std::make_shared<ClassNameEXP>(func_call->func_name);
        auto constr_call = std::make_shared<ConstructorCallEXP>(
          std::move(class_name_expr),
          std::move(func_call->arguments));
        return constr_call;
      }

      // auto func_call_to_exp = std::shared_ptr<Expression>(func_call.get());
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
    std::shared_ptr<Entity> after_dot = parsePrimary();
    // @TODO throw error if after_dot is not an Identifier

    // then we decide if our identifier is a fieldaccess or a methodcall
    // - check for brackets -> MethodCall
    if (peek()->kind == TOKEN_LBRACKET) {
      token = next();

      // after_dot then is a method_name in a method call
      auto method_call = std::make_shared<MethodCallEXP>();
      parseArguments(method_call);

      auto method_call_to_exp = std::shared_ptr<Expression>(method_call.get());
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
      // std::shared_ptr<Entity> field = parsePrimary();
      // auto left = std::shared_ptr<Expression>(dynamic_cast<Expression*>(node.get()));
      auto field_access = std::make_shared<FieldRefEXP>(std::get<std::string>(next()->value));
      auto field_access_to_exp = std::shared_ptr<Expression>(field_access.get());
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
    return std::shared_ptr<Entity>(comp->parts[0].get());
  };
  if (parts_num == 1) {
    switch (comp->parts[1]->getKind()) {
      case E_Method_Call: {
      auto left = std::move(comp->parts[0]);
        std::shared_ptr<MethodCallEXP> method_call =
          std::shared_ptr<MethodCallEXP>(
              dynamic_cast<MethodCallEXP *>(comp->parts[1].get()));
        return method_call;
      }
      case E_Field_Reference: {
        std::shared_ptr<FieldAccessEXP> field_access;
        auto left = std::move(comp->parts[0]);
        auto field_ref = std::shared_ptr<FieldAccessEXP>(dynamic_cast<FieldAccessEXP *>(comp->parts[1].get()));
        field_access->left = std::move(left);
        field_access->field_name = field_ref->field_name;
        return field_access;
      }
      default: return comp;
    }
  }

  return comp;
}

std::shared_ptr<ParameterDecl> Parser::parseParameterDecl() {
  std::unique_ptr<Token> token = peek();

  // read first parameter
  token = peek();
  if (token->kind != TOKEN_IDENTIFIER) return nullptr;
  token = next();
  auto param_name = std::get<std::string>(token->value);

  // read ':'
  token = peek();
  if (token->kind != TOKEN_COLON) return nullptr;
  token = next();

  // read type
  token = peek();
  if (token->kind != TOKEN_IDENTIFIER) return nullptr;
  token = next();
  auto param_type = std::get<std::string>(token->value);

  // create paramdecl
  auto paramDecl = std::make_shared<ParameterDecl>(param_name, globalTypeTable.types[moduleName].getType(param_type));
  return paramDecl;
}


void Parser::parseParameters(const std::shared_ptr<FuncDecl>& funcDecl) {
  std::unique_ptr<Token> token = peek();
  if (token == nullptr || token->kind != TOKEN_LBRACKET) return;
  token = next();

  auto paramDecl = parseParameterDecl();

  funcDecl->args.push_back(std::move(paramDecl));

  while (token->kind == TOKEN_COMMA) {
    token = next();

    paramDecl = parseParameterDecl();
    funcDecl->args.push_back(std::move(paramDecl));

    token = peek();
    if (token == nullptr) return;
  }

  if (funcDecl->args[0] == nullptr) {
    funcDecl->isVoided = true;
    funcDecl->args.clear();
  }
}

void Parser::parseParameters(const std::shared_ptr<MethodDecl>& funcDecl) {
  std::unique_ptr<Token> token = peek();
  if (token == nullptr || token->kind != TOKEN_LBRACKET) return;
  token = next();

  auto paramDecl = parseParameterDecl();

  funcDecl->args.push_back(paramDecl);

  while (token->kind == TOKEN_COMMA) {
    token = next();

    paramDecl = parseParameterDecl();
    funcDecl->args.push_back(std::move(paramDecl));

    token = peek();
    if (token == nullptr) return;
  }

  if (funcDecl->args[0] == nullptr) {
    funcDecl->isVoided = true;
    funcDecl->args.clear();
  }
}

void Parser::parseParameters(const std::shared_ptr<ConstrDecl>& constrDecl) {
  std::unique_ptr<Token> token = peek();
  if (token == nullptr || token->kind != TOKEN_LBRACKET) return;
  token = next();

  auto paramDecl = parseParameterDecl();

  constrDecl->args.push_back(paramDecl);

  while (token->kind == TOKEN_COMMA) {
    token = next();

    paramDecl = parseParameterDecl();
    constrDecl->args.push_back(std::move(paramDecl));

    token = peek();
    if (token == nullptr) return;
  }

  token = peek();
  if (token == nullptr || token->kind != TOKEN_RBRACKET) return;
  token = next();

  if (constrDecl->args[0] == nullptr) {
    constrDecl->isDefault = true;
    constrDecl->args.clear();
  }
}

void Parser::parseArguments(const std::shared_ptr<MethodCallEXP> &method_name) {
  auto node = parseExpression();
  auto expr = std::shared_ptr<Expression>(dynamic_cast<Expression*>(node.get()));
  method_name->arguments.push_back(std::move(expr));

  std::unique_ptr<Token> token = peek();
  if (token == nullptr) return;

  // arg, arg, arg
  // ',' is a delimiter
  while (token->kind == TOKEN_COMMA) {
    token = next();

    std::shared_ptr<Entity> after_comma = parseExpression();
    auto uexpr = std::shared_ptr<Expression>(dynamic_cast<Expression*>(after_comma.get()));
    method_name->arguments.push_back(std::move(uexpr));

    token = peek();
    if (token == nullptr) return;
  }
}

void Parser::parseArguments(const std::shared_ptr<ConstructorCallEXP> &constr_name) {
  auto node = parseExpression();
  auto expr = std::shared_ptr<Expression>(dynamic_cast<Expression*>(node.get()));
  constr_name->arguments.push_back(std::move(expr));

  std::unique_ptr<Token> token = peek();
  if (token == nullptr) return;

  // arg, arg, arg
  // ',' is a delimiter
  while (token->kind == TOKEN_COMMA) {
    token = next();

    std::shared_ptr<Entity> after_comma = parseExpression();
    auto uexpr = std::shared_ptr<Expression>(dynamic_cast<Expression*>(after_comma.get()));
    constr_name->arguments.push_back(std::move(uexpr));

    token = peek();
    if (token == nullptr) return;
  }
}

void Parser::parseArguments(const std::shared_ptr<FuncCallEXP> &function_name) {
  auto node = parseExpression();
  auto expr = std::shared_ptr<Expression>(dynamic_cast<Expression*>(node.get()));
  function_name->arguments.push_back(std::move(expr));

  std::unique_ptr<Token> token = peek();
  if (token == nullptr) return;

  // arg, arg, arg
  // ',' is a delimiter
  while (token->kind == TOKEN_COMMA) {
    token = next();

    std::shared_ptr<Entity> after_comma = parseExpression();
    auto uexpr = std::shared_ptr<Expression>(dynamic_cast<Expression*>(after_comma.get()));
    function_name->arguments.push_back(std::move(uexpr));

    token = peek();
    if (token == nullptr) return;
  }
}

std::shared_ptr<Entity> Parser::parsePrimary() {
  std::unique_ptr<Token> token = peek();
  if (token == nullptr) return nullptr;

  token = next();
  switch (token->kind) {
  case TOKEN_INT_NUMBER: {
    return std::make_shared<IntLiteralEXP>(std::get<int>(token->value));
  }
  case TOKEN_REAL_NUMBER: {
    return std::make_shared<RealLiteralEXP>(std::get<double>(token->value));
  }
  case TOKEN_BOOL_TRUE: {
    return std::make_shared<BoolLiteralEXP>(true);
  }
  case TOKEN_BOOL_FALSE: {
    return std::make_shared<BoolLiteralEXP>(false);
  }
  case TOKEN_STRING: {
    return std::make_shared<StringLiteralEXP>(
        std::get<std::string>(token->value));
  }
  case TOKEN_SELFREF: {
    return std::make_shared<ThisEXP>();
  }
  case TOKEN_IDENTIFIER: {
    auto var = globalSymbolTable.lookup(moduleName, lastDeclaredScopeParent.top(), std::get<std::string>(token->value));
    if (!var)
      throw std::runtime_error("Undefined variable: " +
                               std::get<std::string>(token->value));
    switch (var->getKind()) {
      case E_Field_Decl: {
        return std::make_shared<FieldRefEXP>(std::get<std::string>(token->value));
      }
      case E_Variable_Decl: {
        return std::make_shared<VarRefEXP>(std::get<std::string>(token->value));
      }
      default: return nullptr;
    }

  }
  default:
    return nullptr;
  }
}