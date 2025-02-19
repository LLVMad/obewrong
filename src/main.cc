#include <stdio.h>
#include <iostream>

#include "Lexer.h"
#include "SourceManager.h"

int main() {
  SourceManager sm;
  auto buff = std::make_shared<SourceBuffer>(sm.readSource("/home/artjom/Uni/comp_constr/obewrong/test/hello_world.obw"));

  Lexer lexer(buff);

  for (int i = 0; i < 20; i++) {
    std::unique_ptr<Token> next_tok = lexer.next();
    if (next_tok) {
      std::cout << "[ " << Lexer::getTokenTypeName(next_tok->type);

      if (!next_tok->lexeme.empty()) std::cout << ",\t" << next_tok->lexeme;
      if (next_tok->value.identId) std::cout << ",\t" << next_tok->value.identId;
      else if (next_tok->value.intValue) std::cout << ",\t" << next_tok->value.intValue;
      else if (next_tok->value.realValue) std::cout << ",\t" << next_tok->value.realValue;

      std::cout << " ]" << std::endl;
    }
    else std::cout << "nullptr" << std::endl;
  }
}