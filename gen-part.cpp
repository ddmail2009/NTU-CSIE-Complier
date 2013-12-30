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
    CodeGenStream("sw\t$ra,0($sp)");
    CodeGenStream("sw\t$fp,-4($sp)");
    CodeGenStream("add\t$fp,$sp,-4");
    CodeGenStream("add\t$sp,$sp,-8");
    CodeGenStream("lw\t$2,_framesize_%s", functionName);
    CodeGenStream("sub\t$sp,$sp,$2");

    /* naive save Caller Save Register */
    for(int i = 0; i < 8; i++)
        CodeGenStream("sw\t$%d,%d($sp)", 8 + i, 32 - 4 * i);

    /* pushNewAR(); */ 

    CodeGenStream("_begin_%s:", functionName);
}

// offset is at least 32 bytes (at least 8 caller save registers)
void gen_epilogue(const char *functionName, int offset) {
    CodeGenStream("_end_%s:", functionName);
    /* naive restore Caller Save Register */
    for(int i = 0; i < 8; i++)
        CodeGenStream("lw\t$%d,%d($sp)", 8 + i, 32 - 4 * i);

    CodeGenStream("lw\t$ra, 4($fp)"); // restore return address
    CodeGenStream("add\t$sp, $fp, 4"); // pop AR
    CodeGenStream("lw\t$fp, 0($fp)"); // restore caller (old) $fp
    if (strcmp(functionName, "main") == 0) {
        SysCallParameter p(EXIT);
        genSysCall(&p);
    }
    else {
        CodeGenStream("jr\t$ra");
        CodeGenStream("%s", DATA);
    }

    /* determine the _framsize_functionName, and this value is at least 32
     * bytes
     */
    CodeGenStream("_framesize_%s: %s %d", functionName, WORD, offset);
}

void genGlobalVariableWithInit(const Variable* var) {
    CodeGenStream("%s", DATA);
    if(var->type() == INT_TYPE) {
        CodeGenStream("_%s:\t%s %d", var->getId(), WORD, var->getInt());
    }
    else if(var->type() == FLOAT_TYPE) {
        CodeGenStream("_%s:\t%s %lf", var->getId(), WORD, var->getFloat());
    }
    else if(var->type() == CONST_STRING_TYPE) {
        CodeGenStream("string%d: .asciiz \"%s\"", string_literal_number, var->getString());
        string_literal_number++;
    }
    else {
        fprintf(stderr, "unknowed type of global variable\n");
        exit(1);
    }
}

/* store it to AR, and load it to the register */
void genStackVariableWithInit(const SymbolTableEntry* entry, const Variable& var) {
    CodeGenStream("%s", TEXT);
    if(entry->attribute->getTypeDes()->getKind() == SCALAR_TYPE_DESCRIPTOR) {
        if(var.type() == INT_TYPE) {
            CodeGenStream("sw\t%d,%d($fp)", var.getInt(), entry->offset());
            CodeGenStream("lw\t$%d,%d($fp)",entry);
        }
        else if(var.type() == FLOAT_TYPE) {
            CodeGenStream("sw\t%lf,%d($fp)", var.getFloat(), entry->offset());
        }
        else { /* string literal -> put it in .data */
            CodeGenStream("string%d: .asciiz \"%s\"", string_literal_number, var.getString());
            string_literal_number++;
        }
    }
    else { // ARRAY_TYPE_DESCRIPTOR
        /* TODO */
    }
}

