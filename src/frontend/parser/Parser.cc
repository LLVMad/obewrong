#include "frontend/parser/Parser.h"

#include "frontend/SymbolTable.h"
#include "frontend/TypeTable.h"
#include "frontend/parser/Expression.h"
#include "frontend/parser/Statement.h"

#define SYNCED_TOKEN(kind)                                                     \
  ((kind == TOKEN_CLASS) || (kind == TOKEN_FUNC) || (kind == TOKEN_METHOD) ||  \
   (kind == TOKEN_ENUM))

#define PARSE_ERR(path, msg)                                                   \
  ERR("%s:%d:%d %s\n", path, peek(0)->line + 1, peek(0)->column + 1, msg)

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
  case TOKEN_TYPE_BYTE:
    return true;
  default:
    return false;
  }
}

bool is_binary_operator(TokenKind kind) {
  switch (kind) {
  case TOKEN_PLUS:       // +
  case TOKEN_MINUS:      // -
  case TOKEN_STAR:       // *
  case TOKEN_SLASH:      // /
  case TOKEN_PERCENT:    // %
  case TOKEN_EQUAL:      // ==
  case TOKEN_NOT_EQUAL:  // !=
  case TOKEN_LESS:       // <
  case TOKEN_LESS_EQUAL: // <=
  case TOKEN_MORE_EQUAL: // >=
  case TOKEN_MORE:
  case TOKEN_BIT_AND:         // &
  case TOKEN_BIT_OR:          // |
  case TOKEN_BIT_XOR:         // ^
  case TOKEN_BIT_SHIFT_LEFT:  // <<
  case TOKEN_BIT_SHIFT_RIGHT: // >>
  case TOKEN_LOGIC_AND:       // &&
  case TOKEN_LOGIC_OR:        // ||
    return true;
  default:
    return false;
  }
}

bool is_unary_operator(TokenKind kind) {
  switch (kind) {
  case TOKEN_LOGIC_NOT:
  case TOKEN_BIT_INV:
  case TOKEN_BIT_XOR:
  case TOKEN_INCREMENT:
  case TOKEN_DECREMENT:
    return true;
  default:
    return false;
  }
}

std::unique_ptr<Token> Parser::next() {
  if (tokens[tokenPos + 1] != nullptr) {
    return std::make_unique<Token>(*tokens[++tokenPos]);
  }

  return nullptr;
}

std::unique_ptr<Token> Parser::peek() {
  if (tokens[tokenPos + 1] != nullptr) {
    return std::make_unique<Token>(*tokens[tokenPos + 1]);
  }

  return nullptr;
}

std::unique_ptr<Token> Parser::peek(size_t i) {
  if (tokens[tokenPos + i] != nullptr) {
    return std::make_unique<Token>(*tokens[tokenPos + i]);
  }

  return nullptr;
}

// @TODO
std::unique_ptr<Token> Parser::expect(TokenKind expectedToken,
                                      std::string msg) {
  std::unique_ptr<Token> token = peek();
  int pos = tokenPos;

  // if (token->kind != expectedToken) PARSE_ERR(msg);

  while (token->kind != expectedToken || !SYNCED_TOKEN(token->kind)) {
    // now we in panic mode
    // search for sync token or expected token
    if (tokens[pos + 1]->kind == TOKEN_EOF) {
      PARSE_ERR(sm.getLastFilePath().c_str(), "Your program is gibberish\n");
      exit(0);
    }
    token = std::make_unique<Token>(*tokens[++pos]);
  }

  return std::move(token);
}

std::shared_ptr<Entity> Parser::parseProgram() {
  // return parseExpression();
  std::unique_ptr<Token> token = peek();
  if (token == nullptr || token->kind != TOKEN_MODULE_DECL)
    return nullptr;
  token = next(); // eat 'module'

  // read module name
  token = peek();
  if (token == nullptr || token->kind != TOKEN_IDENTIFIER)
    return nullptr;
  token = next();

  auto root = std::make_shared<ModuleDecl>(std::get<std::string>(token->value));
  moduleName = std::get<std::string>(token->value);
  token = peek();
  while (token->kind == TOKEN_DOT) {
    token = next();
    moduleName += ("." + std::get<std::string>(next()->value));
    token = peek();
  }

  globalSymbolTable->enterScope(SCOPE_MODULE, moduleName);

  token = peek();
  while (token->kind == TOKEN_MODULE_IMP) {
    token = next();

    auto importedModuleName = std::get<std::string>(next()->value);

    token = peek();
    while (token->kind == TOKEN_DOT) {
      token = next();
      importedModuleName += ("." + std::get<std::string>(next()->value));
      token = peek();
    }

    root->addImport(importedModuleName);

    size_t last_dot = importedModuleName.find_last_of('.');
    if (last_dot == std::string::npos)
      last_dot = -1;
    auto modNameShort = importedModuleName.substr(last_dot + 1);
    if (!this->sm.isImportProvided(modNameShort)) {
      throw std::runtime_error("Module import provided does not exist : " +
                               importedModuleName);
    }

    globalSymbolTable->copySymbolFromModulesToCurrent(
        importedModuleName, // from
        moduleName          // to
    );

    token = peek();
  }
  // token = next();

  // initBuiltinTypes();
  // lastDeclaredScopeParent.emplace("Global");
  for (const auto &[name, type] : globalTypeTable->builtinTypes.types) {
    globalTypeTable->addType(moduleName, name, type);
  }

  // @NOTE entering GLOBAL scope is done at creation of SymbolTable
  // which is not good, probably should happen here

  token = peek();
  std::shared_ptr<Entity> child;
  while (token->kind != TOKEN_EOF) {
    // in global scope we can wait for class or function
    switch (token->kind) {
    case TOKEN_CLASS: {
      // token = next();
      child = parseClassDecl();
    } break;
    case TOKEN_FUNC: {
      child = parseFunctionDecl();
    } break;
    case TOKEN_ENUM: {
      child = parseEnumDecl();
    } break;
    case TOKEN_VAR_DECL: {
      child = parseVarDecl();
    } break;
    default:
      break;
    }

    root->children.push_back(child);
    token = peek();
  }

  // globalSymbolTable->moduleSymbolTables.clear();
  // globalTypeTable->types.clear();

  globalSymbolTable->exitScope();

  return root;
}

std::shared_ptr<FuncDecl> Parser::parseFunctionDecl() {
  std::unique_ptr<Token> token = peek();
  if (token == nullptr || token->kind != TOKEN_FUNC)
    return nullptr;
  token = next(); // eat 'func'

  // get func  name
  token = peek();
  if (token == nullptr || token->kind != TOKEN_IDENTIFIER)
    return nullptr;
  token = next();
  auto func_name = std::get<std::string>(token->value);

  globalSymbolTable->enterScope(SCOPE_METHOD, func_name);

  auto func = std::make_shared<FuncDecl>(func_name, func_name == "main");


  // get func parameters
  parseParameters(func);

  std::vector<std::shared_ptr<Type>> args_types;
  if (!func->isVoided) {
    for (const auto &param : func->args) {
      auto param_decl = std::dynamic_pointer_cast<ParameterDecl>(param);
      args_types.push_back(param_decl->type);
    }
  }

  // get return type
  token = peek();

  if (token->kind != TOKEN_COLON /*!isTypeName(token->kind)*/) {
    // no return type
    // build signature
    auto signature = std::make_shared<TypeFunc>(args_types);
    func->isVoid = true;
    func->signature = signature;

    // get body of method
    auto body_block =
        std::dynamic_pointer_cast<Block>(parseBlock(BLOCK_IN_FUNCTION));

    func->body = body_block;

    // quit scope
    // lastDeclaredScopeParent.pop();
    globalSymbolTable->exitScope();

    globalSymbolTable->getCurrentScope()->addSymbol(func->name, func);

    return func;
  };

  token = next();
  token = next();
  auto return_type =
      globalTypeTable->getType(moduleName, std::get<std::string>(token->value));

  // build signature
  auto signature = func->isVoided
                       ? std::make_shared<TypeFunc>(return_type)
                       : std::make_shared<TypeFunc>(return_type, args_types);

  // get body of method
  auto body_block =
      std::dynamic_pointer_cast<Block>(parseBlock(BLOCK_IN_METHOD));

  func->body = body_block;
  func->signature = signature;

  // quit scope
  // lastDeclaredScopeParent.pop();
  globalSymbolTable->exitScope();

  globalSymbolTable->getCurrentScope()->addSymbol(func->name, func);

  return func;
}

