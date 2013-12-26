#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

#include "codeGen.h"
#include "header.h"

FILE *f;

void DebugInfo(AST_NODE *node, const char *format, ...){
#ifdef DEBUG
  va_list args;
  fprintf(stderr, "line[%d]: ", node->linenumber);
  va_start(args, format);
  vfprintf(stderr, format, args);
  fprintf(stderr, "\n");
  va_end(args);
#endif
}
void CodeGenStream(const char *format, ...){
  va_list args;
  va_start(args, format);
  vfprintf(f, format, args);
  va_end(args);
  fprintf(f, "\n");
}

void codeGen(AST_NODE* prog){
  f = fopen("test.output", "w");
  DebugInfo(prog, "start codeGen");
  genGeneralNode(prog);
  DebugInfo(prog, "end codeGen");
}

void genGeneralNode(AST_NODE *node){
  while(node != NULL){
    switch(node->type()){
      case PROGRAM_NODE:
        genGeneralNode(node->child);
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
      case CONST_VALUE_NODE:
        break;
      case NONEMPTY_ASSIGN_EXPR_LIST_NODE:
        break;
      case NONEMPTY_RELOP_EXPR_LIST_NODE:
        break;
      default:
        DebugInfo(node, "Unknown Node type: %d", node->type());
    }
    node = node->rightSibling;
  }
}
