#include <stdio.h>
#include <iostream>

#include "Lexer.h"
#include "SourceManager.h"

int main() {
  SourceManager sm;
  auto buff = std::make_shared<SourceBuffer>(sm.readSource("/home/artjom/Uni/comp_constr/obewrong/test/hello_world.obw"));
  // std::cout << buff.data << std::endl;

  Lexer lexer(buff);

  for (int i = 0; i < 5; i++) {
    std::unique_ptr<Token> next_tok = lexer.next();
    if (next_tok) std::cout << "Token: " << next_tok->type << std::endl;
    else std::cout << "nullptr" << std::endl;
  }
}