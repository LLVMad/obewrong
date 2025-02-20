#include <stdio.h>
#include <iostream>

#include "frontend/Lexer.h"
#include "frontend/SourceManager.h"

int main(int argc, char *argv[]) {
  if (argc != 3) {}

  SourceManager sm;
  auto buff = std::make_shared<SourceBuffer>(sm.readSource(argv[1]));

  Lexer lexer(buff);

  int times = std::stoi(argv[2]);
  for (int i = 0; i < times; i++) {
    std::unique_ptr<Token> next_tok = lexer.next();
    if (next_tok) {
      std::cout << "[ "
                << next_tok->line << ":"
                << next_tok->column << " "
                << Lexer::getTokenTypeName(next_tok->type);

      // if (!next_tok->.empty()) std::cout << ",\t" << next_tok->lexeme;
      // if (next_tok->value.identId) std::cout << ",\t" << next_tok->value.identId;
      switch (next_tok->value.index()) {
        case 1: {
          std::cout << ", " << std::get<int>(next_tok->value);
        } break;
        case 2: {
          std::cout << ", " << std::get<double>(next_tok->value);
        } break;
        case 3: {
          std::cout << ", " << std::get<std::string>(next_tok->value);
        } break;
        default: break;
      }

      std::cout << " ]" << std::endl;
    }
    else std::cout << "nullptr" << std::endl;
  }
}