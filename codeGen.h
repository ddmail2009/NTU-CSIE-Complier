#ifndef _CodeGen_H
#define _CodeGen_H

#include "header.h"

void CodeGenStream(const char *format, ...);
void DebugInfo(AST_NODE *node, const char *format, ...);
void genGeneralNode(AST_NODE *node);
void codeGen(AST_NODE* prog);

#endif
