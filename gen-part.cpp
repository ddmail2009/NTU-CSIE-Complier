#include "header.h"
#include <cstdio>
#include <cstring>
const char* TEXT= ".text";
const char* DATA= ".data";
const char* WORD= ".word";

extern void CodeGenStream(const char *format, ...);
int reg_number = 0;

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
    CodeGenStream("lw\t$2, _framesize_%s", functionName);
    CodeGenStream("sub\t$sp,$sp,$2");

    /* pushNewAR(); */ 

    CodeGenStream("_begin_%s:", functionName);
}

void gen_epilogue(const char *functionName, int offset) {
    CodeGenStream("_end_%s:", functionName);
    CodeGenStream("lw\t$ra, 4($fp)"); // restore return address
    CodeGenStream("add\t$sp, $fp, 4"); // pop AR
    CodeGenStream("lw\t$fp, 0($fp)"); // restore caller (old) $fp
    if (strcmp(functionName, "main") == 0) {
        CodeGenStream("li\t$v0, 10");
        CodeGenStream("syscall");
    }
    else {
        CodeGenStream("jr\t$ra");
        CodeGenStream("%s", DATA);
    }
    CodeGenStream("_framesize_%s: %s %d", functionName, WORD, offset);
}

void genVariableWithInit(bool first, C_type c, const char* name, int a, double d) {
    if(first) {
        CodeGenStream("%s", DATA);
    }
    if(c == INTEGERC) {
        CodeGenStream("_%s:\t%s %d", name, WORD, a);
    }
    else if(c == FLOATC) {
        CodeGenStream("_%s:\t%s %lf", name, WORD, d);
    }
}

int getReg() {
    return 1;
}
