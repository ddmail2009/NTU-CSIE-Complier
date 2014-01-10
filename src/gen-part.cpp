#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "header.h"
#include "symbolTable.h"
#include "codeGen.h"
#include "gen-part.h"
#include "Register.h"

const char* TEXT= ".text";
const char* DATA= ".data";
const char* WORD= ".word";
const char* SYSCALL = "syscall";

extern void CodeGenStream(const char *format, ...);
extern RegisterSystem regSystem;

int gen_head(const char *name) {
    CodeGenStream("%s", TEXT);
    CodeGenStream("%s:", name);
    return 0;
}

void gen_prologue(const char *functionName) {
    Register *ra = regSystem.getReg("$ra");
    Register *sp = regSystem.getReg("$sp");
    Register *fp = regSystem.getReg("$fp");
    Address address(sp);

    ra->save(address);
    fp->save(address - 4);
    fp->operand(BINARY_OP_ADD, sp, -4);
    sp->operand(BINARY_OP_ADD, sp, -8);
    Register *tmp = regSystem.getReg(INT_TYPE);
    tmp->load(Address("_framesize_%s", functionName));
    sp->operand(BINARY_OP_SUB, sp, tmp);

    /* naive save Caller Save Register */
    regSystem.saveCalleeReg();
    CodeGenStream("_begin_%s:", functionName);
}

// offset is at least 32 bytes (at least 8 caller save registers)
void gen_epilogue(const char *functionName) {
    CodeGenStream("# epilogue sequence");
    CodeGenStream("_end_%s:", functionName);
    /* naive restore Caller Save Register */
    regSystem.restoreCalleeReg();

    Register *ra = regSystem.getReg("$ra");
    Register *sp = regSystem.getReg("$sp");
    Register *fp = regSystem.getReg("$fp");
    Address address(fp);

    ra->load(address + 4);
    sp->operand(BINARY_OP_ADD, fp, 4);
    fp->load(address);
    if (strcmp(functionName, "main") == 0) {
        genSyscall(EXIT, NULL);
    }
    else {
        CodeGenStream("jr\t$ra");
    }
}

void genSyscall(const SystemCallCode &code, Register *value){
    Register *v0 = regSystem.getReg("$v0");
    v0->load((int)code);

    switch(code){
        case PRINT_INT:
            {
                Register *a0 = regSystem.getReg("$a0");
                a0->load(value);
                CodeGenStream("syscall");
                break;
            }
        case PRINT_FLOAT:
            {
                Register *f12 = regSystem.getReg("$f12");
                f12->load(value);
                CodeGenStream("syscall");
                break;
            }
        case PRINT_STRING:
            {
                Register *a0 = regSystem.getReg("$a0");
                a0->load(value);
                CodeGenStream("syscall");
                break;
            }
        case READ_INT:
            {
                CodeGenStream("syscall");
                value->load(v0);
                break;
            }
        case READ_FLOAT:
            {
                CodeGenStream("syscall");
                Register *f0 = regSystem.getReg("f0");
                value->load(f0);
                break;
            }
        case EXIT:
            break;
        default:
            CodeGenStream("#Unhandled case in our C-- project, error syscall code");
    }
}

/* In C--, we use double instead of float to implement FLOATC.
 *
 * NOTE: if write_string, PLEASE CALL genGlobalVariableWithInit FIRST
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
*/