std::shared_ptr<MethodDecl> Parser::parseMethodDecl() {
  std::unique_ptr<Token> token = peek();
  if (token == nullptr || token->kind != TOKEN_METHOD)
    return nullptr;
  token = next(); // eat 'method'

  // get method name
  token = peek();
  bool is_static = token->kind == TOKEN_STATIC;
  if (is_static) {
    token = next();
    token = peek();
  }

  if (token == nullptr || token->kind != TOKEN_IDENTIFIER)
    return nullptr;
  token = next();
  auto method_name = std::get<std::string>(token->value);

  // new our scope is this method
  // lastDeclaredScopeParent.emplace(method_name);
  globalSymbolTable->enterScope(SCOPE_METHOD, method_name);

  auto method = std::make_shared<MethodDecl>(method_name);
  if (is_static)
    method->isStatic = true;

  // get method parameters
  parseParameters(method);

  std::vector<std::shared_ptr<Type>> args_types;
  if (!method->isVoided) {
    for (const auto &param : method->args) {
      auto param_decl = std::dynamic_pointer_cast<ParameterDecl>(param);
      args_types.push_back(param_decl->type);
    }
  }

  // get return type
  token = peek();

  if (token->kind != TOKEN_COLON /*!isTypeName(token->kind)*/) {
    // no return type
    // build signature
    auto signature = std::make_shared<TypeFunc>(args_types);
    method->isVoid = true;
    method->signature = signature;

    // get body of method
    auto body_block =
        std::dynamic_pointer_cast<Block>(parseBlock(BLOCK_IN_METHOD));

    method->body = body_block;

    // quit scope
    // lastDeclaredScopeParent.pop();
    globalSymbolTable->exitScope();

    globalSymbolTable->getCurrentScope()->addSymbol(method->name, method);

    return method;
  };

  token = next();
  token = next();
  auto return_type =
      globalTypeTable->getType(moduleName, std::get<std::string>(token->value));

  // build signature
  auto signature = method->isVoided
                       ? std::make_shared<TypeFunc>(return_type)
                       : std::make_shared<TypeFunc>(return_type, args_types);

  // get body of method
  auto body_block =
      std::dynamic_pointer_cast<Block>(parseBlock(BLOCK_IN_METHOD));

  method->body = body_block;
  method->signature = signature;

  // quit scope
  // lastDeclaredScopeParent.pop();
  globalSymbolTable->exitScope();

  globalSymbolTable->getCurrentScope()->addSymbol(method->name, method);

  return method;
}

std::shared_ptr<SwitchSTMT> Parser::parseSwitch() {
  std::unique_ptr<Token> token = peek();
  if (token == nullptr || token->kind != TOKEN_SWITCH)
    return nullptr;
  token = next();

  // token = next(); // eat '('
  auto condition = parseExpression();

  auto switch_st = std::make_shared<SwitchSTMT>(condition);

  token = peek();
  if (token == nullptr || token->kind != TOKEN_BBEGIN)
    return nullptr;
  token = next();
  while (token->kind != TOKEN_BEND) {

    // parse case stmt
    auto case_st = parseCase();
    // std::shared_ptr<CaseSTMT>(dynamic_cast<CaseSTMT *>(parseCase().get()));
    switch_st->addCase(case_st);

    token = peek();
    if (token == nullptr)
      return nullptr;
  }

  token = next(); // eat 'end'
  return switch_st;
}

std::shared_ptr<CaseSTMT> Parser::parseCase() {
  std::unique_ptr<Token> token = peek();

  // default case
  if (token->kind == TOKEN_DEFAULT) {
    token = next();
    auto body_block = parseBlock(BLOCK_IN_SWITCH);
    // std::dynamic_pointer_cast<Block>(parseB)
    // std::shared_ptr<Block>(
    //   dynamic_cast<Block *>(parseBlock(BLOCK_IN_SWITCH).get()));

    return std::make_shared<CaseSTMT>(body_block);
  }

  if (token == nullptr || token->kind != TOKEN_CASE)
    return nullptr;
  token = next(); // eat 'case'

  // case literal condition
  auto cond_lit = parsePrimary();
  // std::shared_ptr<Expression>(
  //   dynamic_cast<Expression *>(parsePrimary().get()));

  token = peek();
  if (token == nullptr || token->kind != TOKEN_THEN)
    return nullptr;
  // token = next();

  // case body
  auto body_block = parseBlock(BLOCK_IN_SWITCH);
  // std::dynamic_pointer_cast<Block>(parseB)
  // std::shared_ptr<Block>(
  //   dynamic_cast<Block *>(parseBlock(BLOCK_IN_SWITCH).get()));

  return std::make_shared<CaseSTMT>(cond_lit, body_block);
}

std::shared_ptr<IfSTMT> Parser::parseIfStatement() {
  std::unique_ptr<Token> token = peek();
  if (token == nullptr || token->kind != TOKEN_IF)
    return nullptr;
  token = next(); // eat 'if'

  auto condition = parseExpression();

  auto ifTrue = parseBlock(BLOCK_IN_IF);

  std::shared_ptr<Entity> ifFalse = nullptr;
  token = peek();
  if (token->kind == TOKEN_ELSE) {
    if (peek(2)->kind == TOKEN_IF) {
      token = next();
      ifFalse = parseIfStatement();
    } else {
      ifFalse = parseBlock(BLOCK_IN_IF);
    }

    return std::make_shared<IfSTMT>(condition, ifTrue, ifFalse);
  }

  return std::make_shared<IfSTMT>(condition, ifTrue);
}

std::shared_ptr<VarDecl> Parser::parseVarDecl() {
  std::unique_ptr<Token> token = peek();
  if (token == nullptr || token->kind != TOKEN_VAR_DECL)
    PARSE_ERR(sm.getLastFilePath().c_str(),
              "Expected `var` keyword for a var declaration\n");
  else
    token = next();

  // read var name
  std::string var_name;
  token = peek();
  if (token->kind != TOKEN_IDENTIFIER) {
    PARSE_ERR(sm.getLastFilePath().c_str(), "Expected var name\n");
    var_name = "unknown";
  } else {
    var_name = std::get<std::string>(next()->value);
    // token = next();
  }

  // check if type specifier is present
  if (peek()->kind != TOKEN_COLON)
    PARSE_ERR(sm.getLastFilePath().c_str(),
              "Expected type qualifier for a variable\n");
  else
    token = next();

  // short constr call
  // var a : Integer(2)
  std::shared_ptr<Type> var_type;
  if (peek(2)->kind == TOKEN_LBRACKET) {
    token = peek(); // get type name but dont eat it
    var_type = globalTypeTable->getType(moduleName,
                                        std::get<std::string>(token->value));

    auto var = std::make_shared<VarDecl>(var_name, var_type);

    auto initializer = parseExpression();
    // std::shared_ptr<Expression>(
    //   dynamic_cast<Expression *>(parseExpression().get()));
    var->initializer = initializer;

    if (peek()->kind == TOKEN_RBRACKET)
      token = next();

    globalSymbolTable->getCurrentScope()->addSymbol(var->name, var);

    return var;
  }

  // read type
  token = next();
  bool isPointer = false;
  if (token->kind == TOKEN_ACCESS) {
    // pointer to type
    isPointer = true;
    token = next();
  }

  std::string type_name = "byte";
  if (!isTypeName(token->kind) && token->kind != TOKEN_IDENTIFIER) {
    PARSE_ERR(sm.getLastFilePath().c_str(),
              "Expected type qualifier for a variable\n");
    var_type = globalTypeTable->types[moduleName].getType("byte");
  } else {
    if (isPointer) {
      auto toType = globalTypeTable->types[moduleName].getType(
        std::get<std::string>(token->value));
      var_type = std::make_shared<TypeAccess>(toType);
      type_name = var_name; // alias a type by the variable name
      globalTypeTable->addType(moduleName, type_name, var_type);
    } else {
      var_type = globalTypeTable->types[moduleName].getType(
          std::get<std::string>(token->value));
    }
  }

  // composite container type
  if (peek()->kind == TOKEN_LSBRACKET) {

    token = next(); // eat '['

    auto el_type_name = std::get<std::string>(next()->value);
    auto el_type = globalTypeTable->getType(moduleName, el_type_name);

    // @note <TypeList> is safe, because it has only el_type field
    auto var_type_array = std::make_shared<TypeList>(el_type);
    var_type_array->el_type = std::move(el_type);

    // size_t array_size = 0;

    // "," SIZE
    if (peek()->kind == TOKEN_COMMA) {
      auto var_type_const_array = std::make_shared<TypeArray>();
      var_type_const_array->el_type = std::move(el_type);
      token = next(); // eat ','

      token = peek();
      // @TODO can size be an expression?
      token = next();
      size_t array_size = std::get<int>(token->value);

      var_type_const_array->el_type = std::move(var_type_array->el_type);
      var_type_const_array->size = array_size;

      // Array type cant be shared, because arrays
      // have different length
      // taken from c++ primer
      // `"The number of elements in an array is part of the array's type."`
      // so we add a new array type when we encounter a decl of array
      // indexed by a var name ?
      globalTypeTable->addType(moduleName, var_name, var_type_const_array);

      var_type = var_type_const_array;
    } else {
      // Array type cant be shared, because arrays
      // have different length
      // taken from c++ primer
      // `"The number of elements in an array is part of the array's type."`
      // so we add a new array type when we encounter a decl of array
      // indexed by a var name ?
      globalTypeTable->addType(moduleName, var_name, var_type_array);

      var_type = var_type_array;
    }

    token = next(); // eat ']'
  }

  // type not found ?
  // @TODO return some cool error
  if (var_type == nullptr)
    return nullptr;

  auto var = std::make_shared<VarDecl>(var_name, var_type);

  // read initializer
  if (peek()->kind != TOKEN_ASSIGNMENT) {
    // this->globalSymbolTable->addToGlobalScope(
    //     moduleName, lastDeclaredScopeParent.top(), var);
    globalSymbolTable->getCurrentScope()->addSymbol(var->name, var);
    return var;
  }

  token = next();

  auto initializer = parseExpression();
  // std::shared_ptr<Expression>(
  //   dynamic_cast<Expression *>(parseExpression().get()));
  var->initializer = initializer;

  globalSymbolTable->getCurrentScope()->addSymbol(var->name, var);
  // this->globalSymbolTable->addToGlobalScope(moduleName,
  //                                           lastDeclaredScopeParent.top(),
  //                                           var);

  return var;
}

