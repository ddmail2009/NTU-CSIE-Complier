#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "header.h"
#include "symbolTable.h"
#include "codeGen.h"
#include "gen-part.h"
#include "Register.h"

extern RegisterSystem regSystem;


/* Utility Function */

void DebugIndent(int n){
#ifdef DEBUG
    for(int i=0; i<n; i++)
        fprintf(stderr, "\t");
#endif
}

void DebugInfo(const char *format, ...){
#ifdef DEBUG
    va_list args;
    va_start(args, format);
    fprintf(stderr, "\e[31m");
    vfprintf(stderr, format, args);
    fprintf(stderr, "\e[m\n");
    va_end(args);
#endif
}

void DebugInfo(AST_NODE *node, const char *format, ...){
#ifdef DEBUG
    fprintf(stderr, "\e[32mline[%d]: \e[m", node->linenumber);
    va_list args;
    va_start(args, format);
    DebugInfo(format, args);
    va_end(args);
#endif
}

void ASSERT(bool condition, const char *format, ...){
    if(!condition){
        DebugIndent(1);
        va_list args;
        va_start(args, format);
        DebugInfo(format, args);
        va_end(args);
    }
    assert(condition);
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
                Register *f0 = regSystem.getReg("$f0");
                value->load(f0);
                break;
            }
        case EXIT:
            CodeGenStream("syscall");
            break;
        default:
            CodeGenStream("#Unhandled case in our C-- project, error syscall code");
    }
}
