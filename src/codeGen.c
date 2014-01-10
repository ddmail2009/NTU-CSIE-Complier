#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <vector>

#include "codeGen.h"
#include "header.h"
#include "Register.h"
#include "symbolTable.h"
#include "gen-part.h"

std::vector<const char*> strConst;
const char* curFuncName;
int curFuncOffset;
extern RegisterSystem regSystem;

/* Utility Function */
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
    static FILE *f = fopen("output.s", "w");
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


/* Starting function */
void codeGen(AST_NODE* prog){
    genGeneralNode(prog);
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
                CodeGenStream("_%s:\t.word %d", entry->name, IDNode->child->getConIntValue());
            else if(type->getDataType() == FLOAT_TYPE)
                CodeGenStream("_%s:\t.float %lf", entry->name, IDNode->child->getConFloatValue());
        }
        entry->setAddress(new Address("_%s", entry->name));
    }
    // local variable
    else {
        curFuncOffset -= type->size();

        Register *fp = regSystem.getReg("$fp");
        entry->setAddress(new Address(fp, curFuncOffset));

        if(IDNode->getIDKind() == WITH_INIT_ID){
            genExprRelatedNode(IDNode->child);
            if(type->getDataType() == INT_TYPE)
                CodeGenStream("sw\t%s, %s", IDNode->child->getTempName(), entry->getAddress().getName());
            else if(type->getDataType() == FLOAT_TYPE)
                CodeGenStream("s.s\t%s, %s", IDNode->child->getTempName(), entry->getAddress().getName());
        }
        DebugInfo(IDNode, "set ID: %s to offset: %d", entry->name, curFuncOffset);
    }
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
                curFuncName = nameNode->getIDName();
                curFuncOffset = 0;
                gen_head(curFuncName);
                gen_prologue(curFuncName);
                genGeneralNodeWithSibling(node->child);
                gen_epilogue(curFuncName);
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
    CodeGenStream("_framesize_%s: .word %d", curFuncName, curFuncOffset+32);
    for(std::vector<const char*>::size_type i=0; i!=strConst.size(); i++){
        CodeGenStream("_Str_%s%d: .asciiz %s", curFuncName, i, strConst[i]);
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

    Register *leftreg = leftValue->getTempReg(RegDisableload);
    Register *rightreg = rightValue->getTempReg();
    leftreg->load(rightreg);
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
    genGeneralNode(conditionNode);
    CodeGenStream("beqz\t%s, _End%s", conditionNode->getTempName(), ForStmtTag);
    genGeneralNode(blockNode);
    genGeneralNode(stepNode);
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
    genGeneralNode(conditionNode);
    CodeGenStream("beqz\t%s, _End%s", conditionNode->getTempName(), WhileStmtTag);
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
            node->getTempReg();
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

    genGeneralNode(paramNode);
    Register *reg = paramNode->getTempReg();

    DebugInfo(node, "register param @ %s", reg->name());
    switch(paramNode->getDataType()){
        case INT_TYPE:
            genSyscall(PRINT_INT, reg);
            break;
        case FLOAT_TYPE:
            genSyscall(PRINT_FLOAT, reg);
            break;
        case CONST_STRING_TYPE:
            genSyscall(PRINT_STRING, reg);
            break;
        default:
            ;
    }
    return ;
    switch(paramNode->type()){
        case IDENTIFIER_NODE:
        {
            Register *reg = paramNode->getTempReg();
            switch(paramNode->getDataType()){
                case INT_TYPE:
                    genSyscall(PRINT_INT, reg);
                    break;
                case FLOAT_TYPE:
                    genSyscall(PRINT_FLOAT, reg);
                    break;
                default:
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
                    CodeGenStream("la\t$a0, _Str_%s%d", curFuncName, strConst.size());
                    strConst.push_back(paramNode->getCharPtrValue());
                    break;
            }
            CodeGenStream("syscall");
            break;
        }
        default:
            DebugInfo(node, "Unhandle case in genWriteFunction, type: %d", paramNode->type());
    }
}

void genReadFunction(AST_NODE *node){
    Register *reg = node->getTempReg();
    switch(node->getDataType()){
        case INT_TYPE:
            genSyscall(READ_INT, reg);
            /*
            CodeGenStream("li\t$v0, 5");
            CodeGenStream("syscall");
            genMoveCommand(INT_TYPE, node->getDataType(), "v0", node->getTempName());
            */
            break;
        case FLOAT_TYPE:
            genSyscall(READ_FLOAT, reg);
            /*
            CodeGenStream("li\t$v0, 6");
            CodeGenStream("syscall");
            genMoveCommand(FLOAT_TYPE, node->getDataType(), "f0", node->getTempName());
            */
            break;
        default:
            DebugInfo(node, "Unhandle case in genReadFunction, type: %d", node->type());
            break;
    }
}

