#ifndef _CodeGen_H
#define _CodeGen_H

#include "header.h"
#include <vector>

void CodeGenStream(const char *format, ...);
void DebugInfo(AST_NODE *node, const char *format, ...);
void genGeneralNode(AST_NODE *node);
void codeGen(AST_NODE* prog);
void genIfStmt(AST_NODE *node);
void genStmtNode(AST_NODE *node);
int genCondition(AST_NODE *node);
void genReturnStmt(AST_NODE *node);
void genGeneralNodeWithSibling(AST_NODE *node);

#endif
