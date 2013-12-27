#ifndef _CodeGen_H
#define _CodeGen_H

#include "header.h"
#include <vector>

// GenSpecific Type Node
void genIfStmt(AST_NODE *node);
void genWhileStmt(AST_NODE* node);
void genForStmt(AST_NODE* node);

// UnDone Function
void genReturnStmt(AST_NODE *node);
void genStmtNode(AST_NODE *node);
int genCondition(AST_NODE *node);

// Starter Function
void codeGen(AST_NODE* prog);

// Utility Function
void genGeneralNodeWithSibling(AST_NODE *node);
void genGeneralNode(AST_NODE *node);
void CodeGenStream(const char *format, ...);
void DebugInfo(AST_NODE *node, const char *format, ...);
#endif
