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
RegisterSystem regSystem;
ARSystem ar;

/* Starting function */
void codeGen(AST_NODE* prog){
    genGeneralNode(prog);
}

void genVariable(AST_NODE *IDNode, bool isParam = false){
    SymbolTableEntry *entry = IDNode->getSymbol();
    TypeDescriptor *type = entry->attribute->getTypeDes();

    ar.addVariable(entry, isParam);
    // global variable, store in global data field
    if(entry->nestingLevel == 0) {
        Address addr = ar.getAddress(IDNode);
        CodeGenStream(".data");
        // SCALAR TYPE
        if(IDNode->getIDKind() == NORMAL_ID){
            if(type->getDataType() == INT_TYPE)
                CodeGenStream("%s:\t.word %d", addr.getName(), 0);
            else if(type->getDataType() == FLOAT_TYPE)
                CodeGenStream("%s:\t.float %lf", addr.getName(), 0.0);
        }
        // ARRAY TYPE
        else if(IDNode->getIDKind() == ARRAY_ID){
            CodeGenStream("%s:\t.space %d", addr.getName(), type->size());
        }
        // SCALAR WITH INIT 
        else if(IDNode->getIDKind() == WITH_INIT_ID){
            if(type->getDataType() == INT_TYPE) 
                CodeGenStream("%s:\t.word 0", addr.getName());
            else if(type->getDataType() == FLOAT_TYPE)
                CodeGenStream("%s:\t.float 0.0", addr.getName());
            // inline sub routine would called when main function prologue
            CodeGenStream(".text");

            char routineStart[1024], routineEnd[1024];
            ar.getTag("_InlineSubRoutine_Start", routineStart);
            ar.getTag("_InlineSubRoutine_End", routineEnd);
            CodeGenStream("%s:", routineStart);
            genExprRelatedNode(IDNode->child);
            Register *reg = IDNode->child->getTempReg();
            reg->save(addr);
            CodeGenStream("j %s", routineEnd);
            ar.globleInitRoutine(routineStart, routineEnd);
        }
    }
    // local variable, store in local ar field
    else {
        if(IDNode->getIDKind() == WITH_INIT_ID){
            genExprRelatedNode(IDNode->child);
            Register *reg = IDNode->getTempReg(RegDisableload);
            reg->load(IDNode->child->getTempReg());
        }
    }
}

void genDeclareNode(AST_NODE *node){
    DebugInfo(node, "gen declare node");
    switch(node->getDeclKind()){
        case VARIABLE_DECL:
            {
                node = node->child->rightSibling;
                while(node){
                    genVariable(node);
                    node = node->rightSibling;
                }
                break;
            }
        case TYPE_DECL:
            DebugInfo(node, "TypeDef node, doesn't need to do anything");
            break;
        case FUNCTION_DECL:
            {
                AST_NODE *nameNode = node->child->rightSibling;
                ar.prologue(nameNode->getIDName());
                genGeneralNodeWithSibling(node->child);
                ar.epilogue();
                break;
            }
        case FUNCTION_PARAMETER_DECL:
            {
                node = node->child->rightSibling;
                while(node){
                    genVariable(node, true);
                    node = node->rightSibling;
                }
                break;
            }
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

    genExprRelatedNode(rightValue);

    Register *leftreg = leftValue->getTempReg(RegDisableload);
    Register *rightreg = rightValue->getTempReg();
    leftreg->load(rightreg);
}

void genForStmt(AST_NODE* node){
    DebugInfo(node->child, "gen For stmt");

    char ForStmtTag[1024];
    ar.getTag("_FOR", ForStmtTag);

    AST_NODE *initNode = node->child;
    AST_NODE *conditionNode = initNode->rightSibling;
    AST_NODE *stepNode = conditionNode->rightSibling;
    AST_NODE *blockNode = stepNode->rightSibling;

    genGeneralNode(initNode);
    CodeGenStream("%s:", ForStmtTag);
    genGeneralNode(conditionNode);
    conditionNode->getTempReg()->branch("_End%s", ForStmtTag);
    genGeneralNode(blockNode);
    genGeneralNode(stepNode);
    CodeGenStream("j\t%s", ForStmtTag);
    CodeGenStream("_End%s:", ForStmtTag);
}

void genWhileStmt(AST_NODE* node){
    DebugInfo(node->child, "gen While stmt");

    char WhileStmtTag[1024];
    ar.getTag("_WHILE", WhileStmtTag);

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
}

void genReadFunction(AST_NODE *node){
    Register *reg = node->getTempReg();
    switch(node->getDataType()){
        case INT_TYPE:
            genSyscall(READ_INT, reg);
            break;
        case FLOAT_TYPE:
            genSyscall(READ_FLOAT, reg);
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
        //regSystem.saveCallerReg();

        Register *sp = regSystem.getReg("$sp");
        int paramleft = curFuncNameNode->getSymbol()->attribute->getFuncSig()->getParameterCount();
        sp->operand(BINARY_OP_SUB, sp, 4*paramleft);

        AST_NODE *param = paramListNode->child;
        for(int i=0; i<paramleft; i++){
            genGeneralNode(param);
            param->getTempReg()->save(Address(sp) + 4*i + 4);
            param = param->rightSibling;
        }
        CodeGenStream("jal\t%s", curFuncName);
        sp->operand(BINARY_OP_ADD, sp, 4*paramleft);
        Register *v0 = regSystem.getReg("$v0");
        Register *reg = node->getTempReg();
        reg->load(v0);
        //CodeGenStream("addi\t$sp, $sp, -4");
        //CodeGenStream("sw\t$12, 4($sp)");
        //CodeGenStream("jal\t%s", curFuncNameNode->getIDName());
        //CodeGenStream("addi\t$sp, $sp, 4");
        //sp->operand(BINARY_OP_ADD, sp, -offset);
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

    char EndTag[1024];
    ar.getEndTag(EndTag);
    CodeGenStream("j\t%s", EndTag);
}



void genIfStmt(AST_NODE *node){
    DebugInfo(node->child, "gen If stmt");

    char IfStmtTag[1024];
    ar.getTag("_IF", IfStmtTag);

    AST_NODE* conditionNode = node->child;
    AST_NODE* blockNode = conditionNode->rightSibling;
    AST_NODE* elseNode = blockNode->rightSibling;

    bool hasElse = false;
    char branchName[1024];
    if(elseNode->type() != NUL_NODE){
        hasElse = true;
        ar.getTag("_IfBranch", branchName);
    }

    genGeneralNode(conditionNode);
    if(hasElse) 
        conditionNode->getTempReg()->branch(branchName);

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
            ar.addVariable(node->getCharPtrValue());
            // do not load word, instead load address, specify 'la' instead of 'lw'
            reg->load(ar.getAddress(node), false);
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
            node->setRegister(node->child->getTempReg());
            break;
        default:
            DebugInfo(node, "Unknown Node type: %d", node->type());
    }
}
