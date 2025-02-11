#include <iostream>
#include <fstream>
#include <sstream>

#include "Lexer.h"

Lexer::Lexer(const std::string& file_path) {
  this->current = nullptr;
  this->file_path = file_path;

  std::ifstream source_file = std::ifstream(file_path);
  if (!source_file) {
    std::cerr << "Failed to open file " << file_path << std::endl;
  }

  std::stringstream source_stream;
  source_stream << source_file.rdbuf();
  this->buffer = source_stream.str();
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
