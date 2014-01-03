#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <vector>

#include "codeGen.h"
#include "header.h"
#include "symbolTable.h"
#include "gen-part.h"

FILE *f;
std::vector<const char*> strConst;
const char* funcName;
int funcOffset;

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
    f = fopen("output.s", "w");
    DebugInfo(prog, "start codeGen");
    genGeneralNode(prog);
    DebugInfo(prog, "end codeGen");
}

void genVariable(AST_NODE *IDNode){
    SymbolTableEntry *entry = IDNode->getSymbol();
    TypeDescriptor *type = entry->attribute->getTypeDes();

    // global variable
    if(entry->nestingLevel == 0) {
        CodeGenStream(".data");
        // SCALAR TYPE
        if(IDNode->getIDKind() == NORMAL_ID){
            if(type->getDataType() == INT_TYPE)
                CodeGenStream("_%s:\t.word %d", entry->name, 0);
            else if(type->getDataType() == FLOAT_TYPE)
                CodeGenStream("_%s:\t.float %lf", entry->name, 0.0);
        }
        // ARRAY TYPE
        else if(IDNode->getIDKind() == ARRAY_ID){
            int size = 0;
            for(int i=0; i<type->getDimension(); i++)
                size += type->getArrayDimensionSize(i);
            CodeGenStream("_%s:\t.space %d", entry->name, size);
        }
        //ARRAY TYPE
        else if(IDNode->getIDKind() == WITH_INIT_ID){
            if(type->getDataType() == INT_TYPE) 
                CodeGenStream("_%s:\t.word %d", entry->name, IDNode->getConIntValue());
            else if(type->getDataType() == FLOAT_TYPE)
                CodeGenStream("_%s:\t.float %lf", entry->name, IDNode->getConFloatValue());
        }
    }
    // local variable
    else {
        // calculate offset already in the same scope
        int offset = 0;
        SymbolTableEntry *cur = entry;
        while(cur){
            if(offset < cur->offset()) offset = cur->offset();
            cur = cur->nextInSameLevel;
        }

        offset += type->size(); 
        entry->setOffset(offset);

        if(IDNode->getIDKind() == WITH_INIT_ID){
            genExprRelatedNode(IDNode->child);
            if(type->getDataType() == INT_TYPE)
                CodeGenStream("sw\t$%s, %d($fp)", IDNode->child->getTemporaryPlace(), entry->offset());
            else if(type->getDataType() == FLOAT_TYPE)
                CodeGenStream("sw\t$%s, %d($fp)", IDNode->child->getTemporaryPlace(), entry->offset());
        }
        DebugInfo(IDNode, "set ID: %s to offset: %d", entry->name, offset);
    }
}

void genDeclareNode(AST_NODE *node){
    DebugInfo(node, "gen declare node");
    switch(node->getDeclKind()){
        case VARIABLE_DECL:
            node = node->child->rightSibling;
            while(node){
                genVariable(node);
                if(node->getSymbol()->offset() > funcOffset) funcOffset = node->getSymbol()->offset();
                node = node->rightSibling;
            }
            break;
        case TYPE_DECL:
            DebugInfo(node, "TypeDef node, doesn't need to do anything");
            break;
        case FUNCTION_DECL:
        {
            AST_NODE *nameNode = node->child->rightSibling;
            funcName = nameNode->getIDName();
            gen_head(funcName);
            gen_prologue(funcName);
            genGeneralNodeWithSibling(node->child);
            gen_epilogue(funcName);
            gen_epiDataField();
            break;
        }
        case FUNCTION_PARAMETER_DECL:
            break;
        default:
            DebugInfo(node, "Unknown DeclKind: %d", node->getDeclKind());
    }
}

