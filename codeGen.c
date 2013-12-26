#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "codeGen.h"

void Error(char *str){
  fprintf(stderr, str);
}

void codeGen(AST_NODE* prog){
  Error("start codeGen");
  genGeneralNode(prog);
}

void genGeneralNode(AST_Node *node){
  switch(node->type()){
    case PROGRAM_NODE:
      genProgramNode(prog);
      break;
    case PROGRAM_NODE:
      break;
    case DECLARATION_NODE:
      break;
    case IDENTIFIER_NODE:
      break;
    case PARAM_LIST_NODE:
      break;
    case NUL_NODE:
      break;
    case BLOCK_NODE:
      break;
    case VARIABLE_DECL_LIST_NODE:
      break;
    case STMT_LIST_NODE:
      break;
    case STMT_NODE:
      break;
    case EXPR_NODE:
      break;
    case CONST_VALUE_NODE,:
      break;
    case NONEMPTY_ASSIGN_EXPR_LIST_NODE:
      break;
    case NONEMPTY_RELOP_EXPR_LIST_NOD:
      break;
    default:
      Error(node, "Unknown Node type: %d", node->type())
  }
}