void genFunctionCall(AST_NODE *node){
    AST_NODE *curFuncNameNode = node->child;
    AST_NODE *paramListNode = node->child->rightSibling;

    const char *curFuncName = curFuncNameNode->getIDName();
    DebugInfo(node, "genFunctionCall: %s", curFuncName);
    if(!strcmp(curFuncName, "write"))
        genWriteFunction(node);
    else if(!strcmp(curFuncName, "read"))
        genReadFunction(node);
    else if(!strcmp(curFuncName, "fread"))
        genReadFunction(node);
    else {
        genGeneralNode(paramListNode);

        // TODO
        // Need to calculate the function offset
        CodeGenStream("addi\t$sp, $sp, -4");
        CodeGenStream("sw\t$12, 4($sp)");
        CodeGenStream("jal\t%s", curFuncNameNode->getIDName());
        CodeGenStream("addi\t$sp, $sp, 4");
        Register *reg = node->getTempReg();
        Register *v0 = regSystem.getReg("$v0");
        reg->load(v0);
        //genMoveCommand(INT_TYPE, node->getDataType(), "v0", node->getTempName());
    }
}


void genExprNode(AST_NODE *node){
    switch(node->getExprKind()){
        case BINARY_OPERATION:
            DebugInfo(node, "gen left");
            genExprRelatedNode(node->child);
            DebugInfo(node, "gen right");
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
    DebugInfo(node, "result reg: %s", node->getTempReg()->name());
}

void genReturnStmt(AST_NODE *node){
    DebugInfo(node, "gen return stmt");
    genExprRelatedNode(node->child);

    Register *v0 = regSystem.getReg("$v0");
    v0->load(node->child->getTempReg());
    CodeGenStream("j\t_end_%s", curFuncName);
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

    genGeneralNode(conditionNode);
    if(hasElse) CodeGenStream("beqz\t%s, %s", conditionNode->getTempName(), branchName);

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

void genOpStmt(AST_NODE *node){
    switch(node->getExprKind()){
        case BINARY_OPERATION:
        {
            AST_NODE *left = node->child;
            AST_NODE *right = node->child->rightSibling;

            Register *leftReg = left->getTempReg();
            Register *rightReg = right->getTempReg();
            Register *reg = node->getTempReg(RegforceCaller);
            reg->operand(node->getBinaryOp(), leftReg, rightReg);
            break;
        }
        case UNARY_OPERATION:
        {
            Register *reg = node->getTempReg(RegforceCaller);
            reg->operand(node->getUnaryOp(), node->child->getTempReg());
        }
        default:
            break;
    }
}

void genConStmt(AST_NODE *node){
    Register *reg = node->getTempReg();
    switch(node->getConType()){
        case INTEGERC:
            reg->load(node->getConIntValue());
            break;
        case FLOATC:
            reg->load(node->getConFloatValue());
            break;
        case STRINGC:
            char tmp[10];
            sprintf(tmp, "_Str_%s%d", curFuncName, strConst.size());
            reg->load(tmp);
            strConst.push_back(node->getCharPtrValue());
            break;
        default:
            DebugInfo(node, "Unhandle case in genExprRelatedNode:constValue, conType: %d", node->getConType());
            break;
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
            if(node->parent->type() != DECLARATION_NODE)
                node->getTempReg();
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
            genConStmt(node);
            break;
        case NUL_NODE:
            break;
        case PARAM_LIST_NODE:
            genGeneralNodeWithSibling(node->child);
            break;
        case STMT_LIST_NODE:
            genGeneralNodeWithSibling(node->child);
            if(node->child) node->setRegister(node->child->getTempReg());
            break;
        case VARIABLE_DECL_LIST_NODE:
            genGeneralNodeWithSibling(node->child);
            if(node->child) node->setRegister(node->child->getTempReg());
            break;
        case NONEMPTY_ASSIGN_EXPR_LIST_NODE:
            genGeneralNodeWithSibling(node->child);
            node->setRegister(node->child->getTempReg());
            break;
        case NONEMPTY_RELOP_EXPR_LIST_NODE:
            genGeneralNodeWithSibling(node->child);
            DebugInfo(node, "node child in reg: %s", node->child->getTempReg());
            node->setRegister(node->child->getTempReg());
            DebugInfo(node, "result in reg: %s", node->getTempReg());
            break;
        default:
            DebugInfo(node, "Unknown Node type: %d", node->type());
    }
}
