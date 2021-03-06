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
            ar.globalInitRoutine(routineStart, routineEnd);
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

    DebugInfo("gen right value reg");
    genExprRelatedNode(rightValue);
    Register *rightreg = rightValue->getTempReg();
    regSystem.lock(rightreg, true);
    DebugInfo("gen left value reg");
    Register *leftreg = leftValue->getTempReg(RegDisableload);
    leftreg->load(rightreg);
    leftreg->save();
    regSystem.lock(rightreg, false);
}

void genForStmt(AST_NODE* node){
    DebugInfo(node->child, "gen For stmt");

    char ForStmtTag[1024];
    ar.getTag("_FOR", ForStmtTag);

    AST_NODE *initNode = node->child;
    AST_NODE *conditionNode = initNode->rightSibling;
    AST_NODE *stepNode = conditionNode->rightSibling;
    AST_NODE *blockNode = stepNode->rightSibling;

    DebugInfo("init Node Generate");
    genGeneralNode(initNode);

    regSystem.saveAndClear();
    CodeGenStream("%s:", ForStmtTag);
    DebugInfo("condition Node Generate");
    genGeneralNode(conditionNode);
    conditionNode->getTempReg()->branch("_End%s", ForStmtTag);
    regSystem.saveAndClear();

    DebugInfo("block Node Generate");
    genGeneralNode(blockNode);
    genGeneralNode(stepNode);
    regSystem.saveAndClear();
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
        // TODO
        // Need to calculate the function offset
        Register *sp = regSystem.getReg("$sp");
        FunctionSignature *func = curFuncNameNode->getSymbol()->attribute->getFuncSig();
        Parameter* actualparam = func->getParams();

        int paramleft = func->getParameterCount();
        sp->operand(BINARY_OP_SUB, sp, 4*paramleft);

        AST_NODE *param = paramListNode->child;
        for(int i=0; i<paramleft; i++){
            genGeneralNode(param);
            DebugInfo("acutal name: %s, type:%d, param type: %d", actualparam->parameterName, actualparam->type->getDataType(), param->getDataType());
            // if two param type match, just save
            if(actualparam->type->getDataType() == param->getDataType())
                param->getTempReg()->save(Address(sp) + 4*i + 4);
            // else convert to the corresponding param type
            else{
                Register *reg = regSystem.getReg(actualparam->type->getDataType(), true);
                reg->load(param->getTempReg());
                reg->save(Address(sp) + 4*i + 4);
            }

            actualparam = actualparam->next;
            param = param->rightSibling;
        }
        //regSystem.saveTempReg();
        CodeGenStream("jal\t%s", curFuncName);
        regSystem.clearRegRecord();

        sp->operand(BINARY_OP_ADD, sp, 4*paramleft);

        Register *returnReg;
        if(curFuncNameNode->getSymbol()->attribute->getDataType() == INT_TYPE)
            returnReg = regSystem.getReg("$v0");
        else 
            returnReg = regSystem.getReg("$f0");
        Register *reg = node->getTempReg();
        reg->load(returnReg);
    }
}


void genExprNode(AST_NODE *node){
    switch(node->getExprKind()){
        case BINARY_OPERATION:
            {
                AST_NODE *left = node->child;
                AST_NODE *right = node->child->rightSibling;

                DebugInfo(node, "gen left");
                genExprRelatedNode(left);
                regSystem.lock(left->getTempReg(), true);

                char branchTest[1024];
                ar.getTag("LogicalShort", branchTest);

                if(node->getBinaryOp() == BINARY_OP_AND){
                    // if false then short circuit
                    left->getTempReg()->branch("_False%s", branchTest);
                } else if(node->getBinaryOp() == BINARY_OP_OR){
                    // if left is true, the short circuit
                    left->getTempReg()->branch2("_False%s", branchTest);
                }

                DebugInfo(node, "gen right");
                genExprRelatedNode(right);
                regSystem.lock(right->getTempReg(), true);

                Register *leftReg = left->getTempReg();
                Register *rightReg = right->getTempReg();
                Register *reg = node->getTempReg(RegforceCaller);
                reg->operand(node->getBinaryOp(), leftReg, rightReg);
                if(node->getBinaryOp() == BINARY_OP_AND || node->getBinaryOp() == BINARY_OP_OR){
                    CodeGenStream("j\tEnd%s", branchTest);
                    CodeGenStream("_False%s:", branchTest);
                    Register *reg = node->getTempReg(RegforceCaller);
                    reg->load(left->getTempReg());
                    CodeGenStream("End%s:", branchTest);
                }
                regSystem.lock(left->getTempReg(), false);
                regSystem.lock(right->getTempReg(), false);
                break;
            }
        case UNARY_OPERATION:
            {
                genExprRelatedNode(node->child);

                Register *reg = node->getTempReg(RegforceCaller);
                reg->operand(node->getUnaryOp(), node->child->getTempReg());
                break;
            }
        default:
            DebugInfo(node, "Unhandle case in genExprNode, nodeExprKind: %d\n", node->getExprKind());
            break;
    }
    DebugInfo(node, "result reg: %s", node->getTempReg()->name());
}

void genReturnStmt(AST_NODE *node){
    DebugInfo(node, "gen return stmt");
    genExprRelatedNode(node->child);

    AST_NODE *tmp = node;
    while(1){
        if(tmp->type() == DECLARATION_NODE && tmp->getDeclKind() == FUNCTION_DECL)
            break;
        tmp = tmp->parent;
    }
    tmp = tmp->child->rightSibling;

    if(tmp->getSymbol()->attribute->getDataType() == INT_TYPE){
        Register *v0 = regSystem.getReg("$v0");
        v0->load(node->child->getTempReg());
    } else {
        Register *f0 = regSystem.getReg("$f0");
        f0->load(node->child->getTempReg());
    }

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

    char branchName[1024];
    ar.getTag("_IfBranch", branchName);
    char EndName[1024];
    sprintf(EndName, "_End%s", IfStmtTag);

    genGeneralNode(conditionNode);
    conditionNode->getTempReg()->branch(branchName);

    genGeneralNode(blockNode);
    regSystem.saveAndClear();
    CodeGenStream("j\t%s", EndName);
    CodeGenStream("%s:", branchName);
    genGeneralNode(elseNode);
    CodeGenStream("%s:", EndName);
}

void genConStmt(AST_NODE *node){
    Register *reg = node->getTempReg(RegforceCaller);
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
            reg->load(ar.getAddress(node));
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
            if(node->parent->type() != DECLARATION_NODE){
                node->getTempReg();
            }
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
