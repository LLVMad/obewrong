#ifndef PRINTER_AST_H
#define PRINTER_AST_H

#include "frontend/SymbolTable.h"
#include "frontend/TypeTable.h"
#include "frontend/parser/Entity.h"
#include <string>
#include <vector>

inline std::string blockKindToString(BlockKind kind) {
  switch (kind) {
  case BLOCK_IN_CLASS:
    return "BLOCK_IN_CLASS";
  case BLOCK_IN_METHOD:
    return "BLOCK_IN_METHOD";
  case BLOCK_IN_FUNCTION:
    return "BLOCK_IN_FUNCTION";
  case BLOCK_IN_WHILE:
    return "BLOCK_IN_WHILE";
  case BLOCK_IN_IF:
    return "BLOCK_IN_IF";
  case BLOCK_IN_FOR:
    return "BLOCK_IN_FOR";
  case BLOCK_IN_SWITCH:
    return "BLOCK_IN_SWITCH";
  default:
    return "UNKNOWN_BLOCK";
  }
}

class PrinterAst {
public:
  PrinterAst(std::shared_ptr<GlobalTypeTable> globalTypeTable,
             std::shared_ptr<SymbolTable> symbolTable)
      : globalTypeTable(globalTypeTable), symbolTable(symbolTable) {}

  void printAST(const std::shared_ptr<Entity> &root);

private:
  std::shared_ptr<GlobalTypeTable> globalTypeTable;
  std::shared_ptr<SymbolTable> symbolTable;

  void printExpression(const std::shared_ptr<Expression> &expr, int indent);
  void printEntity(const std::shared_ptr<Entity> &entity, int indent);
  void printBlock(const std::shared_ptr<Block> &block, int indent);
  void printIfSTMT(const std::shared_ptr<IfSTMT> &ifStmt, int indent);
  void printVarDecl(const std::shared_ptr<VarDecl> &varDecl, int indent);
  void printModuleDecl(const std::shared_ptr<ModuleDecl> &moduleDecl,
                       int indent);
  void printClassDecl(const std::shared_ptr<ClassDecl> &classDecl, int indent);
  void printConstrDecl(const std::shared_ptr<ConstrDecl> &constrDecl,
                       int indent);
  void printParamDecl(const std::shared_ptr<ParameterDecl> &paramDecl,
                      int indent);
  void printFieldDecl(const std::shared_ptr<FieldDecl> &fieldDecl, int indent);
  void printMethodDecl(const std::shared_ptr<MethodDecl> &methodDecl,
                       int indent);
  void printEnumDecl(const std::shared_ptr<EnumDecl> &enumDecl, int indent);
  void printArrayDecl(const std::shared_ptr<ArrayDecl> &arrDecl, int indent);
  void printRealLiteral(const std::shared_ptr<RealLiteralEXP> &realLit,
                        int indent);

  void printVarRef(const std::shared_ptr<VarRefEXP> &varRef, int indent);
  void printFieldRef(const std::shared_ptr<FieldRefEXP> &fieldRef, int indent);

  void printReturnSTMT(const std::shared_ptr<ReturnSTMT> &returnStmt,
                       int indent);
  void printSwitchSTMT(const std::shared_ptr<SwitchSTMT> &switchStmt,
                       int indent);
  void printCaseSTMT(const std::shared_ptr<CaseSTMT> &caseStmt, int indent);
  void printWhileSTMT(const std::shared_ptr<WhileSTMT> &whileStmt, int indent);

  void printAssignment(const std::shared_ptr<AssignmentSTMT> &assignment,
                       int indent);
  void printIntLiteral(const std::shared_ptr<IntLiteralEXP> &intLiteral,
                       int indent);
  void printMethodCall(const std::shared_ptr<MethodCallEXP> &methodCall,
                       int indent);
  void printFuncDecl(const std::shared_ptr<FuncDecl> &funcDecl, int indent);
  void printFuncCall(const std::shared_ptr<FuncCallEXP> &funcCall, int indent);
  void printBinaryOp(const std::shared_ptr<BinaryOpEXP> &op, int indent);

  void printStringLiteral(const std::shared_ptr<StringLiteralEXP> &strLit,
                          int indent);
  void printBoolLiteral(const std::shared_ptr<BoolLiteralEXP> &boolLit,
                        int indent);
  void printArrayLiteral(const std::shared_ptr<ArrayLiteralExpr> &arrLit,
                         int indent);
  void printEnumRef(const std::shared_ptr<EnumRefEXP> &enumRef, int indent);

  void printThis(const std::shared_ptr<ThisEXP> &thisExp, int indent);
  void printForLoop(const std::shared_ptr<ForSTMT> &forLoop, int indent);
  void printClassName(const std::shared_ptr<ClassNameEXP> &classNameExp,
                      int indent);
  void printConstrCall(const std::shared_ptr<ConstructorCallEXP> &constrCall,
                       int indent);
  void printUnaryOp(const std::shared_ptr<UnaryOpEXP> &op, int indent);
};

#endif