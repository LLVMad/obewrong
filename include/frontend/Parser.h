#ifndef OBW_PARSER_H
#define OBW_PARSER_H

class Parser {
public:

private:
  /*
    Parse function for each non-terminal construct:
      Program
      ClassDeclaration
      ClassName
      MemberDeclaration
      VariableDeclaration
      MethodDeclaration
      MethodHeader
      MethodBody
      Parameters
      ParameterDeclaration
      Body
      ConstructorDeclaration
      Statement
      Assignment
      WhileLoop
      IfStatement
      ReturnStatement
      Expression
      Primary
      Arguments
   */
  void parseProgram();
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
}

#endif
