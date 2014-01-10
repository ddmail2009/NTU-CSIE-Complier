#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "Register.h"
#include "codeGen.h"
#include "symbolTable.h"
#include <time.h>
const char Binarycommand[][2][10] = {
    {"add", "add.s"},
    {"sub", "sub.s"},
    {"mul", "mul.s"},
    {"div", "div.s"},
    {"seq", "c.eq.s"},
    {"sge", "c.le.s"},
    {"sle", "c.le.s"},
    {"sne", "c.eq.s"},
    {"sgt", "c.lt.s"},
    {"slt", "c.lt.s"},
    {"and", "and"},
    {"or" , "or"}
};

RegisterSystem regSystem;

Register::Register(const char *name, DATA_TYPE type){
    strncpy(reg_name, name, 10);
    this->reg_type = type;
    this->dirty = false;
}

const char *Register::name() const{
    return reg_name;
}

const DATA_TYPE Register::type() const{
    return reg_type;
}

bool Register::isDirty(){
    return dirty;
}

void Register::operand(BINARY_OPERATOR op, const Register *leftFrom, const int value){
    Register *tmp = regSystem.getReg(INT_TYPE);
    tmp->load(value);
    this->operand(op, leftFrom, tmp);
}
void Register::operand(BINARY_OPERATOR op, const Register *leftFrom, const double value){
    Register *tmp = regSystem.getReg(FLOAT_TYPE);
    tmp->load(value);
    this->operand(op, leftFrom, tmp);
}
void Register::operand(BINARY_OPERATOR op, const Register *left, const Register *right){
    bool negate[] = {
        false, false, false, false,
        false, true, false, true,
        true, false, false, false
    };

    if(left->type() == FLOAT_TYPE && right->type() == INT_TYPE){
        Register *tmp = regSystem.getReg(FLOAT_TYPE);
        tmp->load(right);
        right = tmp;
    }
    if(left->type() == INT_TYPE && right->type() == FLOAT_TYPE){
        Register *tmp = regSystem.getReg(FLOAT_TYPE);
        tmp->load(left);
        left = tmp;
    }

    if(type() == INT_TYPE && (left->type() == FLOAT_TYPE || right->type() == FLOAT_TYPE)){
        static int branchIndex = 0;
        CodeGenStream("%s\t%s, %s", Binarycommand[op][FLOAT_TYPE], left->name(), right->name());
        CodeGenStream("bc1f _False%d", branchIndex);
        CodeGenStream("li\t%s, %d", name(), negate[op]);
        CodeGenStream("_False%d:", branchIndex);
        CodeGenStream("li\t%s, %d", name(), !negate[op]);
        branchIndex ++;
    } else 
        CodeGenStream("%s\t%s, %s, %s", Binarycommand[op][type()], name(), left->name(), right->name());
    this->dirty = true;
}

void Register::operand(UNARY_OPERATOR op, const Register *left, const Register *right){
    this->dirty = true;
}

void Register::load(int value){
    this->load((float) value);
}

void Register::load(double value){
    if(type() == INT_TYPE)
        CodeGenStream("li\t%s, %d", name(), (int) value);
    else if(type() == FLOAT_TYPE)
        CodeGenStream("li.s\t%s, %lf", name(), (double) value);

    this->dirty = true;
}

void Register::load(const Register *from){
    if(type() == INT_TYPE && from->type() == FLOAT_TYPE){
        Register *reg = regSystem.getReg(FLOAT_TYPE);
        CodeGenStream("mov.s\t%s, %s", reg->name(), from->name());
        CodeGenStream("cvt.w.s\t%s, %s", reg->name(), reg->name());
        CodeGenStream("mfc1\t%s, %s", name(), reg->name());
    } else if(type() == FLOAT_TYPE && from->type() == INT_TYPE){
        CodeGenStream("mtc1\t%s, %s", from->name(), name());
        CodeGenStream("cvt.s.w\t%s, %s", name(), name());
    } else if(type() == INT_TYPE){
        CodeGenStream("move\t%s, %s", name(), from->name());
    } else if(type() == FLOAT_TYPE){
        CodeGenStream("mov.s\t%s, %s", name(), from->name());
    }
    this->dirty = true;
}

void Register::load(const Address &addr, bool loadword){
    if(type() == FLOAT_TYPE){
        CodeGenStream("l.s\t%s, %s", name(), addr.getName());
    } else {
        char word[][3] = {"lw", "la"};
        int index = 0;
        if(addr.isLabel() && !loadword) index = 1;

        CodeGenStream("%s\t%s, %s", word[index], name(), addr.getName());
    }
    this->dirty = true;
}

void Register::save(){
    if(targetType == 0){
        ;
    } else {
        this->save(*(Address*)target);
    }
    this->dirty = false;
}

