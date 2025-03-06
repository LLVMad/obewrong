#ifndef OBW_PARSER_H
#define OBW_PARSER_H

class Parser {
public:

private:
  void parseProgram();
  void parseProgramDecls();
  void parseClassDeclaration();
  void parseMemberDeclaration();
  void parseVariableDeclaration();
  void parseMethodDeclaration();
  void parseMethodHeader();
  void parseMethodBody();
  void parseParameters();
  void parseBody();
  void parseStatement();
  void parseAssignmentStatement();
  void parseIfStatement();
  void parseWhileStatement();
  void parseReturnStatement();
  void parseExpression();
  void parseMethodCall();
  void parseArguments();
  void parseConstructorDeclaration();
};

#endif
