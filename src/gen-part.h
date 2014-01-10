#ifndef _Gen_Part_H
#define _Gen_Part_H

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


void genSyscall(const SystemCallCode &code, Register *value);
int gen_head(const char *name);
void gen_prologue(const char *functionName);
void gen_epilogue(const char *functionName);


#endif