void genStoreToTempValueToRegister(const AST_NODE* node, DATA_TYPE type) {
    if(type == INT_TYPE) {
    }
    else if(type == FLOAT_TYPE) {
    }
    else {
        fprintf(stderr, "this case shouldn't be happened in C--\n");
        exit(1);
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
        if(reg_number <= 14) {
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
void genSysCall(const SysCallParameter* information) {
    switch(information->type) {
        case PRINT_INT:
            CodeGenStream("#print_int system call");
            CodeGenStream("li\t$v0, %d", information->type);
            CodeGenStream("la\t$a0, %d", information->val.ival);
            break;
        case PRINT_FLOAT:
            fprintf(stderr, "this case shouldn't be happened in C--\n");
            exit(1);
            break;
        case PRINT_DOUBLE:
            CodeGenStream("#print_double system call");
            CodeGenStream("li\t$v0, %d", information->type);
            CodeGenStream("la\t$f12, %lf", information->val.fval);
            break;
        case PRINT_STRING:
            CodeGenStream("#print_string system call");
            CodeGenStream("li\t$v0, %d", information->type);
            /* assume the const string is generated just before the genSysCall */
            CodeGenStream("la\t$a0, string%d", string_literal_number - 1);
            break;
        case READ_INT:
            CodeGenStream("#read_int system call");
            CodeGenStream("li\t$v0, %d", information->type);
            break;
        case READ_FLOAT:
            fprintf(stderr, "this case shouldn't be happened in C--\n");
            exit(1);
            break;
        case READ_DOUBLE:
            CodeGenStream("#read_double system call");
            CodeGenStream("li\t$v0, %d", information->type);
            break;
        case READ_STRING: 
            CodeGenStream("#read_string system call");
            CodeGenStream("li\t$v0, %d", information->type);
            CodeGenStream("la\t$a0, string%d", string_literal_number - 1); // address of string to print
            break;
        case SBRK:
            CodeGenStream("#sbrk system call");
            CodeGenStream("li\t$v0, %d", information->type);
            break;
        case EXIT:
            CodeGenStream("#exit system call");
            CodeGenStream("li\t$v0, %d", information->type);
            break;
        default:
            fprintf(stderr, "unknowed type of SysCall\n");
            exit(1);
            break;
    }
    CodeGenStream("%s", SYSCALL);
}

void genOpStmt(AST_NODE *node){
    switch(node->getExprKind()){
        case BINARY_OPERATION:
        {
            AST_NODE *left = node->child;
            AST_NODE *right = node->child->rightSibling;

            node->setTemporaryPlace(node->getDataType());
            switch(node->getBinaryOp()){
                case BINARY_OP_ADD:
                    CodeGenStream("add\t%d, $%d, %d", node->getTemporaryPlace(), left->getTemporaryPlace(), right->getTemporaryPlace());
                    break;
                case BINARY_OP_SUB:
                    CodeGenStream("sub\t$%d, $%d, $%d", node->getTemporaryPlace(), left->getTemporaryPlace(), right->getTemporaryPlace());
                    break;
                case BINARY_OP_MUL:
                    CodeGenStream("mul\t$%d, $%d, $%d", node->getTemporaryPlace(), left->getTemporaryPlace(), right->getTemporaryPlace());
                    break;
                case BINARY_OP_DIV:
                    CodeGenStream("div\t$%d, $%d, $%d", node->getTemporaryPlace(), left->getTemporaryPlace(), right->getTemporaryPlace());
                    break;
                case BINARY_OP_EQ:
                    CodeGenStream("seq\t$%d, $%d, $%d", node->getTemporaryPlace(), left->getTemporaryPlace(), right->getTemporaryPlace());
                    break;
                case BINARY_OP_GE:
                    CodeGenStream("sge\t$%d, $%d, $%d", node->getTemporaryPlace(), left->getTemporaryPlace(), right->getTemporaryPlace());
                    break;
                case BINARY_OP_LE:
                    CodeGenStream("sle\t$%d, $%d, $%d", node->getTemporaryPlace(), left->getTemporaryPlace(), right->getTemporaryPlace());
                    break;
                case BINARY_OP_NE:
                    CodeGenStream("sne\t$%d, $%d, $%d", node->getTemporaryPlace(), left->getTemporaryPlace(), right->getTemporaryPlace());
                    break;
                case BINARY_OP_GT:
                    CodeGenStream("sgt\t$%d, $%d, $%d", node->getTemporaryPlace(), left->getTemporaryPlace(), right->getTemporaryPlace());
                    break;
                case BINARY_OP_LT:
                    CodeGenStream("slt\t$%d, $%d, $%d", node->getTemporaryPlace(), left->getTemporaryPlace(), right->getTemporaryPlace());
                    break;
                case BINARY_OP_AND:
                    CodeGenStream("and\t$%d, $%d, $%d", node->getTemporaryPlace(), left->getTemporaryPlace(), right->getTemporaryPlace());
                    break;
                case BINARY_OP_OR:
                    CodeGenStream("or\t$%d, $%d, $%d", node->getTemporaryPlace(), left->getTemporaryPlace(), right->getTemporaryPlace());
                    break;
                default:
                    break;
            }
        }
        case UNARY_OPERATION:
        {
        }
        default:
            break;
    }
}

void genConStmt(AST_NODE *node){
    node->setTemporaryPlace(node->getDataType());
    switch(node->getConType()){
        case INTEGERC:
            CodeGenStream("li\t$%d, %d", node->getTemporaryPlace(), node->getConIntValue());
            break;
        case FLOATC:
            CodeGenStream("li\t$%d, %lf", node->getTemporaryPlace(), node->getConFloatValue());
            break;
        case STRINGC:
            DebugInfo(node, "Unhandle case in genExprRelatedNode:constValue, conType: %d", node->getConType());
            break;
        default:
            DebugInfo(node, "Unhandle case in genExprRelatedNode:constValue, conType: %d", node->getConType());
            break;
    }
}