void gen_epiDataField(){
    CodeGenStream(".data");
    CodeGenStream("_framesize_%s: .word %d", funcName, funcOffset+32);
    for(std::vector<const char*>::size_type i=0; i!=strConst.size(); i++){
        CodeGenStream("_Str_%s%d: .asciiz %s", funcName, i, strConst[i]);
    }
    strConst.clear();
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

    genExprRelatedNode(rightValue);
    char location[1024];
    leftValue->getSymbol()->getLocation(leftValue, location);

    if(leftValue->getSymbol()->attribute->getDataType() == INT_TYPE)
        CodeGenStream("sw\t$%s, %s", rightValue->getTemporaryPlace(), location);
    else 
        CodeGenStream("s.s\t$%s, %s", rightValue->getTemporaryPlace(), location);
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
    CodeGenStream("beqz\t$%s, _End%s", conditionNode->getTemporaryPlace(), ForStmtTag);
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
    CodeGenStream("beqz\t$%s, _End%s", conditionNode->getTemporaryPlace(), WhileStmtTag);
    genGeneralNode(blockNode);
    CodeGenStream("j\t%s", WhileStmtTag);
    CodeGenStream("_End%s:", WhileStmtTag);
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
            char location[1024];
            node->setTemporaryPlace();
            node->getSymbol()->getLocation(node, location);
            if(node->getDataType() == FLOAT_TYPE)
                CodeGenStream("l.s\t$%s, %s", node->getTemporaryPlace(), location);
            else    
                CodeGenStream("lw\t$%s, %s", node->getTemporaryPlace(), location);
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
        {
            char location[1024];
            SymbolTableEntry *entry = paramNode->getSymbol();
            entry->getLocation(paramNode, location);
            switch(entry->attribute->getTypeDes()->getDataType()){
                case INT_TYPE:
                    CodeGenStream("li\t$v0, 1");
                    CodeGenStream("lw\t$a0, %s", location);
                    break;
                case FLOAT_TYPE:
                    CodeGenStream("li\t$v0, 2");
                    CodeGenStream("l.s\t$f12, %s", location);
                    break;
                default:
                    DebugInfo(paramNode, "IDNode type: %d shouldn't happen in write parameter", entry->attribute->getTypeDes()->getDataType());
                    break;
            }
            break;
        }
        case CONST_VALUE_NODE:
        {
            switch(paramNode->getConType()){
                case INTEGERC:
                    CodeGenStream("li\t$v0, 1");
                    CodeGenStream("li\t$a0, %d", paramNode->getConIntValue());
                    break;
                case FLOATC:
                    CodeGenStream("li\t$v0, 2");
                    CodeGenStream("li.s\t$f12, %lf", paramNode->getConFloatValue());
                    break;
                case STRINGC:
                    CodeGenStream("li\t$v0, 4");
                    CodeGenStream("la\t$a0, _Str_%s%d", funcName, strConst.size());
                    strConst.push_back(paramNode->getCharPtrValue());
                    break;
            }
            break;
        }
        default:
            DebugInfo(node, "Unhandle case in genWriteFunction, type: %d", paramNode->type());
    }
    CodeGenStream("syscall");
}

void genReadFunction(AST_NODE *node){
    node->setTemporaryPlace();
    switch(node->getDataType()){
        case INT_TYPE:
            CodeGenStream("li\t$v0, 5");
            CodeGenStream("syscall");
            genMoveCommand(INT_TYPE, node->getDataType(), "v0", node->getTemporaryPlace());
            CodeGenStream("move\t$%s, $v0", node->getTemporaryPlace());
            break;
        case FLOAT_TYPE:
            CodeGenStream("li\t$v0, 6");
            CodeGenStream("syscall");
            genMoveCommand(FLOAT_TYPE, node->getDataType(), "f0", node->getTemporaryPlace());
            break;
        default:
            DebugInfo(node, "Unhandle case in genReadFunction, type: %d", node->type());
            break;
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
        genReadFunction(node);
    else if(!strcmp(funcName, "fread"))
        genReadFunction(node);
    else {
        genGeneralNode(paramListNode);

        // TODO
        // Need to calculate the function offset
        CodeGenStream("addi\t$sp, $sp, -4");
        CodeGenStream("sw\t$12, 4($sp)");
        CodeGenStream("jal\t%s", funcNameNode->getIDName());
        CodeGenStream("addi\t$sp, $sp, 4");
        node->setTemporaryPlace();
        genMoveCommand(INT_TYPE, node->getDataType(), "v0", node->getTemporaryPlace());
    }
}

void genMoveCommand(DATA_TYPE srctype, DATA_TYPE desttype, const char* src, const char* dest){
    if(srctype == INT_TYPE && desttype == INT_TYPE)
        CodeGenStream("move\t$%s, $%s", dest, src);
    else if(srctype == INT_TYPE && desttype == FLOAT_TYPE){
        CodeGenStream("mtc1\t$%s, $%s", src, dest);
        CodeGenStream("cvt.s.w\t$%s, $%s", dest, dest);
    }
    else if(srctype == FLOAT_TYPE && desttype == INT_TYPE){
        CodeGenStream("cvt.w.s $%s, $%s", src, src);
        CodeGenStream("mfc1\t$%s, $%s", dest, src);
    }
    else if(srctype == FLOAT_TYPE && desttype == FLOAT_TYPE)
        CodeGenStream("mov.s\t$%s, $%s", dest, src);
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
    genExprRelatedNode(node->child);
    genMoveCommand(node->getDataType(), INT_TYPE, node->child->getTemporaryPlace(), "v0");
    CodeGenStream("j\t_end_%s", funcName);
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
    if(hasElse) CodeGenStream("beqz\t$%s, %s", conditionNode->getTemporaryPlace(), branchName);

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
