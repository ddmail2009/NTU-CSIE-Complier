#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

#include "codeGen.h"
#include "header.h"
#include "symbolTable.h"
#include "gen-part.h"
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
    fprintf(stderr, output);
}

void getTagName(const char *prefix, AST_NODE *node, char *name){
    sprintf(name, "%s_%d_%d", prefix, node->linenumber, node->type());
}

void codeGen(AST_NODE* prog){
    f = fopen("test.output", "w");
    DebugInfo(prog, "start codeGen");
    genGeneralNode(prog);
    DebugInfo(prog, "end codeGen");
}

void genDeclareNode(AST_NODE *node){
    DebugInfo(node, "gen declare node");
    switch(node->getDeclKind()){
        case VARIABLE_DECL:
            node = node->child->rightSibling;
            while(node){
                genVariable(node);
                node = node->rightSibling;
            }
            break;
        case TYPE_DECL:
            DebugInfo(node, "TypeDef node, doesn't need to do anything");
            break;
        case FUNCTION_DECL:
        {
            AST_NODE *nameNode = node->child->rightSibling;
            gen_head(nameNode->getIDName());
            gen_prologue(nameNode->getIDName());
            genGeneralNodeWithSibling(node->child);
            gen_epilogue(nameNode->getIDName(), 0);
            break;
        }
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
                genAssignStmt(node);
                break;
            case IF_STMT:
                genIfStmt(node);
                break;
            case FUNCTION_CALL_STMT:
                genFunctionCall(node);
                break;
            case RETURN_STMT:
                genReturnStmt(node);
                break;
            default:
                DebugInfo(node, "Unknown StmtType: %d", node->getStmtType());
                break;
        }
    }
}

void genAssignStmt(AST_NODE *node){
    AST_NODE *leftValue = node->child;
    AST_NODE *rightValue = node->child->rightSibling;

    leftValue->setTemporaryPlace(leftValue->getDataType());
    genExprRelatedNode(rightValue);
    char location[1024];
    leftValue->getSymbol()->getLocation(location);

    CodeGenStream("sw\t$%d, %s", rightValue->getTemporaryPlace(), location);
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
    genAssignOrExpr(conditionNode);
    CodeGenStream("beqz\t$%d, _End%s", conditionNode->getTemporaryPlace(), ForStmtTag);
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
    genAssignOrExpr(conditionNode);
    CodeGenStream("beqz\t$%d, _End%s", conditionNode->getTemporaryPlace(), WhileStmtTag);
    genGeneralNode(blockNode);
    CodeGenStream("j\t%s", WhileStmtTag);
    CodeGenStream("_End%s:", WhileStmtTag);
}

int getArrayOffset(AST_NODE *node){
    int offset = 0;
    if(node->getIDKind() == ARRAY_ID){
        TypeDescriptor *type = node->getSymbol()->attribute->getTypeDes();
        AST_NODE *dim = node->child;
        int dimension = 0;
        while(dim){
            offset += dim->getConIntValue();
            dim = dim->rightSibling;
            if(!dim) break;

            offset += type->getArrayDimensionSize(dimension);
            dimension += 1;
        }
    }
    return offset;
}

void genExprRelatedNode(AST_NODE *node){
    switch(node->nodeType){
        case EXPR_NODE:
            genExprNode(node);
            break;
        case STMT_NODE:
            //function call
            genFunctionCall(node);
            break;
        case IDENTIFIER_NODE:
        {
            //TODO
            int offset = getArrayOffset(node);

            char location[1024];
            node->setTemporaryPlace(node->getDataType());
            node->getSymbol()->getLocation(location, offset);
            DebugInfo(node, "IDName: %s, location: %s", node->getIDName(), location);
            CodeGenStream("lw\t$%d, %s", node->getTemporaryPlace(), location);
            break;
        }
        case CONST_VALUE_NODE:
            genConStmt(node);
            break;
        default:
            DebugInfo(node, "Unhandle case in genExprRelatedNode, type: %d", node->type());
            break;
    }
}

void genWriteFunction(AST_NODE *node){
    AST_NODE *paramNode = node->child->rightSibling->child;

    switch(paramNode->type()){
        case IDENTIFIER_NODE:
            DebugInfo(node, "write paramNode, IDKind: %d", paramNode->getIDKind());
            break;
        case CONST_VALUE_NODE:
        {
            switch(paramNode->getConType()){
                case INTEGERC:
                    genSysCall(SysCallParameter(PRINT_INT, paramNode->getConIntValue()));
                    break;
                case FLOATC:
                    genSysCall(SysCallParameter(PRINT_FLOAT, paramNode->getConFloatValue()));
                    break;
                case STRINGC:
                    DebugInfo(paramNode, "STRINGC : %s", paramNode->getCharPtrValue());
                    genSysCall(SysCallParameter(PRINT_STRING, paramNode->getCharPtrValue()));
                    break;
                default:
                    break;
            }
            break;
        }
        default:
            DebugInfo(node, "Unhandle case in genWriteFunction, type: %d", paramNode->type());
    }
}

void genFunctionCall(AST_NODE *node){
    AST_NODE *funcNameNode = node->child;
    AST_NODE *paramListNode = node->child->rightSibling;

    const char *funcName = funcNameNode->getIDName();
    DebugInfo(node, "genFunctionCall: %s", funcName);
    if(!strcmp(funcName, "write"))
        genWriteFunction(node);
    else if(!strcmp(funcName, "read"))
        ;//genReadFunction(node);
    else {
        genGeneralNode(paramListNode->child);
        node->setTemporaryPlace(node->getDataType());

        // TODO
        // Need to calculate the function offset
        CodeGenStream("addi\t$sp, $sp, -4");
        CodeGenStream("sw\t$12, 4($sp)");
        CodeGenStream("jal\t%s", funcNameNode->getIDName());
        CodeGenStream("addi\t$sp, $sp, 4");
        CodeGenStream("move\t$%d, $v0", node->getTemporaryPlace());
    }
}

void genExprNode(AST_NODE *node){
    switch(node->getExprKind()){
        case BINARY_OPERATION:
            genExprRelatedNode(node->child);
            genExprRelatedNode(node->child->rightSibling);
            break;
        case UNARY_OPERATION:
            genExprRelatedNode(node->child);
            break;
        default:
            DebugInfo(node, "Unhandle case in genExprNode, nodeExprKind: %d\n", node->getExprKind());
            break;
    }
    genOpStmt(node);
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

    genExprRelatedNode(node->child);
    CodeGenStream("move\t$v0, $%d", node->child->getTemporaryPlace());
    CodeGenStream("j\t_end_%s", cur->getIDName());
}

void genAssignOrExpr(AST_NODE *node){
    if(node->type() == STMT_NODE)
        genStmtNode(node);
    else 
        genExprRelatedNode(node);
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

    genAssignOrExpr(conditionNode);
    if(hasElse) CodeGenStream("beqz\t$%d, %s", conditionNode->getTemporaryPlace(), branchName);

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
            genGeneralNodeWithSibling(node->child);
            break;
        case DECLARATION_NODE:
            genDeclareNode(node);
            break;
        case IDENTIFIER_NODE:
            ;
            break;
        case BLOCK_NODE:
            genGeneralNodeWithSibling(node->child);
            break;
        case STMT_NODE:
            genStmtNode(node);
            break;
        case EXPR_NODE:
            genExprNode(node);
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
