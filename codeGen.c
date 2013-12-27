#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

#include "codeGen.h"
#include "header.h"
#include "symbolTable.h"
#include "gen-part.cpp"
FILE *f;

void DebugInfo(AST_NODE *node, const char *format, ...){
#ifdef DEBUG
  va_list args;
  fprintf(stderr, "\e[31mline[%d]: ", node->linenumber);
  va_start(args, format);
  vfprintf(stderr, format, args);
  fprintf(stderr, "\e[m\n");
  va_end(args);
#endif
}

void CodeGenStream(const char *format, ...){
  char indention[1024] = "";
  char output[1024];

  va_list args;
  va_start(args, format);
  char formation[1024];
  vsprintf(formation, format, args);
  sprintf(output, "%s%s\n", indention, formation);
  va_end(args);

  fprintf(f, output);
  fprintf(stdout, output);
}

void getTagName(const char *prefix, AST_NODE *node, char *name){
  sprintf(name, "%s_%d_%d", prefix, node->linenumber, node->type());
}

void codeGen(AST_NODE* prog){
  f = fopen("test.output", "w");
  DebugInfo(prog, "start codeGen");
  genGeneralNodeWithSibling(prog);
  DebugInfo(prog, "end codeGen");
}

void genVaraibleDeclNode(AST_NODE *node){
  AST_NODE *typeNode = node->child;
  AST_NODE *IDNode = typeNode->rightSibling;

  while(IDNode){
    SymbolTableEntry *entry = IDNode->getSymbol();
    bool isGlobal = entry->nestingLevel == 0 ? true : false;
    const char *IDName = entry->name;

    switch(IDNode->getIDKind()){
      case NORMAL_ID:
        DebugInfo(node, "normalID dataType: %d, IDName: %s", node->dataType, IDName);
        if(isGlobal)
          //genGlobalVariableWithInit(node->dataType, IDName);
        break;
      case ARRAY_ID:
        break;
      case WITH_INIT_ID:
        break;
    }
    IDNode = IDNode->rightSibling;
  }
}

void genDeclareNode(AST_NODE *node){
  DebugInfo(node, "gen declare node");
  switch(node->getDeclKind()){
    case VARIABLE_DECL:
      genVaraibleDeclNode(node);
      break;
    case TYPE_DECL:
      break;
    case FUNCTION_DECL:
      {
        AST_NODE *nameNode = node->child->rightSibling;
        gen_head(nameNode->getIDName());
        gen_prologue(nameNode->getIDName());
        genGeneralNodeWithSibling(node->child);
        gen_epilogue(nameNode->getIDName(), 0);
      }
      break;
    case FUNCTION_PARAMETER_DECL:
      break;
    default:
      DebugInfo(node, "Unknown DeclKind: %d", node->getDeclKind());
  }
}

void genStmtNode(AST_NODE *node){
  if(node->type() == BLOCK_NODE)
    genGeneralNode(node->child);
  else {
    switch(node->getStmtType()){
      case WHILE_STMT:
        genWhileStmt(node);
        break;
      case FOR_STMT:
        genForStmt(node);
        break;
      case ASSIGN_STMT:
        break;
      case IF_STMT:
        genIfStmt(node);
        break;
      case FUNCTION_CALL_STMT:
        break;
      case RETURN_STMT:
        genReturnStmt(node);
        break;
      default:
        DebugInfo(node, "Unknown StmtType: %d", node->getStmtType());
    }
  }
}

void genForStmt(AST_NODE* node){
  DebugInfo(node->child, "gen For stmt");

  char ForStmtTag[1024];
  getTagName("_FOR", node->child, ForStmtTag);

  AST_NODE *initNode = node->child;
  AST_NODE *conditionNode = initNode->rightSibling;
  AST_NODE *stepNode = conditionNode->rightSibling;
  AST_NODE *blockNode = stepNode->rightSibling;

  genGeneralNode(initNode);
  CodeGenStream("%s:", ForStmtTag);
  int reg = genCondition(conditionNode);
  CodeGenStream("beqz\t$%d, _End%s", reg, ForStmtTag);
  genGeneralNode(stepNode);
  genGeneralNode(blockNode);
  CodeGenStream("j\t%s", ForStmtTag);
  CodeGenStream("_End%s:", ForStmtTag);
}

