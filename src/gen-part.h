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

void CodeGenStream(const char *format, ...);
void DebugInfo(AST_NODE *node, const char *format, ...);
void genSyscall(const SystemCallCode &code, Register *value);
#endif
