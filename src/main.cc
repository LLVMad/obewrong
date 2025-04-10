#include <iostream>
#include <stdio.h>
#include <vector>

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
  auto buff = std::make_shared<SourceBuffer>(sm.readSource(args[0]));

  Lexer lexer(buff);

  std::vector<std::unique_ptr<Token>> tokens = lexer.lex();

  auto globalSymbolTable = std::make_shared<SymbolTable>();
  auto globalTypeTable = std::make_shared<GlobalTypeTable>();

  Parser parser(sm, std::move(tokens), globalSymbolTable, globalTypeTable);

  std::shared_ptr<Entity> parseTree = parser.parseProgram();

  std::cout << parseTree->getKind() << std::endl;

  PrinterAst printer(globalTypeTable, globalSymbolTable);
  printer.printAST(parseTree);
};