std::shared_ptr<AssignmentSTMT>
Parser::parseAssignment(std::shared_ptr<Expression> left) {
  // auto var_name = std::get<std::string>(token->value);
  auto var_ref = std::static_pointer_cast<VarRefEXP>(left);
  std::unique_ptr<Token> token = peek();

  // eat ':='
  if (peek()->kind != TOKEN_ASSIGNMENT)
    return nullptr;
  token = next();

  // read rvalue expression
  auto initializer = parseExpression();

  auto ass = std::make_shared<AssignmentSTMT>(var_ref, initializer);

  return ass;
}

std::shared_ptr<AssignmentSTMT> Parser::parseAssignment() {
  std::unique_ptr<Token> token = peek();
  if (token == nullptr ||
      (token->kind != TOKEN_IDENTIFIER && token->kind != TOKEN_SELFREF))
    return nullptr;

  // read lvalue var name
  // can be a single var
  // or a fieldref
  token = next();

  // check if its a fieldref
  // token = peek();
  if (peek()->kind == TOKEN_DOT) {
    token = next();

    token = next(); // get to field name
    // @TODO expect identif
    auto var_name = std::get<std::string>(token->value);
    auto var_ref = std::make_shared<FieldRefEXP>(var_name);

    // eat ':='
    if (peek()->kind != TOKEN_ASSIGNMENT)
      return nullptr;
    token = next();

    // read rvalue expression
    auto initializer = parseExpression();

    auto ass = std::make_shared<AssignmentSTMT>(var_ref, initializer);

    return ass;
  }

  auto var_name = std::get<std::string>(token->value);
  auto var_ref = std::make_shared<VarRefEXP>(var_name);

  // eat ':='
  if (peek()->kind != TOKEN_ASSIGNMENT)
    return nullptr;
  token = next();

  // read rvalue expression
  auto initializer = parseExpression();

  auto ass = std::make_shared<AssignmentSTMT>(var_ref, initializer);

  return ass;
}

std::shared_ptr<Block> Parser::parseBlock(BlockKind blockKind) {
  std::unique_ptr<Token> token = peek();
  if (token == nullptr ||
      (token->kind != TOKEN_BBEGIN && token->kind != TOKEN_THEN &&
       token->kind != TOKEN_LOOP && token->kind != TOKEN_ELSE))
    return nullptr;
  token = next();
  // if (token->kind == TOKEN_ELSE) token = peek(); // costil
  std::vector<std::shared_ptr<Entity>> block_body;

  while (token->kind !=
         TOKEN_BEND /*&& token->kind != TOKEN_ELSE -> questionable but kek*/) {
    std::shared_ptr<Entity> part;

    token = peek();
    switch (token->kind) {
    case TOKEN_VAR_DECL: {
      if (blockKind > 0)
        part = parseVarDecl();
      else // in class
        part = parseFieldDecl();
    } break;
    case TOKEN_IDENTIFIER:
    case TOKEN_PRINT: {
      // @FIX assign can be further than 2 or 4 tokens ahead
      if (peek(2)->kind == TOKEN_ASSIGNMENT ||
          peek(4)->kind == TOKEN_ASSIGNMENT) {
        // a := a.set(...)
        // a.value := 2
        part = parseAssignment();
      } else {
        // a.set(...)
        // printl(...)
        part = parseExpression();
        if (peek()->kind == TOKEN_ASSIGNMENT) {
          part = parseAssignment(std::static_pointer_cast<Expression>(part));
        }
      }
    } break;
    case TOKEN_IF: {
      part = parseIfStatement();
    } break;
    case TOKEN_WHILE: {
      part = parseWhileStatement();
    } break;
    case TOKEN_FOR: {
      part = parseForStatement();
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
      case BLOCK_IN_METHOD: {
        // @FIX assign can be further than 2 or 4 tokens ahead
        if (peek(2)->kind == TOKEN_ASSIGNMENT ||
            peek(4)->kind == TOKEN_ASSIGNMENT) {
          // a := a.set(...)
          // a.value := 2
          part = parseAssignment();
        } else {
          // a.set(...)
          // printl(...)
          part = parseExpression();
        }
      } break;
      default:
        break;
      }
    } break;
    case TOKEN_METHOD: {
      part = parseMethodDecl();
    } break;
    // @TODO
    default:
      return nullptr;
    }

    block_body.push_back(part);

    token = peek();
    if (token == nullptr)
      return nullptr;
  }

  // eat 'end'
  if (token->kind == TOKEN_BEND)
    token = next();

  return std::make_shared<Block>(block_body, blockKind);
}

std::shared_ptr<ConstrDecl> Parser::parseConstructorDecl() {
  std::unique_ptr<Token> token = peek();
  if (token == nullptr || token->kind != TOKEN_SELFREF)
    return nullptr;
  token = next(); // eat 'this'

  // take class name for "mangled" name of constructor function
  auto className = globalSymbolTable->getCurrentScope()->prevScope()->getName();

  // ????
  // lastDeclaredScopeParent.emplace("this");
  globalSymbolTable->enterScope(SCOPE_METHOD, className + "_Create");

  // read parameters
  // @TODO name constructor
  auto constr = std::make_shared<ConstrDecl>(className + "_Create");

  // read params
  parseParameters(constr);

  std::vector<std::shared_ptr<Type>> args_types;
  if (!constr->isDefault) {
    for (const auto &param : constr->args) {
      auto param_decl = std::dynamic_pointer_cast<ParameterDecl>(param);
      args_types.push_back(param_decl->type);

      // to make different constructors distinguishable
      // we add param names to it
      globalSymbolTable->getCurrentScope()->appendToName(param_decl->name);
    }
  }

  // get return type ???? no return type in constructorts lol
  // token = next();
  // auto return_type =
  //   globalTypeTable->getType(moduleName,
  //   std::get<std::string>(token->value));

  // build signature
  auto signature = constr->isDefault ? std::make_shared<TypeFunc>()
                                     : std::make_shared<TypeFunc>(args_types);

  // read constr body
  auto body_block = parseBlock(BLOCK_IN_METHOD);

  constr->body = body_block;
  constr->signature = signature;

  // lastDeclaredScopeParent.pop();
  globalSymbolTable->exitScope();

  return constr;
}