void genWhileStmt(AST_NODE* node){
  DebugInfo(node->child, "gen While stmt");

  char WhileStmtTag[1024];
  getTagName("_WHILE", node->child, WhileStmtTag);

  AST_NODE *conditionNode = node->child;
  AST_NODE *blockNode = conditionNode->rightSibling;

  CodeGenStream("%s:", WhileStmtTag);
  int reg = genCondition(conditionNode);
  CodeGenStream("beqz\t$%d, _End%s", reg, WhileStmtTag);
  genGeneralNode(blockNode);
  CodeGenStream("j\t%s", WhileStmtTag);
  CodeGenStream("_End%s:", WhileStmtTag);
}

int genExprRelatedNode(AST_NODE* node){
  return 1;
}

void genReturnStmt(AST_NODE *node){
  DebugInfo(node, "gen return stmt");

  AST_NODE *cur = node;
  while(cur){
    if(cur->type() == DECLARATION_NODE && cur->getDeclKind() == FUNCTION_DECL)
      break;
    cur = cur->parent;
  }
  assert(cur!=NULL);
    cur = cur->child->rightSibling;

  int reg = genExprRelatedNode(node->child);
  //CodeGenStream("li\t$9, 1");
  //CodeGenStream("lw\t$9, 8($fp)");
  CodeGenStream("move\t$v0, $%d", reg);
  CodeGenStream("j\t_end_%s", cur->getIDName());
}

int genCondition(AST_NODE *node){
  CodeGenStream("load variable and compare here");
  return 1;
}


void genIfStmt(AST_NODE *node){
  DebugInfo(node->child, "gen If stmt");

  char IfStmtTag[1024];
  getTagName("_IF", node->child, IfStmtTag);

  AST_NODE* conditionNode = node->child;
  AST_NODE* blockNode = conditionNode->rightSibling;
  AST_NODE* elseNode = blockNode->rightSibling;

  bool hasElse = false;
  char branchName[1024];
  if(elseNode->type() != NUL_NODE){
    hasElse = true;
    getTagName("_IfBranch", node->child, branchName);
  }

  int reg = genCondition(conditionNode);
  if(hasElse) CodeGenStream("beqz\t$%d, %s", reg, branchName);

  genGeneralNode(blockNode);
  if(hasElse){
    char EndName[1024];
    sprintf(EndName, "_End%s", IfStmtTag);
    CodeGenStream("j\t%s", EndName);
    CodeGenStream("%s:", branchName);
    genGeneralNode(elseNode);
    CodeGenStream("%s:", EndName);
  }
}

void genGeneralNodeWithSibling(AST_NODE *node){
  while(node != NULL){
    genGeneralNode(node);
    node = node->rightSibling;
  }
}

void genGeneralNode(AST_NODE *node){
  switch(node->type()){
    case PROGRAM_NODE:
      genGeneralNode(node->child);
      break;
    case DECLARATION_NODE:
      genDeclareNode(node);
      break;
    case IDENTIFIER_NODE:
      break;
    case BLOCK_NODE:
      genGeneralNode(node->child);
      break;
    case STMT_NODE:
      genStmtNode(node);
      break;
    case EXPR_NODE:
      break;
    case CONST_VALUE_NODE:
      break;
    case NUL_NODE:
      break;
    case PARAM_LIST_NODE:
      genGeneralNodeWithSibling(node->child);
      break;
    case STMT_LIST_NODE:
      genGeneralNodeWithSibling(node->child);
      break;
    case VARIABLE_DECL_LIST_NODE:
      genGeneralNodeWithSibling(node->child);
      break;
    case NONEMPTY_ASSIGN_EXPR_LIST_NODE:
      genGeneralNodeWithSibling(node->child);
      break;
    case NONEMPTY_RELOP_EXPR_LIST_NODE:
      genGeneralNodeWithSibling(node->child);
      break;
    default:
      DebugInfo(node, "Unknown Node type: %d", node->type());
  }
}