void Register::save(const Address &addr){
    if(type() == FLOAT_TYPE)
        CodeGenStream("s.s\t%s, %s", name(), addr.getName());
    else 
        CodeGenStream("sw\t%s, %s", name(), addr.getName());
}

void Register::setTarget(const AST_NODE *node){
    targetType = 0;
    target = node;
}
void Register::setTarget(const Address &addr){
    targetType = 1;
    target = &addr;
}
bool Register::fit(const AST_NODE *node) const{
    if(targetType != 0) return false;
    if(target != node) return false;
    return true;
}
bool Register::fit(const Address &addr) const{
    if(targetType != 1) return false;
    if(target != &addr) return false;
    return true;
}

//////////////////////////////////
// Address method 
//////////////////////////////////
Address::Address(Register *reg, int offset){
    this->reg = reg;
    this->offset = offset;
    this->setName();
    addrIsLabel = false;
}

Address::Address(const char *format, ...){
    va_list args;
    va_start(args, format);
    vsprintf(this->addrName, format, args);
    va_end(args);
    addrIsLabel = true;
}

Address Address::operator +(int i) const{
    return *this - (-i);
}

Address Address::operator -(int i) const{
    Address tmp = *this;
    tmp.offset -= i;
    tmp.setName();
    return tmp;
}

void Address::setName(){
    if(!isLabel()) sprintf(addrName, "%d(%s)", offset, reg->name());
}

//////////////////////////
// RegisterSystem method
/////////////////////////
RegisterSystem::RegisterSystem(){
    char tmp[10];
    srand(time(NULL));
    for(int i=0; i<8; i++){
        sprintf(tmp, "$s%d", i);
        calleeReg[i] = new Register(tmp);
        registers.push_back(calleeReg[i]);
    }

    for(int i=0; i<10; i++){
        sprintf(tmp, "$t%d", i);
        callerReg[i] = new Register(tmp);
        registers.push_back(callerReg[i]);
    }

    for(int i=0; i<30; i++){
        sprintf(tmp, "$f%d", i);
        floatReg[i] = new Register(tmp, FLOAT_TYPE);
        registers.push_back(floatReg[i]);
    }

    Register *v0 = new Register("$v0", INT_TYPE);
    registers.push_back(v0);
    Register *ra = new Register("$ra", INT_TYPE);
    registers.push_back(ra);
    Register *fp = new Register("$fp", INT_TYPE);
    registers.push_back(fp);
    Register *sp = new Register("$sp", INT_TYPE);
    registers.push_back(sp);
    Register *a0 = new Register("$a0", INT_TYPE);
    registers.push_back(a0);
}


Register *RegisterSystem::getReg(DATA_TYPE type, bool isCaller){
    if(type == FLOAT_TYPE){
        return floatReg[findVacant(floatReg, 30)];
    }
    else if(isCaller) 
        return callerReg[findVacant(callerReg, 10)];
    else 
        return calleeReg[findVacant(calleeReg, 8)];
}

Register *RegisterSystem::getReg(const char *str){
    for(std::vector<Register*>::iterator iter=registers.begin(); iter!=registers.end(); iter++){
        if(!strcmp((*iter)->name(), str)){
            return (*iter);
        }
    }
    fprintf(stderr, "couldn't find register named: %s\n", str);
}

int RegisterSystem::findVacant(Register *arr[], int size){
    // find clean register
    for(int i=0; i<size; i++)
        if(!arr[i]->isDirty())
            return i;

    // register full, save existing register and return random index
    int index = random()%size;
    arr[index]->save();
    return index;
}

//////////////////////////////
// AST NODE register method
/////////////////////////////
void AST_NODE::setRegister(Register *tmp, bool autoload){
    reg = tmp;
    if(type() == IDENTIFIER_NODE){
        if(autoload) reg->load(this->getSymbol()->getAddress());
        reg->setTarget(this->getSymbol()->getAddress());
    }
    else 
        reg->setTarget(this);
}

Register *AST_NODE::getTempReg(int option){
    bool isReset = option & RegReset;
    bool isCaller = option & RegforceCaller;
    bool isDisableload = option & RegDisableload;

    if(!isReset && reg != NULL && reg->fit(this)){;
    } else if(!isReset && this->type() == IDENTIFIER_NODE && regSystem.getFit(this->getSymbol()->getAddress()) != NULL){
        reg = regSystem.getFit(getSymbol()->getAddress());
        fprintf(stderr, "found register: %s in same symbol\n", reg->name());
    } else {
        Register *tmp = regSystem.getReg(getDataType(), isCaller);
        setRegister(tmp, !isDisableload);
        fprintf(stderr, "cant find any, alloc a new register: %s\n", reg->name());
    }
    return reg;
}

const char *AST_NODE::getTempName(){
    Register *reg = getTempReg();
    return reg->name();
}