std::shared_ptr<ClassDecl> Parser::parseClassDecl() {
  std::unique_ptr<Token> token = peek();
  if (token == nullptr || token->kind != TOKEN_CLASS)
    return nullptr;
  token = next(); // eat 'class'

  // read classname
  if ((peek()->kind != TOKEN_IDENTIFIER) && !isTypeName(peek()->kind))
    return nullptr;
  token = next();
  auto class_name = std::get<std::string>(token->value);

  // check if its generic
  // if (peek()->kind == TOKEN_RSBRACKET) {
  //
  // }

  // now our scope is this class
  // lastDeclaredScopeParent.emplace(class_name);
  globalSymbolTable->enterScope(SCOPE_CLASS, class_name);

  // see if there is extends
  std::shared_ptr<Decl> base_class = nullptr;
  if (peek()->kind == TOKEN_EXTENDS) {
    token = next();

    token = next();
    auto current_scope = globalSymbolTable->getCurrentScope();
    auto module_scope = globalSymbolTable->getModuleScope(current_scope);
    base_class = module_scope->lookup(std::get<std::string>(token->value));

    // copy declarations of base class to child class

    // copy symbol table of base class to child class
    globalSymbolTable->copySymbolsAndChildren(module_scope, base_class->name,
                                              class_name);

    // auto base_class_scope =
    // globalSymbolTable->getModuleScope(current_scope)->

    // auto base_class_type = globalTypeTable->getType(
    //     moduleName, std::get<std::string>(token->value));
  }

  // read body of class
  auto body_block = parseBlock(BLOCK_IN_CLASS);

  std::vector<std::shared_ptr<Type>> fieldTypes;
  std::vector<std::shared_ptr<TypeFunc>> methodTypes;
  std::vector<std::shared_ptr<FieldDecl>> fields;
  std::vector<std::shared_ptr<MethodDecl>> methods;
  std::vector<std::shared_ptr<ConstrDecl>> constructors;

  for (auto &ent : body_block->parts) {
    switch (ent->getKind()) {
    case E_Field_Decl: {
      auto field = std::dynamic_pointer_cast<FieldDecl>(ent);
      fields.push_back(field);
      fieldTypes.push_back(field->type);
    } break;
    case E_Method_Decl: {
      auto method = std::dynamic_pointer_cast<MethodDecl>(ent);
      methods.push_back(method);
      methodTypes.push_back(method->signature);
    } break;
    case E_Constructor_Decl: {
      auto constr = std::dynamic_pointer_cast<ConstrDecl>(ent);
      constructors.push_back(constr);
      methodTypes.push_back(constr->signature);
    } break;
    default:
      break;
    }
  }

  globalSymbolTable->exitScope();
  // lastDeclaredScopeParent.pop();

  auto class_new_type =
      std::make_shared<TypeClass>(class_name, fieldTypes, methodTypes);
  // add new type (class) to LE SYMBOOOOOL TAABLEEEEESS si si papa
  globalTypeTable->addType(moduleName, class_name, class_new_type);

  auto class_stmt = std::make_shared<ClassDecl>(class_name, class_new_type,
                                                fields, methods, constructors);
  if (base_class) {
    auto baseClassDecl = std::static_pointer_cast<ClassDecl>(base_class);
    class_stmt->base_class = baseClassDecl;
    class_stmt->type->base_class = baseClassDecl->type;
  }
  // globalSymbolTable->addToGlobalScope(moduleName, "Global", class_stmt);
  globalSymbolTable->getCurrentScope()->addSymbol(class_name, class_stmt);

  return class_stmt;
}

std::shared_ptr<EnumDecl> Parser::parseEnumDecl() {
  std::unique_ptr<Token> token = peek();
  if (token == nullptr || token->kind != TOKEN_ENUM)
    return nullptr;
  token = next(); // eat 'enum'

  token = peek();
  if (token->kind != TOKEN_IDENTIFIER) {
    PARSE_ERR(sm.getLastFilePath().c_str(), "Expected a name for enum type");
    while (token->kind != TOKEN_BEND) {
      token = next();
    }
    token = next();
    return std::make_shared<EnumDecl>("unknown_enum");
  }
  auto enum_name = std::get<std::string>(token->value);

  globalSymbolTable->enterScope(SCOPE_ENUM, enum_name);

  auto enumDecl = std::make_shared<EnumDecl>(enum_name);
  token = peek();
  while (token->kind != TOKEN_BEND) {
    token = next();

    auto item = std::get<std::string>(token->value);
    enumDecl->addItem(item);

    token = peek();
    if (token->kind == TOKEN_COMMA)
      token = next();
  }
  token = next(); // eat 'end'

  globalSymbolTable->exitScope();
  globalSymbolTable->getCurrentScope()->addSymbol(enum_name, enumDecl);

  return enumDecl;
}

std::shared_ptr<FieldDecl> Parser::parseFieldDecl() {
  std::unique_ptr<Token> token = peek();
  if (token == nullptr || token->kind != TOKEN_VAR_DECL)
    PARSE_ERR(sm.getLastFilePath().c_str(),
              "Expected `var` keyword for field declaration\n");
  else
    token = next();

  // read lvalue var name
  token = peek();

  std::string var_name;
  if (token->kind != TOKEN_IDENTIFIER) {
    PARSE_ERR(sm.getLastFilePath().c_str(), "Expected field name\n");
    var_name = "unknown";
  } else {
    var_name = std::get<std::string>(next()->value);
    // token = next();
  }

  // eat ':'
  if (peek()->kind != TOKEN_COLON)
    PARSE_ERR(sm.getLastFilePath().c_str(),
              "Expected type qualifier for a field\n");
  else
    token = next();

  // read type
  token = next();
  bool isPointer = false;
  if (token->kind == TOKEN_ACCESS) {
    // pointer to type
    isPointer = true;
    token = next();
  }

  std::string type_name = "byte";
  std::shared_ptr<Type> var_type;
  if (!isTypeName(token->kind) && token->kind != TOKEN_IDENTIFIER) {
    PARSE_ERR(sm.getLastFilePath().c_str(),
              "Expected type qualifier for a field\n");
    var_type = globalTypeTable->types[moduleName].getType("byte");
  } else {
    // token = next();
    if (isPointer) {
      auto toType = globalTypeTable->types[moduleName].getType(
        std::get<std::string>(token->value));
      var_type = std::make_shared<TypeAccess>(toType);
      type_name = var_name; // alias a type by the variable name
      globalTypeTable->addType(moduleName, type_name, var_type);
    } else {
      var_type = globalTypeTable->types[moduleName].getType(
          std::get<std::string>(token->value));
    }
  }

  auto ass = std::make_shared<FieldDecl>(var_name, var_type);

  globalSymbolTable->getCurrentScope()->addSymbol(var_name, ass);
  // this->globalSymbolTable->addToGlobalScope(this->moduleName,
  //                                           this->lastDeclaredClass, ass);
  // this->symbolTable.addSymbol()

  return ass;
}

std::shared_ptr<WhileSTMT> Parser::parseWhileStatement() {
  std::unique_ptr<Token> token = peek();
  if (token == nullptr || token->kind != TOKEN_WHILE)
    return nullptr;
  token = next(); // eat 'while'

  token = peek();
  std::shared_ptr<Expression> condition;
  if (token->kind != TOKEN_IDENTIFIER && token->kind != TOKEN_INT_NUMBER &&
      token->kind != TOKEN_REAL_NUMBER && token->kind != TOKEN_STRING &&
      token->kind != TOKEN_BOOL_TRUE && token->kind != TOKEN_BOOL_FALSE &&
      token->kind != TOKEN_LSBRACKET && token->kind != TOKEN_LBRACKET) {
    PARSE_ERR(sm.getLastFilePath().c_str(),
              "Expected condition for a while statement\n");
    condition = std::make_shared<DummyExpression>("unknown_condition");
  } else
    condition = parseExpression();

  token = peek();
  std::shared_ptr<Block> block_body;
  if (token->kind != TOKEN_LOOP) {
    PARSE_ERR(sm.getLastFilePath().c_str(),
              "Expected 'loop' keyword at the start of a while body\n");
    block_body = nullptr;
  } else
    block_body = parseBlock(BLOCK_IN_WHILE);

  return std::make_shared<WhileSTMT>(condition, block_body);
}

