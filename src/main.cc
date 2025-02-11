#include "Lexer.h"
#include "stdio.h"

int main() {
  Lexer lexer("/home/artjom/Uni/comp_constr/obewrong/test/counter.obw");

  lexer.next();
  lexer.next();
  lexer.next();
}