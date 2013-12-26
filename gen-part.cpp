#include "header.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>

const char* TEXT= ".text";
const char* DATA= ".data";
const char* WORD= ".word";
const char* SYSCALL = "syscall";

extern void CodeGenStream(const char *format, ...);
int reg_number = 7;
int string_literal_number = 0;

enum SystemCallCode {
  PRINT_INT = 1, // $a0 = integer
  PRINT_FLOAT,   // $f12 = float
  PRINT_DOUBLE,  // $f12 = double
  PRINT_STRING,  // $a0 = string
  READ_INT,      // integer (in $v0)
  READ_FLOAT,    // float (in $f0)
  READ_DOUBLE,   // double (in $f0)
  READ_STRING,   // $a0 = buffer, $a1 = length
  SBRK,          // $a0 = amount     address(in $v0)
  EXIT,
};
// this enum does nothing
enum ReadSysCallType {
  INT,
  //FLOAT, // C-- only have float, but we use double instead
  DOUBLE,
  STRING,
  CHAR
};

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

void genStringLiteral(const char* str) {
  CodeGenStream("%s", DATA);
  CodeGenStream("string%d: .asciiz \"%s\"", string_literal_number, str);
  string_literal_number++;
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

void loadSysCall(enum SystemCallCode which) {
  switch(which) {
    case PRINT_INT:
      CodeGenStream("#print_int system call");
      CodeGenStream("li\tv0 %d", PRINT_INT);
      break;
    case PRINT_FLOAT:
      fprintf(stderr, "this case shouldn't be happened in C--\n");
      exit(1);
      break;
    case PRINT_DOUBLE:
      CodeGenStream("#print_double system call");
      CodeGenStream("li\tv0 %d", PRINT_DOUBLE);
      break;
    case PRINT_STRING:
      CodeGenStream("#print_string system call");
      CodeGenStream("li\tv0 %d", PRINT_STRING);
      break;
    case READ_INT:
      break;
    case READ_FLOAT:
      break;
    case READ_DOUBLE:
      break;
    case READ_STRING:
      break;
    case SBRK:
      break;
    case EXIT:
      break;
  }
}

void genWriteStringSysCall(int stringLiteralNum) {
  loadSysCall(PRINT_STRING);
  CodeGenStream("la\t$a0 string%d", stringLiteralNum); // address of string to print
  CodeGenStream("%s", SYSCALL);
}

void genWriteIntSysCall(int val) {
  loadSysCall(PRINT_INT);
  CodeGenStream("la\t$a0 %d", val);
  CodeGenStream("%s", SYSCALL);
}

void genWriteDoubleSysCall(double val) {
  loadSysCall(PRINT_DOUBLE);
  CodeGenStream("la\t$f12 %lf", val);
  CodeGenStream("%s", SYSCALL);
}

void genReadSysCall() {
}