std::shared_ptr<ForSTMT> Parser::parseForStatement() {
  std::unique_ptr<Token> token = peek();
  if (token == nullptr || token->kind != TOKEN_FOR)
    return nullptr;
  token = next(); // eat 'for'

  // assignment
  token = peek();
  if (token->kind != TOKEN_IDENTIFIER) {
    PARSE_ERR(sm.getLastFilePath().c_str(),
              "Expected an identifier in a for loop\n");
    while (token->kind != TOKEN_BEND) {
      token = next();
    }
    // token = next();
    return std::make_shared<ForSTMT>();
  }
  auto varRef = parseAssignment();

  // eat ','
  token = peek();
  if (token->kind != TOKEN_COMMA)
    PARSE_ERR(sm.getLastFilePath().c_str(),
              "Expected comma after an assignment in a for loop\n");
  else
    token = next();

  // condition
  auto cond = parseExpression();

  // eat ','
  token = peek();
  if (token->kind != TOKEN_COMMA)
    PARSE_ERR(sm.getLastFilePath().c_str(),
              "Expected comma after an expression in a for loop\n");
  else
    token = next();

  // step after i.e. "i++"
  auto post = parseAssignment();

  auto block_body = parseBlock(BLOCK_IN_FOR);

  return std::make_shared<ForSTMT>(varRef, cond, post, block_body);

  // auto varAssign = std::make_shared<AssignmentSTMT>(std::move(varRef), )
}

std::shared_ptr<ReturnSTMT> Parser::parseReturnStatement() {
  std::unique_ptr<Token> token = peek();
  if (token == nullptr || token->kind != TOKEN_RETURN)
    return nullptr; // well, i just undetstood that it cant happen

  // eat 'return' lexeme
  token = next();

  token = peek();
  // @TODO that is super bad
  if (token->kind != TOKEN_IDENTIFIER && token->kind != TOKEN_INT_NUMBER &&
      token->kind != TOKEN_REAL_NUMBER && token->kind != TOKEN_STRING &&
      token->kind != TOKEN_BOOL_TRUE && token->kind != TOKEN_BOOL_FALSE &&
      token->kind != TOKEN_LSBRACKET && token->kind != TOKEN_LBRACKET &&
      token->kind != TOKEN_SELFREF) {
    PARSE_ERR(sm.getLastFilePath().c_str(),
              "Expected expression after `return`\n");
    // token = next();
    return std::make_shared<ReturnSTMT>();
  }

  std::shared_ptr<Expression> expr = parseExpression();

  auto ret = std::make_shared<ReturnSTMT>(expr);
  return ret;
}

int getPrecedence(OperatorKind op) {
  switch (op) {
    // Postfix (highest precedence)
  case OP_INCREMENT:
  case OP_DECREMENT:
    return 1;

    // Unary (prefix)
  // case OP_PLUS:            // Unary +
  case OP_UNARY_MINUS: // Unary -
  case OP_LOGIC_NOT:   // !
  case OP_BIT_NOT:     // ~
    return 2;

    // Multiplicative
  case OP_MULTIPLY:
  case OP_DIVIDE:
  case OP_MODULUS:
    return 4;

    // Additive
  case OP_PLUS:  // Binary +
  case OP_MINUS: // Binary -
    return 3;

    // Shift
  case OP_BIT_LSHIFT:
  case OP_BIT_RSHIFT:
    return 5;

    // Relational
  case OP_LESS:
  case OP_LESS_EQUAL:
  case OP_MORE:
  case OP_MORE_EQUAL:
    return 6;

    // Equality
  case OP_EQUAL:
  case OP_NOT_EQUAL:
    return 7;

    // Bitwise AND
  case OP_BIT_AND:
    return 8;

    // Bitwise XOR
  case OP_BIT_XOR:
    return 9;

    // Bitwise OR
  case OP_BIT_OR:
    return 10;

    // Logical AND
  case OP_LOGIC_AND:
    return 11;

    // Logical OR
  case OP_LOGIC_OR:
    return 12;

    // Parentheses (special handling)
  case OP_LPAREN:
  case OP_RPAREN:
    return 0;

  default:
    return -1;
  }
}

OperatorKind Parser::tokenToOperator(TokenKind kind) {
  switch (kind) {
  case TOKEN_EQUAL:
    return OP_EQUAL;
  case TOKEN_NOT_EQUAL:
    return OP_NOT_EQUAL;
  case TOKEN_LESS:
    return OP_LESS;
  case TOKEN_LESS_EQUAL:
    return OP_LESS_EQUAL;
  case TOKEN_MORE_EQUAL:
    return OP_MORE_EQUAL;
  case TOKEN_BIT_AND:
    return OP_BIT_AND;
  case TOKEN_BIT_OR:
    return OP_BIT_OR;
  case TOKEN_BIT_XOR:
    return OP_BIT_XOR;
  case TOKEN_BIT_SHIFT_LEFT:
    return OP_BIT_LSHIFT;
  case TOKEN_BIT_SHIFT_RIGHT:
    return OP_BIT_RSHIFT;
  case TOKEN_PLUS:
    return OP_PLUS;
  case TOKEN_INCREMENT:
    return OP_INCREMENT;
  case TOKEN_DECREMENT:
    return OP_DECREMENT;
  case TOKEN_MINUS: {
    if (peek(0)->kind != TOKEN_IDENTIFIER)
      return OP_UNARY_MINUS;
    return OP_MINUS;
  }
  case TOKEN_STAR:
    return OP_MULTIPLY;
  case TOKEN_SLASH:
    return OP_DIVIDE;
  case TOKEN_PERCENT:
    return OP_MODULUS;
  case TOKEN_LOGIC_AND:
    return OP_LOGIC_AND;
  case TOKEN_LOGIC_NOT:
    return OP_LOGIC_NOT;
  case TOKEN_LOGIC_OR:
    return OP_LOGIC_OR;
  case TOKEN_LBRACKET:
    return OP_LPAREN;
  case TOKEN_RBRACKET:
    return OP_RPAREN;
  case TOKEN_MORE:
    return OP_MORE;

  default:
    throw std::runtime_error("Not an operator");
  }
}

std::shared_ptr<Expression>
Parser::parseBinaryOp(std::shared_ptr<Expression> firstOperand) {
  std::stack<OperatorKind> op_stack;
  std::stack<std::shared_ptr<Expression>> expr_stack;
  // std::stack<bool> paren_stack;

  if (firstOperand)
    expr_stack.push(firstOperand);

  while (true) {
    auto token = peek();
    if (!token)
      break;

    if (is_unary_operator(token->kind)) {
      OperatorKind op = tokenToOperator(token->kind);
      next();
      auto operand = parsePrimary();
      expr_stack.push(std::make_shared<UnaryOpEXP>(op, operand));
      continue;
    }

    if (token->kind == TOKEN_LBRACKET) {
      next(); // eat '('

      // Parse nested expression recursively
      auto nested_expr = parseExpression();

      // Look ahead for closing bracket
      if (!peek() || peek()->kind != TOKEN_RBRACKET) {
        PARSE_ERR(sm.getLastFilePath().c_str(),
                  "Expected ')' in a binary expression");
        // throw std::runtime_error("Expected closing bracket");
      } else
        next(); // eat ')'

      expr_stack.push(nested_expr);
      continue;
    }

    if (!is_binary_operator(token->kind))
      break;

    OperatorKind curr_op = tokenToOperator(token->kind);
    next(); // eat op

    while (!op_stack.empty() &&
           getPrecedence(op_stack.top()) >= getPrecedence(curr_op)) {
      OperatorKind op = op_stack.top();
      op_stack.pop();

      auto right = expr_stack.top();
      expr_stack.pop();
      auto left = expr_stack.top();
      expr_stack.pop();

      expr_stack.push(std::make_shared<BinaryOpEXP>(op, left, right));
    }

    op_stack.push(curr_op);

    auto right = parsePrimary();
    expr_stack.push(right);
  }

  while (!op_stack.empty()) {
    OperatorKind op = op_stack.top();
    op_stack.pop();

    auto right = expr_stack.top();
    expr_stack.pop();
    auto left = expr_stack.top();
    expr_stack.pop();

    expr_stack.push(std::make_shared<BinaryOpEXP>(op, left, right));
  }

  return expr_stack.top();
}

