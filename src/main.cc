#include <iostream>
#include <stdio.h>
#include <vector>

#include "backend/CodegenVisitor.h"
#include "frontend/SourceManager.h"
#include "frontend/lexer/Lexer.h"
#include "frontend/parser/Parser.h"
#include "frontend/semantic/PrinterAst.h"
#include "frontend/semantic/SemanticAnalyzer.h"

int main(int argc, char *argv[]) {
  SourceManager sm;
  std::vector<std::string> args;

  for (int i = 1; i < argc; i++) {
    args.push_back(argv[i]);
    sm.addFile(argv[i]);
    printf("%s.\n", argv[i]);
  }

  auto globalSymbolTable = std::make_shared<SymbolTable>();
  auto context = std::make_shared<llvm::LLVMContext>();
  auto globalTypeTable = std::make_shared<GlobalTypeTable>();
  globalTypeTable->initBuiltinTypes();
  for (int i = args.size() - 1; i >= 0; i--) {
    printf("#==== parsing %s\n", args[i].c_str());
    auto buff = std::make_shared<SourceBuffer>(sm.readSource(args[i]));

    Lexer lexer(buff);

    std::vector<std::unique_ptr<Token>> tokens = lexer.lex();

    Parser parser(sm, buff, std::move(tokens), globalSymbolTable,
                  globalTypeTable);

    std::shared_ptr<ModuleDecl> parseTree = parser.parseProgram();

    std::cout << parseTree->getKind() << std::endl;

    PrinterAst printer(globalTypeTable, globalSymbolTable);
    parseTree->accept(printer);

    // semantic
    // if error -> exit(-1)

    // auto global_scope = globalSymbolTable->getGlobalScope();
    // CodeGenVisitor cgvisitor(sm, buff, global_scope, globalTypeTable,
    // context); parseTree->accept(cgvisitor);
    // // cgvisitor.visitDefault(parseTree);
    // cgvisitor.dumpIR();
    // cgvisitor.createObjFile();

    SemanticAnalyzer analyzer(globalTypeTable, globalSymbolTable);
    parseTree->accept(analyzer);
  }
  // auto buff = std::make_shared<SourceBuffer>(sm.readSource(args[0]));
  //
  // Lexer lexer(buff);
  //
  // std::vector<std::unique_ptr<Token>> tokens = lexer.lex();
  //
  // auto globalSymbolTable = std::make_shared<SymbolTable>();
  // auto globalTypeTable = std::make_shared<GlobalTypeTable>();
  //
  // Parser parser(sm, std::move(tokens), globalSymbolTable, globalTypeTable);
  //
  // std::shared_ptr<Entity> parseTree = parser.parseProgram();
  //
  // std::cout << parseTree->getKind() << std::endl;
};