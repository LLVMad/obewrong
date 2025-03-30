#include <iostream>
#include <stdio.h>
#include <vector>

#include "frontend/SourceManager.h"
#include "frontend/lexer/Lexer.h"
#include "frontend/parser/Parser.h"

int main(int argc, char *argv[]) {
  if (argc != 3) {
  }

  SourceManager sm;
  auto buff = std::make_shared<SourceBuffer>(sm.readSource(argv[1]));

  Lexer lexer(buff);

  std::vector<std::unique_ptr<Token>> tokens = lexer.lex();

  auto globalSymbolTable = std::make_shared<GlobalSymbolTable>();
  auto globalTypeTable = std::make_shared<GlobalTypeTable>();

  Parser parser(std::move(tokens), globalSymbolTable, globalTypeTable);

  std::shared_ptr<Entity> parseTree = parser.parseProgram();

  std::cout << parseTree->getKind() << std::endl;
};