std::shared_ptr<Expression> Parser::parseExpression() {
  // binary operation expression
  if (is_unary_operator(peek()->kind) || peek()->kind == TOKEN_LBRACKET) {
    return parseBinaryOp(nullptr);
  }

  std::shared_ptr<Expression> node = parsePrimary();

  // if (peek()->kind == TOKEN_ASSIGNMENT) {
  //   return parseAssignment(node);
  // }

  // if (peek()->kind == TOKEN_COLON) {
  //   PARSE_ERR(sm.getLastFilePath().c_str(), "You probably forgot to add 'var'
  //   keyword for a variable declaration\n"); std::unique_ptr<Token> token =
  //   peek(); while (token->kind != TOKEN_ASSIGNMENT) token = next(); next();
  //   return parseExpression();
  // }

  // what it could be
  auto comp = std::make_shared<CompoundEXP>();

  // auto node_to_exp = std::dynamic_pointer_cast<Expression>(node);
  comp->addExpression(node);

  // std::shared_ptr<MethodCallEXP> method_call;
  // std::shared_ptr<FieldAccessEXP> field_access;

  std::unique_ptr<Token> token = peek();
  if (token == nullptr)
    return node;

  // assignment
  if (token->kind == TOKEN_ASSIGNMENT) {
    // @TODO
  }

  // enum ref or static method call
  if (token->kind == TOKEN_DOUBLE_COLON) {
    token = next();

    token = peek();
    if (token->kind != TOKEN_IDENTIFIER) {
      PARSE_ERR(sm.getLastFilePath().c_str(),
                "Expected an identifier after '::'\n");
      return std::make_shared<Expression>(E_Dummy);
    }
    token = next();

    switch (node->getKind()) {
    case E_Enum_Decl:
    case E_Enum_Reference: {
      auto node_as_var =
          std::static_pointer_cast<VarRefEXP>(comp->parts.back());
      auto enumName = node_as_var->var_name;

      auto itemName = std::get<std::string>(token->value);

      comp->addExpression(std::make_shared<EnumRefEXP>(enumName, itemName));
    } break;
    case E_Class_Decl:
    case E_Class_Name: {
      auto node_as_class_name_expr =
          std::static_pointer_cast<ClassNameEXP>(comp->parts.back());
      auto className = node_as_class_name_expr->name;
      auto methodName = std::get<std::string>(token->value);

      auto staticMethodCall = std::make_shared<MethodCallEXP>(methodName);
      staticMethodCall->left = node_as_class_name_expr;
      parseArguments(staticMethodCall);
      comp->addExpression(staticMethodCall);
    } break;
    default:
      break;
    }
  }

  if (token->kind == TOKEN_LBRACKET) {
    // its a function call or a constructor call
    token = next();

    // if (peek()->kind == TOKEN_LBRACKET) {
    // token = next();
    // @FIX NOT A VARREF JUST NEED A NAME
    auto func_name = std::static_pointer_cast<VarRefEXP>(node)->var_name;
    auto func_call = std::make_shared<FuncCallEXP>();
    parseArguments(func_call);
    func_call->func_name = func_name;

    // check if its a constructor call
    if (this->globalTypeTable->getType(moduleName, func_name) != nullptr) {
      auto class_name_expr =
          std::make_shared<ClassNameEXP>(func_call->func_name);

      if (func_call->arguments.empty()) {
        auto constr_call =
            std::make_shared<ConstructorCallEXP>(class_name_expr);
        return constr_call;
      }
      auto constr_call = std::make_shared<ConstructorCallEXP>(
          class_name_expr, func_call->arguments);
      return constr_call;
    }

    // auto func_call_to_exp = std::shared_ptr<Expression>(func_call.get());
    // comp->addExpression(func_call_to_exp);

    token = peek();
    if (token == nullptr)
      return nullptr;

    if (token->kind != TOKEN_RBRACKET) {
      PARSE_ERR(sm.getLastFilePath().c_str(),
                "Expected `)` after functional call\n");
    } else {
      // eat closing bracket after func read
      token = next();
    }
    return func_call;
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

    token = next(); // eat '.'

    // // should be an identifier

    std::shared_ptr<Expression> after_dot;
    if (comp->parts.back()->getKind() == E_Var_Reference) {
      auto leftAsVar =
          std::static_pointer_cast<VarRefEXP>(comp->parts.back())->var_name;
      auto calleeType =
          std::static_pointer_cast<ClassDecl>(
              globalSymbolTable->getCurrentScope()->lookup(leftAsVar))
              ->type->name;
      if (calleeType.empty()) {
        after_dot = parsePrimary();
      } else {
        after_dot = parsePrimary(calleeType); /* parsePrimary() */
        ;
      }
    } else {
      after_dot = parsePrimary();
    }

    // @TODO throw error if after_dot is not an Identifier

    // then we decide if our identifier is a fieldaccess or a methodcall
    // - check for brackets -> MethodCall
    if (peek()->kind == TOKEN_LBRACKET) {
      token = next();

      // after_dot then is a method_name in a method call
      auto method_call = std::static_pointer_cast<MethodCallEXP>(after_dot);
      // auto method_call = std::make_shared<MethodCallEXP>();
      // method_call->method_name =
      //   // ???? костыль жёсткий слишком это вообще не рев фар это имя метода
      //   std::static_pointer_cast<VarRefEXP>(after_dot)->var_name;
      parseArguments(method_call);

      auto method_call_to_exp =
          std::dynamic_pointer_cast<Expression>(method_call);
      comp->addExpression(method_call_to_exp);

      token = peek();
      if (token == nullptr)
        return method_call;
      if (token->kind != TOKEN_RBRACKET) {
        PARSE_ERR(sm.getLastFilePath().c_str(),
                  "Expected `)` after method call\n");
      } else {
        // eat closing bracket after func read
        token = next();
      }
    }
    // - if not its a field access, than cannot be chained mind you
    //   so we can probably just return after read ?
    else {
      // token = next();

      // read identifier of a field
      // std::shared_ptr<Entity> field = parsePrimary();
      // auto left =
      // std::shared_ptr<Expression>(dynamic_cast<Expression*>(node.get()));
      auto field_name =
          std::static_pointer_cast<VarRefEXP>(after_dot)->var_name;
      auto obj_ref = std::static_pointer_cast<VarRefEXP>(node);
      auto field_access = std::make_shared<FieldRefEXP>(field_name, obj_ref);
      // std::static_pointer_cast<VarRefEXP>(after_dot);
      // auto field_access_to_exp =
      //     std::dynamic_pointer_cast<Expression>(field_access);
      // std::shared_ptr<Expression>(field_access.get());
      comp->addExpression(field_access);
    }

    token = peek();
    if (token == nullptr)
      return node;
  }

  // binary operation expression
  if (is_binary_operator(token->kind)) {
    return parseBinaryOp(node);
  }

  // number of expr delimitered by a dot
  // - 0 -> cant be, a.
  // - 1 -> methodcall or fieldaccess, a.Plus(2), Pair.first
  // - >1 -> compound
  size_t parts_num = comp->parts.size();
  if (parts_num == 1) {
    return comp->parts[0];
  };
  if (parts_num == 2) {
    switch (comp->parts[1]->getKind()) {
    case E_Method_Call: {
      auto left = std::move(comp->parts[0]);
      std::shared_ptr<MethodCallEXP> method_call =
          std::dynamic_pointer_cast<MethodCallEXP>(comp->parts[1]);
      method_call->left = left;
      // std::shared_ptr<MethodCallEXP>(
      //     dynamic_cast<MethodCallEXP *>(comp->parts[1].get()));
      return method_call;
    }
    case E_Field_Reference: {
      // auto field_access = ;
      // @TEST IT
      auto left = std::static_pointer_cast<VarRefEXP>(comp->parts[0]);
      auto field_ref = std::static_pointer_cast<VarRefEXP>(comp->parts[1]);
      // std::shared_ptr<FieldAccessEXP>(dynamic_cast<FieldAccessEXP
      // *>(comp->parts[1].get()));
      // field_access->left = std::move(left);
      // field_access->field_name = field_ref->field_name;

      return std::make_shared<FieldRefEXP>(field_ref->var_name, left);
    }
    default:
      return comp;
    }
  }

  return comp;
}

