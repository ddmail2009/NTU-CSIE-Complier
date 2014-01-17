#ifndef __Register_H
#define __Register_H

#include <cstdlib>
#include <vector>
#include <map>
#include "header.h"
#include "Address.h"
#include "gen-part.h"

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
        void operand(BINARY_OPERATOR op, Register *left, Register *right);
        void operand(UNARY_OPERATOR op, const Register *from);

        void branch(const char *format, ...) const;
        void branch2(const char *format, ...) const;

        void load(int value);
        void load(double value);
        void load(const char *label);
        void load(Register *from);
        void load(const Register *from);
        void load(const Address &addr);

        // save to the target, if it is a ast_node, it's a temporary value.
        void save();
        void save(const Address &addr);

        void setTarget(const AST_NODE *node);
        void setTarget(const Address addr);
        bool fit(const AST_NODE *node) const;
        bool fit(const Address &addr) const;

        Address *targetAddr;
        bool dirty, modified;
    private:
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

        void globalInitRoutine(const char *start, const char *end);

        void getStartTag(char *outStr) const;
        void getEndTag(char *outStr) const;
        void getTag(const char *prefix, char *outStr) const;
    private:
        Register *sp, *fp;

        char funcName[256];
        int totalOffset, paramOffset;
        std::map<AST_NODE*, Address*> arrVariable;
        std::map<const char*, const char*> strConst;
        std::map<SymbolTableEntry*, Address*> localVarible;
        std::map<SymbolTableEntry*, Address*> globalVariable;
        std::vector<char **> initroutine;
};

// store all the register
class RegisterSystem{
    public:
        RegisterSystem();
        Register *getReg(DATA_TYPE type, bool isCaller = false);
        Register *getReg(const char *format, ...);

        Register *getFit(const Address &addr){
            for(std::vector<int>::size_type i=0; i!=registers.size(); i++)
                if(registers[i]->fit(addr)) return registers[i];
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
            for(std::vector<int>::size_type i=0; i!=callerReg.size(); i++)
                callerReg[i]->clear();
            for(std::vector<int>::size_type i=0; i!=floatCaller.size(); i++)
                floatCaller[i]->clear();
        }

        std::vector<Register*> getCallee(){
            std::vector<Register*> tmp;
            for(std::vector<int>::size_type i=0; i!=calleeReg.size(); i++)
                tmp.push_back(calleeReg[i]);
            for(std::vector<int>::size_type i=0; i!=floatCallee.size(); i++)
                tmp.push_back(floatCallee[i]);
            return tmp;
        }

        void lock(Register *reg, bool l){
            lockMap[reg] = l;
        }
    private:
        bool isLocked(Register *reg){
            return lockMap[reg];
        }

        int findVacant(std::vector<Register*> v);
        std::map<Register*, bool> lockMap;
        std::vector<Register*> registers;
        std::vector<Register*> calleeReg;
        std::vector<Register*> callerReg;
        std::vector<Register*> floatCaller;
        std::vector<Register*> floatCallee;
};

#endif
