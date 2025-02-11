#include <utility>
#include <iostream>
#include <fstream>

#include "Lexer.h"

#include <cstring>

Lexer::Lexer(const std::string& file_path) {
  this->current = nullptr;
  this->file_path = file_path;
  this->source_file = std::ifstream(file_path);

  if (!this->source_file) {
    std::cerr << "Failed to open file " << file_path << std::endl;
  }
}

Lexer::~Lexer() {
  this->source_file.close();
}

std::unique_ptr<Token> Lexer::next() {

  char ch = source_file.get();

  printf("%c ", ch);

  return nullptr;
}