std::shared_ptr<ParameterDecl> Parser::parseParameterDecl() {
  std::unique_ptr<Token> token = peek();

  // read first parameter
  token = peek();

  if (token->kind == TOKEN_VAR_DECL) {
    PARSE_ERR(sm.getLastFilePath().c_str(),
              "Unnecessary 'var' keyword in parameter declaration\n");
    token = next();
    token = peek();
  }

  if (token->kind != TOKEN_IDENTIFIER) {
    PARSE_ERR(sm.getLastFilePath().c_str(),
              "Expected an identifier in a parameter declaration\n");
    return nullptr; // @TODO
  }
  token = next();
  auto param_name = std::get<std::string>(token->value);

  // read ':'
  token = peek();
  if (token->kind != TOKEN_COLON) {
    PARSE_ERR(sm.getLastFilePath().c_str(),
              "Expected a type qualifier for a variable, ignoring other "
              "parameters\n");
    auto paramDummy = std::make_shared<ParameterDecl>(
        param_name, globalTypeTable->types[moduleName].getType("byte"));

    globalSymbolTable->getCurrentScope()->addSymbol(param_name, paramDummy);

    while (token->kind != TOKEN_RBRACKET && token->kind != TOKEN_COMMA) {
      token = next();
    }

    return paramDummy;
  }
  token = next();

  // read type
  // token = peek();
  // if (!isTypeName(token->kind))
  //   return nullptr;
  // token = next();
  token = next();
  bool isPointer = false;
  if (token->kind == TOKEN_ACCESS) {
    // pointer to type
    isPointer = true;
    token = next();
  }

  std::string type_name = "byte";
  std::shared_ptr<Type> param_type;
  if (!isTypeName(token->kind) && token->kind != TOKEN_IDENTIFIER) {
    PARSE_ERR(sm.getLastFilePath().c_str(),
              "Expected type qualifier for a field\n");
    param_type = globalTypeTable->types[moduleName].getType("byte");
  } else {
    // token = next();
    if (isPointer) {
      auto toType = globalTypeTable->types[moduleName].getType(
        std::get<std::string>(token->value));
      param_type = std::make_shared<TypeAccess>(toType);
      type_name = param_name; // alias a type by the variable name
      globalTypeTable->addType(moduleName, type_name, param_type);
    } else {
      param_type = globalTypeTable->types[moduleName].getType(
          std::get<std::string>(token->value));
    }
  }
  // auto param_type = std::get<std::string>(token->value);

  // create paramdecl
  auto paramDecl = std::make_shared<ParameterDecl>(
      param_name, param_type);

  globalSymbolTable->getCurrentScope()->addSymbol(param_name, paramDecl);

  return paramDecl;
}

void Parser::parseParameters(const std::shared_ptr<FuncDecl> &funcDecl) {
  std::unique_ptr<Token> token = peek();
  if (token == nullptr || token->kind != TOKEN_LBRACKET) {
    PARSE_ERR(sm.getLastFilePath().c_str(),
              "Expected '(' following a function declaration\n");

    while (token->kind != TOKEN_BBEGIN) {
      token = next();
    }

    return;
  }
  token = next();

  if (peek()->kind == TOKEN_RBRACKET) {
    funcDecl->isVoided = true;
    token = next();
    return;
  }

  auto paramDecl = parseParameterDecl();

  funcDecl->args.push_back(paramDecl);

  token = peek();

  if (token->kind == TOKEN_IDENTIFIER) {
    PARSE_ERR(sm.getLastFilePath().c_str(),
              "Expected a comma between parameters declarations\n");
    tokenPos--;
    token =
        std::make_unique<Token>(TOKEN_COMMA, peek(0)->line, peek(0)->column);
  }

  while (token->kind == TOKEN_COMMA) {
    token = next();

    paramDecl = parseParameterDecl();
    funcDecl->args.push_back(paramDecl);

    token = peek();
    if (token == nullptr)
      return;
  }

  token = peek();
  if (token == nullptr || token->kind != TOKEN_RBRACKET)
    PARSE_ERR(sm.getLastFilePath().c_str(),
              "Expected ')' following a function declaration\n");
  else
    token = next();

  if (funcDecl->args[0] == nullptr) {
    funcDecl->isVoided = true;
    funcDecl->args.clear();
  }
}

void Parser::parseParameters(const std::shared_ptr<MethodDecl> &funcDecl) {
  std::unique_ptr<Token> token = peek();
  if (token == nullptr || token->kind != TOKEN_LBRACKET) {
    PARSE_ERR(sm.getLastFilePath().c_str(),
              "Expected '(' following a method declaration\n");

    while (token->kind != TOKEN_BBEGIN) {
      token = next();
    }

    return;
  }
  token = next();

  if (peek()->kind == TOKEN_RBRACKET) {
    funcDecl->isVoided = true;
    token = next();
    return;
  }

  auto paramDecl = parseParameterDecl();

  funcDecl->args.push_back(paramDecl);

  token = peek();

  if (token->kind == TOKEN_IDENTIFIER) {
    PARSE_ERR(sm.getLastFilePath().c_str(),
              "Expected a comma between parameters declarations\n");
    tokenPos--;
    token =
        std::make_unique<Token>(TOKEN_COMMA, peek(0)->line, peek(0)->column);
  }

  while (token->kind == TOKEN_COMMA) {
    token = next();

    paramDecl = parseParameterDecl();
    funcDecl->args.push_back(paramDecl);

    token = peek();
    if (token == nullptr)
      return;
  }

  token = peek();
  if (token == nullptr || token->kind != TOKEN_RBRACKET)
    PARSE_ERR(sm.getLastFilePath().c_str(),
              "Expected ')' following a method declaration\n");
  else
    token = next();

  if (funcDecl->args[0] == nullptr) {
    funcDecl->isVoided = true;
    funcDecl->args.clear();
  }
}

void Parser::parseParameters(const std::shared_ptr<ConstrDecl> &constrDecl) {
  std::unique_ptr<Token> token = peek();
  if (token == nullptr || token->kind != TOKEN_LBRACKET) {
    PARSE_ERR(sm.getLastFilePath().c_str(),
              "Expected '(' following a constructor declaration\n");

    while (token->kind != TOKEN_BBEGIN) {
      token = next();
    }

    return;
  }
  token = next();

  if (peek()->kind == TOKEN_RBRACKET) {
    constrDecl->isDefault = true;
    token = next();
    return;
  }

  auto paramDecl = parseParameterDecl();

  constrDecl->args.push_back(paramDecl);

  token = peek();

  if (token->kind == TOKEN_IDENTIFIER) {
    PARSE_ERR(sm.getLastFilePath().c_str(),
              "Expected a comma between parameters declarations\n");
    tokenPos--;
    token =
        std::make_unique<Token>(TOKEN_COMMA, peek(0)->line, peek(0)->column);
  }

  while (token->kind == TOKEN_COMMA) {
    token = next();

    paramDecl = parseParameterDecl();
    constrDecl->args.push_back(paramDecl);

    token = peek();
    if (token == nullptr)
      return;
  }

  token = peek();
  if (token == nullptr || token->kind != TOKEN_RBRACKET)
    PARSE_ERR(sm.getLastFilePath().c_str(),
              "Expected ')' following a constructor declaration\n");
  else
    token = next();

  if (constrDecl->args[0] == nullptr) {
    constrDecl->isDefault = true;
    constrDecl->args.clear();
  }
}

void Parser::parseArguments(const std::shared_ptr<MethodCallEXP> &method_name) {
  if (peek()->kind == TOKEN_RBRACKET) {
    // method_name->isVoided = true;
    next();
    return;
  }

  auto node = parseExpression();
  // auto expr = std::dynamic_pointer_cast<Expression>(node);
  // std::shared_ptr<Expression>(dynamic_cast<Expression*>(node.get()));
  method_name->arguments.push_back(node);

  std::unique_ptr<Token> token = peek();
  if (token == nullptr)
    return;

  // arg, arg, arg
  // ',' is a delimiter
  while (token->kind == TOKEN_COMMA) {
    token = next();

    std::shared_ptr<Expression> after_comma = parseExpression();
    // auto uexpr = std::dynamic_pointer_cast<Expression>(after_comma);
    // std::shared_ptr<Expression>(dynamic_cast<Expression*>(after_comma.get()));
    method_name->arguments.push_back(after_comma);

    token = peek();
    if (token == nullptr)
      return;
  }
}

