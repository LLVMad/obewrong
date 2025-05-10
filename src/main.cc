#include <iostream>
#include <stdio.h>
#include <vector>

#include "backend/CodegenVisitor.h"
#include "frontend/SourceManager.h"
#include "frontend/lexer/Lexer.h"
#include "frontend/parser/Parser.h"
#include "frontend/semantic/PrinterAst.h"

int main(int argc, char *argv[]) {
  SourceManager sm;
  std::vector<std::string> args;

  for (int i = 1; i < argc; i++) {
    args.push_back(argv[i]);
    sm.addFile(argv[i]);
    printf("%s.\n", argv[i]);
  }

  auto globalSymbolTable = std::make_shared<SymbolTable>();
  auto globalTypeTable = std::make_shared<GlobalTypeTable>();
  for (int i = args.size() - 1; i >= 0; i--) {
    printf("#==== parsing %s\n", args[i].c_str());
    auto buff = std::make_shared<SourceBuffer>(sm.readSource(args[i]));

    Lexer lexer(buff);

    std::vector<std::unique_ptr<Token>> tokens = lexer.lex();

    Parser parser(sm, std::move(tokens), globalSymbolTable, globalTypeTable);

    std::shared_ptr<Entity> parseTree = parser.parseProgram();

    std::cout << parseTree->getKind() << std::endl;

    PrinterAst printer(globalTypeTable, globalSymbolTable);
    printer.printAST(parseTree);

    // semantic
    // if error -> exit(-1)

    auto global_scope = globalSymbolTable->getGlobalScope();
    CodeGenVisitor cgvisitor(global_scope, globalTypeTable);
    cgvisitor.visitDefault(parseTree);
    cgvisitor.dumpIR();
    cgvisitor.createObjFile();

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