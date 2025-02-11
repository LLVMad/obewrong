#include <stdio.h>
#include <iostream>

#include "Lexer.h"
#include "SourceManager.h"

int main() {
  /*
  Lexer lexer("/home/artjom/Uni/comp_constr/obewrong/test/counter.obw");

  lexer.next();
  lexer.next();
  lexer.next();
  */

  SourceManager sm;
  SourceBuffer buff = sm.readSource("/home/artjom/Uni/comp_constr/obewrong/test/counter.obw");
  std::cout << buff.data << std::endl;
}