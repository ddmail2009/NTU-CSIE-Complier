#include <cstdio>
#include <cstring>
#include <cstdlib>

#include "header.h"
#include "symbolTable.h"
#include "codeGen.h"
#include "gen-part.h"

const char* TEXT= ".text";
const char* DATA= ".data";
const char* WORD= ".word";
const char* SYSCALL = "syscall";
int reg_number = 7;
int floating_reg_number = 0;
int string_literal_number = 0;

extern void CodeGenStream(const char *format, ...);

int gen_head(const char *name) {
    CodeGenStream("%s", TEXT);
    CodeGenStream("%s:", name);
    return 0;
}

void gen_prologue(const char *functionName) {
    CodeGenStream("# prologue sequence");
    CodeGenStream("sw\t$ra, 0($sp)");
    CodeGenStream("sw\t$fp, -4($sp)");
    CodeGenStream("add\t$fp, $sp, -4");
    CodeGenStream("add\t$sp, $sp, -8");
    CodeGenStream("lw\t$2, _framesize_%s", functionName);
    CodeGenStream("sub\t$sp, $sp, $2");

    /* naive save Caller Save Register */
    for(int i = 0; i < 8; i++)
        CodeGenStream("sw\t$%d, %d($sp)", 8 + i, 32 - 4 * i);

    /* pushNewAR(); */ 

    CodeGenStream("_begin_%s:", functionName);
}

// offset is at least 32 bytes (at least 8 caller save registers)
void gen_epilogue(const char *functionName) {
    CodeGenStream("# epilogue sequence");
    CodeGenStream("_end_%s:", functionName);
    /* naive restore Caller Save Register */
    for(int i = 0; i < 8; i++)
        CodeGenStream("lw\t$%d,%d($sp)", 8 + i, 32 - 4 * i);

    CodeGenStream("lw\t$ra, 4($fp)"); // restore return address
    CodeGenStream("add\t$sp, $fp, 4"); // pop AR
    CodeGenStream("lw\t$fp, 0($fp)"); // restore caller (old) $fp
    if (strcmp(functionName, "main") == 0) {
        genSysCall(SysCallParameter(EXIT));
    }
    else {
        CodeGenStream("jr\t$ra");
    }
}


inline bool isCallerSaveRegister(int reg) {
    return (reg > 7 && reg < 16) || reg == 24 || reg == 25;
}

inline bool isCalleeSaveRegister(int reg) {
    return reg > 15 && reg < 24;
}

/* naive method */
int getReg(RegisterType type) {
    if(type == GENERAL) {
        // Isn't this  15 instead of 25?
        if(reg_number <= 25) {
            return ++reg_number;
        }
        reg_number = 8;
        return reg_number;
    }
    else {
        if(floating_reg_number < 32){
            return ++floating_reg_number;
        }
        floating_reg_number = 0;
        return floating_reg_number;
    }
    return -1;
}

/* In C--, we use double instead of float to implement FLOATC.
 *
 * NOTE: if write_string, PLEASE CALL genGlobalVariableWithInit FIRST
 *
 */
void genSysCall(const SysCallParameter& information) {
    switch(information.type) {
        case PRINT_INT:
            CodeGenStream("#print_int system call");
            CodeGenStream("li\t$v0, %d", information.type);
            CodeGenStream("la\t$a0, %d", information.val.ival);
            break;
        case PRINT_FLOAT:
            fprintf(stderr, "this case shouldn't be happened in C--\n");
            exit(1);
            break;
        case PRINT_DOUBLE:
            CodeGenStream("#print_double system call");
            CodeGenStream("li\t$v0, %d", information.type);
            CodeGenStream("la\t$f12, %lf", information.val.fval);
            break;
        case PRINT_STRING:
            CodeGenStream("string%d: .asciiz \"%s\"", string_literal_number++, information.val.str);
            CodeGenStream("#print_string system call");
            CodeGenStream("li\t$v0, %d", information.type);
            /* assume the const string is generated just before the genSysCall */
            CodeGenStream("la\t$a0, string%d", string_literal_number - 1);
            break;
        case READ_INT:
            CodeGenStream("#read_int system call");
            CodeGenStream("li\t$v0, %d", information.type);
            break;
        case READ_FLOAT:
            fprintf(stderr, "this case shouldn't be happened in C--\n");
            exit(1);
            break;
        case READ_DOUBLE:
            CodeGenStream("#read_double system call");
            CodeGenStream("li\t$v0, %d", information.type);
            break;
        case READ_STRING: 
            CodeGenStream("#read_string system call");
            CodeGenStream("li\t$v0, %d", information.type);
            CodeGenStream("la\t$a0, string%d", string_literal_number - 1); // address of string to print
            break;
        case SBRK:
            CodeGenStream("#sbrk system call");
            CodeGenStream("li\t$v0, %d", information.type);
            break;
        case EXIT:
            CodeGenStream("#exit system call");
            CodeGenStream("li\t$v0, %d", information.type);
            break;
        default:
            fprintf(stderr, "unknowed type of SysCall\n");
            exit(1);
            break;
    }
    CodeGenStream("%s", SYSCALL);
}