void Parser::parseArguments(
    const std::shared_ptr<ConstructorCallEXP> &constr_name) {
  if (peek()->kind == TOKEN_RBRACKET) {
    constr_name->isDefault = true;
    next();
    return;
  }

  auto node = parseExpression();
  auto expr = std::dynamic_pointer_cast<Expression>(node);
  // std::shared_ptr<Expression>(dynamic_cast<Expression*>(node.get()));
  constr_name->arguments.push_back(expr);

  std::unique_ptr<Token> token = peek();
  if (token == nullptr)
    return;

  // arg, arg, arg
  // ',' is a delimiter
  while (token->kind == TOKEN_COMMA) {
    token = next();

    std::shared_ptr<Expression> after_comma = parseExpression();
    // auto uexpr = std::dynamic_pointer_cast<Expression>(after_comma);
    // std::shared_ptr<Expression>(dynamic_cast<Expression*>(after_comma.get()));
    constr_name->arguments.push_back(after_comma);

    token = peek();
    if (token == nullptr)
      return;
  }
}

void Parser::parseArguments(const std::shared_ptr<FuncCallEXP> &function_name) {
  if (peek()->kind == TOKEN_RBRACKET) {
    function_name->isVoided = true;
    // next();
    return;
  }

  auto node = parseExpression();
  // auto expr = std::dynamic_pointer_cast<Expression>(node);
  // std::shared_ptr<Expression>(dynamic_cast<Expression*>(node.get()));
  function_name->arguments.push_back(node);

  std::unique_ptr<Token> token = peek();
  // if (token == nullptr)
  //   return;

  // arg, arg, arg
  // ',' is a delimiter
  while (token->kind == TOKEN_COMMA) {
    token = next();

    std::shared_ptr<Expression> after_comma = parseExpression();
    // auto uexpr = std::dynamic_pointer_cast<Expression>(after_comma);
    // std::shared_ptr<Expression>(dynamic_cast<Expression*>(after_comma.get()));
    function_name->arguments.push_back(after_comma);

    token = peek();
    if (token == nullptr)
      return;
  }
}

std::shared_ptr<Expression> Parser::parsePrimary() {
  std::unique_ptr<Token> token = peek();
  if (token == nullptr)
    return nullptr;

  token = next();
  switch (token->kind) {
  case TOKEN_INT_NUMBER: {
    return std::make_shared<IntLiteralEXP>(std::get<int>(token->value), 16);
  }
  case TOKEN_INT8_NUMBER: {
    return std::make_shared<IntLiteralEXP>(std::get<int>(token->value), 8);
  }
    case TOKEN_INT16_NUMBER: {
    return std::make_shared<IntLiteralEXP>(std::get<int>(token->value), 16);
  }
    case TOKEN_INT32_NUMBER: {
    return std::make_shared<IntLiteralEXP>(std::get<int>(token->value), 32);
  }
    case TOKEN_INT64_NUMBER: {
    return std::make_shared<IntLiteralEXP>(std::get<int>(token->value), 64);
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
  case TOKEN_LBRACKET: {
    // next(); // eat '('
    auto expr = parseExpression();
    next(); // eat ')'
    return expr;
  }
  // array literal
  // "[" Expression "," Expression "," ... "]"
  case TOKEN_LSBRACKET: {
    // token = next(); // eat '['

    std::vector<std::shared_ptr<Expression>> elements;
    elements.push_back(parseExpression());

    while (peek()->kind == TOKEN_COMMA) {
      token = next(); // eat ','
      elements.push_back(parseExpression());
    }

    token = next(); // eat ']'

    return std::make_shared<ArrayLiteralExpr>(elements);
  }
  case TOKEN_IDENTIFIER:
  case TOKEN_PRINT: {
    auto var = globalSymbolTable->getCurrentScope()->lookup(
        std::get<std::string>(token->value));
    if (!var) {
      PARSE_ERR(sm.getLastFilePath().c_str(), "Variable not found in scope\n");
      return std::make_shared<DummyExpression>(std::get<std::string>(token->value));
      // throw std::runtime_error("Undefined variable: " +
      //                          std::get<std::string>(token->value));
    }
    switch (var->getKind()) {
    // case E_Field_Decl: {
    //   // we return it as a var to construct
    //   // a legit FieldRefEXP on parseExpression
    //   // return
    //   // std::make_shared<FieldRefEXP>(std::get<std::string>(token->value),
    //   ); return
    //   std::make_shared<VarRefEXP>(std::get<std::string>(token->value));
    // }
    case E_Variable_Decl:
    case E_Parameter_Decl:
    case E_Field_Decl: {
      if (peek()->kind == TOKEN_LSBRACKET) {
        auto arrayRef =
            std::make_shared<VarRefEXP>(std::get<std::string>(token->value));
        // index op
        token = next();                     // eat '['
        auto indexedBy = parseExpression(); // @TODO ummm... sus
        token = next();                     // eat ']'

        return std::make_shared<ElementRefEXP>(indexedBy, arrayRef);
      }
      return std::make_shared<VarRefEXP>(std::get<std::string>(token->value));
    }
    case E_Class_Decl: {
      return std::make_shared<ClassNameEXP>(
          std::get<std::string>(token->value));
    }
    case E_Function_Decl: {
      return std::make_shared<FuncCallEXP>(std::get<std::string>(token->value));
    }
    case E_Method_Decl: {
      return std::make_shared<MethodCallEXP>(
          std::get<std::string>(token->value));
    }
    case E_Enum_Decl: {
      // @TODO
      return std::make_shared<EnumRefEXP>(std::get<std::string>(token->value));
    }
    default:
      return nullptr;
    }
  }
  default:
    if (isTypeName(token->kind)) {
      return std::make_shared<ClassNameEXP>(
          std::get<std::string>(token->value));
    }
  }
}

std::shared_ptr<Expression>
Parser::parsePrimary(const std::string &classNameToSearchIn) {
  std::unique_ptr<Token> token = peek();
  if (token == nullptr)
    return nullptr;

  token = next();
  switch (token->kind) {
  case TOKEN_INT_NUMBER: {
    return std::make_shared<IntLiteralEXP>(std::get<int>(token->value), 16);
  }
  case TOKEN_INT8_NUMBER: {
    return std::make_shared<IntLiteralEXP>(std::get<int>(token->value), 8);
  }
  case TOKEN_INT16_NUMBER: {
    return std::make_shared<IntLiteralEXP>(std::get<int>(token->value), 16);
  }
  case TOKEN_INT32_NUMBER: {
    return std::make_shared<IntLiteralEXP>(std::get<int>(token->value), 32);
  }
  case TOKEN_INT64_NUMBER: {
    return std::make_shared<IntLiteralEXP>(std::get<int>(token->value), 64);
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
  case TOKEN_LBRACKET: {
    next(); // eat '('
    auto expr = parseExpression();
    next(); // eat ')'
    return expr;
  }
  case TOKEN_IDENTIFIER:
  case TOKEN_PRINT: {
    // getGlobalScope()
    auto currScope = globalSymbolTable->getCurrentScope();
    auto var_name = std::get<std::string>(token->value);
    auto var = globalSymbolTable->getModuleScope(currScope)->lookupInClass(
        var_name, classNameToSearchIn);
    if (!var) {
      PARSE_ERR(sm.getLastFilePath().c_str(), "Variable not found in scope\n");
      return std::make_shared<DummyExpression>(var_name);
    }
    switch (var->getKind()) {
    case E_Field_Decl: {
      // we return it as a var to construct
      // a legit FieldRefEXP on parseExpression
      // return
      // std::make_shared<FieldRefEXP>(std::get<std::string>(token->value), );
      return std::make_shared<VarRefEXP>(std::get<std::string>(token->value));
    }
    case E_Variable_Decl:
    case E_Parameter_Decl: {
      return std::make_shared<VarRefEXP>(std::get<std::string>(token->value));
    }
    case E_Class_Decl: {
      return std::make_shared<ClassNameEXP>(
          std::get<std::string>(token->value));
    }
    case E_Function_Decl: {
      return std::make_shared<FuncCallEXP>(std::get<std::string>(token->value));
    }
    case E_Method_Decl: {
      return std::make_shared<MethodCallEXP>(
          std::get<std::string>(token->value));
    }
    default:
      return nullptr;
    }
  }
  default:
    return nullptr;
  }
}
