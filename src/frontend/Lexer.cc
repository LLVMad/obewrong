#include <iostream>
#include <fstream>
#include <sstream>

#include "Lexer.h"

Lexer::Lexer(SourceBuffer buffer) {
  this->current = nullptr;
  this->buffer = buffer.data.c_str();
}

Lexer::~Lexer() {

}

std::unique_ptr<Token> Lexer::next() {
  // char ch = source_file.get();
  //
  // printf("%c ", ch);
  //
  return nullptr;
}