void genOpStmt(AST_NODE *node){
    const char Binarycommand[][2][10] = {
        {"add", "add.s"},
        {"sub", "sub.s"},
        {"mul", "mul.s"},
        {"div", "div.s"},
        {"seq", "c.eq.s"},
        {"sge", "c.le.s"},
        {"sle", "c.le.s"},
        {"sne", "c.eq.s"},
        {"sgt", "c.lt.s"},
        {"slt", "c.lt.s"},
        {"and", "and"},
        {"or" , "or"}
    };
    bool negate[] = {
        false,
        false,
        false,
        false,
        false,
        true,
        false,
        true,
        true,
        false,
        false,
        false
    };

    switch(node->getExprKind()){
        case BINARY_OPERATION:
        {
            AST_NODE *left = node->child;
            AST_NODE *right = node->child->rightSibling;

            node->setTemporaryPlace();

            char leftPlace[1024], rightPlace[1024];
            strcpy(leftPlace, left->getTemporaryPlace());
            strcpy(rightPlace, right->getTemporaryPlace());
            if(left->getDataType() == FLOAT_TYPE && right->getDataType() == INT_TYPE){
                sprintf(rightPlace, "f%d", getReg(FLOATING));
                CodeGenStream("cvt.s.w\t%s, %s", right->getTemporaryPlace(), rightPlace);
            }
            if(left->getDataType() == INT_TYPE && right->getDataType() == FLOAT_TYPE){
                sprintf(leftPlace, "f%d", getReg(FLOATING));
                CodeGenStream("cvt.s.w\t%s, %s", left->getTemporaryPlace(), rightPlace);
            }
            if(node->getDataType() == INT_TYPE && (left->getDataType() == FLOAT_TYPE || right->getDataType() == FLOAT_TYPE)){
                if(node->getBinaryOp() <= BINARY_OP_DIV)
                    CodeGenStream("%s\t$%s, $%s, $%s", Binarycommand[node->getBinaryOp()][FLOAT_TYPE], node->getTemporaryPlace(), leftPlace, rightPlace);
                else{
                    CodeGenStream("%s\t$%s, $%s", Binarycommand[node->getBinaryOp()][FLOAT_TYPE], leftPlace, rightPlace);
                    CodeGenStream("bc1f _False%d", node->linenumber);
                    CodeGenStream("li\t$%s, %d", node->getTemporaryPlace(), !negate[node->getBinaryOp()]);
                    CodeGenStream("_False%d:", node->linenumber);
                    CodeGenStream("li\t$%s, %d", node->getTemporaryPlace(), negate[node->getBinaryOp()]);
                }
            } else 
                CodeGenStream("%s\t$%s, $%s, $%s", Binarycommand[node->getBinaryOp()][node->getDataType()], node->getTemporaryPlace(), left->getTemporaryPlace(), right->getTemporaryPlace());
            break;
        }
        case UNARY_OPERATION:
        {
            switch(node->getUnaryOp()){
                case UNARY_OP_POSITIVE:
                    CodeGenStream("abs\t$%d, $%d", node->getTemporaryPlace(), node->getTemporaryPlace());
                    break;
                case UNARY_OP_NEGATIVE:
                    CodeGenStream("neg\t$%d, $%d", node->getTemporaryPlace(), node->getTemporaryPlace());
                    break;
                case UNARY_OP_LOGICAL_NEGATION:
                    CodeGenStream("not\t$%d, $%d",node->getTemporaryPlace(), node->getTemporaryPlace());
                    break;
                default:
                    break;
            }
        }
        default:
            break;
    }
}

void genConStmt(AST_NODE *node){
    node->setTemporaryPlace();
    switch(node->getConType()){
        case INTEGERC:
            CodeGenStream("li\t$%s, %d", node->getTemporaryPlace(), node->getConIntValue());
            break;
        case FLOATC:
            CodeGenStream("li.s\t$%s, %lf", node->getTemporaryPlace(), node->getConFloatValue());
            break;
        case STRINGC:
            DebugInfo(node, "Unhandle case in genExprRelatedNode:constValue, conType: %d", node->getConType());
            break;
        default:
            DebugInfo(node, "Unhandle case in genExprRelatedNode:constValue, conType: %d", node->getConType());
            break;
    }
}
