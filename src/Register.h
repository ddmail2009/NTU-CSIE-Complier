#ifndef __Register_H
#define __Register_H

#include <cstdlib>
#include <vector>
#include <map>
#include "header.h"
#include "Address.h"

class Register{
    public:
        Register(const char *n, DATA_TYPE t = INT_TYPE):
            targetAddr(NULL),
            dirty(false),
            modified(false),
            reg_type(t),
            target(NULL),
            targetType(0) {
                strncpy(reg_name, n, 10);
                targetAddr = new Address("");
        }
        void clear();

        const char *name() const;
        const DATA_TYPE type() const;
        bool isDirty() const;
        bool isModified() const;

        void operand(BINARY_OPERATOR op, const Register *left, const int value);
        void operand(BINARY_OPERATOR op, const Register *left, const double value);
        void operand(BINARY_OPERATOR op, const Register *left, const Register *right);
        void operand(UNARY_OPERATOR op, const Register *from);

        void branch(const char *format, ...) const;
        void branch2(const char *format, ...) const;

        void load(int value);
        void load(double value);
        void load(const char *label);
        void load(const Register *from);
        void load(const Address &addr, bool loadWord = true);

        // save to the target, if it is a ast_node, it's a temporary value.
        void save();
        void save(const Address &addr);

        void setTarget(const AST_NODE *node);
        void setTarget(const Address addr);
        bool fit(const AST_NODE *node) const;
        bool fit(const Address &addr) const;

        Address *targetAddr;
    private:
        bool dirty, modified;
        DATA_TYPE reg_type;
        const void *target;
        bool targetType;
        char reg_name[10];
};

// control the offset and gen prologue and epilogue
class ARSystem{
    public:
        ARSystem();
        void clear();
        void addVariable(SymbolTableEntry *entry, bool isParam = false);
        void addVariable(const char *str);
        Address getAddress(AST_NODE *node);
        void prologue(const char *funcName);
        void epilogue();

        void globleInitRoutine(const char *start, const char *end);

        void getStartTag(char *outStr) const;
        void getEndTag(char *outStr) const;
        void getTag(const char *prefix, char *outStr) const;
    private:
        Register *sp, *fp;

        char funcName[256];
        int totalOffset, paramOffset;
        std::map<const char*, const char*> strConst;
        std::map<SymbolTableEntry*, Address*> localVarible;
        std::map<SymbolTableEntry*, Address*> globleVariable;
        std::vector<char **> initroutine;
};

// store all the register
class RegisterSystem{
    public:
        RegisterSystem();
        Register *getReg(DATA_TYPE type, bool isCaller = false);
        Register *getReg(const char *format, ...);

        Register *getFit(const Address &addr){
            for(int i=0; i<8; i++)
                if(calleeReg[i]->fit(addr)) return calleeReg[i];
            for(int i=0; i<8; i++)
                if(callerReg[i]->fit(addr)) return callerReg[i];
            for(int i=0; i<30; i++)
                if(floatReg[i]->fit(addr)) return floatReg[i];
            return NULL;
        }

        void saveAndClear(){
            for(std::vector<Register*>::iterator iter=registers.begin(); iter!=registers.end(); iter++){
                (*iter)->save();
                (*iter)->clear();
            }
        }

        // clear all the register
        void clear(){
            for(std::vector<Register*>::iterator iter=registers.begin(); iter!=registers.end(); iter++){
                (*iter)->clear();
            }
        }

        // clear all recored but callee register, happened after jump back
        void clearRegRecord(){
            for(int i=0; i<10; i++){
                callerReg[i]->clear();
            }
            for(int i=0; i<30; i++){
                floatReg[i]->clear();
            }
        }
    private:
        int findVacant(Register *arr[], int size);

        std::vector<Register*> registers;
        Register *calleeReg[8];
        Register *callerReg[10];
        Register *floatReg[30];
};

#endif
