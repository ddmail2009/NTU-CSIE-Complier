#include <cstdio>
#include <cstring>
#include <cstdlib>

#include "header.h"
#include "gen-part.h"

const char* TEXT= ".text";
const char* DATA= ".data";
const char* WORD= ".word";
const char* SYSCALL = "syscall";
int reg_number = 7;
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
      SysCallParameter p(EXIT);
      genSysCall(&p);
    }
    else {
        CodeGenStream("jr\t$ra");
        CodeGenStream("%s", DATA);
    }
    CodeGenStream("_framesize_%s: %s %d", functionName, WORD, offset);
}

void genGlobalVariableWithInit(const CON_Type* c, const char* idName) {
  CodeGenStream("%s", DATA);
  if(c->const_type == INTEGERC) {
    CodeGenStream("_%s:\t%s %d", idName, WORD, c->const_u.intval);
  }
  else if(c->const_type == FLOATC) {
    CodeGenStream("_%s:\t%s %lf", idName, WORD, c->const_u.fval);
  }
  else {
    CodeGenStream("string%d: .asciiz \"%s\"", string_literal_number, c->const_u.sc);
    string_literal_number++;
  }
}

inline bool isCallerSaveRegister(int reg) {
  return (reg > 7 && reg < 16) || reg == 24 || reg == 25;
}

inline bool isCalleeSaveRegister(int reg) {
  return reg > 15 && reg < 24;
}

/* naive method */
int getReg() {
  if(reg_number <= 25) {
    return ++reg_number;
  }
  reg_number = 8;
  return reg_number;
}

/* In C--, we use double instead of float to implement FLOATC.
 *
 * NOTE: if write_string, PLEASE CALL genStringLiteral FIRST
 *
 */
void genSysCall(const SysCallParameter* information) {
  switch(information->type) {
    case PRINT_INT:
      CodeGenStream("#print_int system call");
      CodeGenStream("li\tv0 %d", information->type);
      CodeGenStream("la\t$a0 %d", information->val.ival);
      break;
    case PRINT_FLOAT:
      fprintf(stderr, "this case shouldn't be happened in C--\n");
      exit(1);
      break;
    case PRINT_DOUBLE:
      CodeGenStream("#print_double system call");
      CodeGenStream("li\tv0 %d", information->type);
      CodeGenStream("la\t$f12 %lf", information->val.fval);
      break;
    case PRINT_STRING:
      CodeGenStream("#print_string system call");
      CodeGenStream("li\tv0 %d", information->type);
      break;
    case READ_INT:
      CodeGenStream("#read_int system call");
      CodeGenStream("li\tv0 %d", information->type);
      break;
    case READ_FLOAT:
      fprintf(stderr, "this case shouldn't be happened in C--\n");
      exit(1);
      break;
    case READ_DOUBLE:
      CodeGenStream("#read_double system call");
      CodeGenStream("li\tv0 %d", information->type);
      break;
    case READ_STRING: 
      CodeGenStream("#read_string system call");
      CodeGenStream("li\tv0 %d", information->type);
      CodeGenStream("la\t$a0 string%d", string_literal_number - 1); // address of string to print
      break;
    case SBRK:
      CodeGenStream("#sbrk system call");
      CodeGenStream("li\tv0 %d", information->type);
      break;
    case EXIT:
      CodeGenStream("#exit system call");
      CodeGenStream("li\tv0 %d", information->type);
      break;
    default:
      fprintf(stderr, "unknowed type of SysCall\n");
      exit(1);
      break;
  }
  CodeGenStream("%s", SYSCALL);
}
