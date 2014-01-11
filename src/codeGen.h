#ifndef _CodeGen_H
#define _CodeGen_H

#include "header.h"
#include <vector>

class Register;

// GenSpecific Type Node
void genIfStmt(AST_NODE *node);
void genWhileStmt(AST_NODE* node);
void genForStmt(AST_NODE* node);
void genExprNode(AST_NODE *node); //BINARY_OP, UNARY_OP...
void genAssignOrExpr(AST_NODE *node); //Condition Expr, a = 3 or 2 + 3

// UnDone Function
void genExprRelatedNode(AST_NODE *node);
void genReturnStmt(AST_NODE *node);
void genStmtNode(AST_NODE *node);
void genAssignOrExpr(AST_NODE *node);
void genFunctionCall(AST_NODE *node);
void genAssignStmt(AST_NODE *node);
void gen_epiDataField();

// Starter Function
void codeGen(AST_NODE* prog);

// Utility Function
void genGeneralNodeWithSibling(AST_NODE *node);
void genGeneralNode(AST_NODE *node);
void genConStmt(AST_NODE *node);
void genOpStmt(AST_NODE *node);
#endif
