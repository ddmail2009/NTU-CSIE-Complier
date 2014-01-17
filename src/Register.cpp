#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
#include <cassert>

#include "Register.h"
#include "codeGen.h"
#include "symbolTable.h"
#include "gen-part.h"
#include <ctime>

extern ARSystem ar;
extern RegisterSystem regSystem;

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
bool BinaryNegate[] = {
    false, false, false, false,
    false, true, false, true,
    true, false, false, false
};

void Register::clear(){
    modified = false;
    dirty = false;
    target = NULL;
    targetAddr = new Address("");
    targetType = 0;
}

const char *Register::name() const{
    return reg_name;
}

const DATA_TYPE Register::type() const{
    return reg_type;
}

bool Register::isDirty() const{
    return dirty;
}

bool Register::isModified() const{
    return modified;
}

void Register::operand(BINARY_OPERATOR op, const Register *leftFrom, const int value){
    if(op == BINARY_OP_ADD){
        CodeGenStream("addi\t%s, %s, %d", name(), leftFrom->name(), value);
    } else if(op == BINARY_OP_SUB){
        CodeGenStream("addi\t%s, %s, %d", name(), leftFrom->name(), -value);
    } else {
        Register *tmp = regSystem.getReg(INT_TYPE, RegforceCaller);
        tmp->load(value);
        this->operand(op, leftFrom, tmp);
    }
    this->modified = true;
}
void Register::operand(BINARY_OPERATOR op, const Register *leftFrom, const double value){
    Register *tmp = regSystem.getReg(FLOAT_TYPE, RegforceCaller);
    tmp->load(value);
    this->operand(op, leftFrom, tmp);
}
void Register::operand(BINARY_OPERATOR op, const Register *left, const Register *right){
    if(left->type() == FLOAT_TYPE && right->type() == INT_TYPE){
        Register *tmp = regSystem.getReg(FLOAT_TYPE, RegforceCaller);
        tmp->load(right);
        right = tmp;
    }
    if(left->type() == INT_TYPE && right->type() == FLOAT_TYPE){
        Register *tmp = regSystem.getReg(FLOAT_TYPE, RegforceCaller);
        tmp->load(left);
        left = tmp;
    }
    if(op > BINARY_OP_DIV && (left->type() == FLOAT_TYPE || right->type() == FLOAT_TYPE)){
        static int branchIndex = 0;
        CodeGenStream("%s\t%s, %s", Binarycommand[op][FLOAT_TYPE], left->name(), right->name());
        CodeGenStream("bc1f _False%d", branchIndex);
        CodeGenStream("li\t%s, %d", name(), !BinaryNegate[op]);
        CodeGenStream("j\tEnd_False%d", branchIndex);
        CodeGenStream("_False%d:", branchIndex);
        CodeGenStream("li\t%s, %d", name(), BinaryNegate[op]);
        CodeGenStream("End_False%d:", branchIndex);
        branchIndex ++;
    } else 
        CodeGenStream("%s\t%s, %s, %s", Binarycommand[op][type()], name(), left->name(), right->name());
    this->dirty = true;
    this->modified = true;
}
void Register::operand(BINARY_OPERATOR op,  Register *left, Register *right){
    if(left->type() == FLOAT_TYPE && right->type() == INT_TYPE){
        Register *tmp = regSystem.getReg(FLOAT_TYPE, RegforceCaller);
        tmp->load(right);
        right = tmp;
    }
    if(left->type() == INT_TYPE && right->type() == FLOAT_TYPE){
        Register *tmp = regSystem.getReg(FLOAT_TYPE, RegforceCaller);
        tmp->load(left);
        left = tmp;
    }
    if(op > BINARY_OP_DIV && (left->type() == FLOAT_TYPE || right->type() == FLOAT_TYPE)){
        static int branchIndex = 0;
        CodeGenStream("%s\t%s, %s", Binarycommand[op][FLOAT_TYPE], left->name(), right->name());
        CodeGenStream("bc1f _False%d", branchIndex);
        CodeGenStream("li\t%s, %d", name(), !BinaryNegate[op]);
        CodeGenStream("j\tEnd_False%d", branchIndex);
        CodeGenStream("_False%d:", branchIndex);
        CodeGenStream("li\t%s, %d", name(), BinaryNegate[op]);
        CodeGenStream("End_False%d:", branchIndex);
        branchIndex ++;
    } else 
        CodeGenStream("%s\t%s, %s, %s", Binarycommand[op][type()], name(), left->name(), right->name());
    this->dirty = true;
    this->modified = true;
    if(left->targetAddr->_volatile) left->clear();
    if(right->targetAddr->_volatile) right->clear();
}
void Register::operand(UNARY_OPERATOR op, const Register *from){
    // TODO, have to consider float case
    switch(op){
        case UNARY_OP_POSITIVE:
            CodeGenStream("abs\t%s, %s", name(), from->name());
            break;
        case UNARY_OP_NEGATIVE:
            // -a = ~a + 1
            CodeGenStream("not\t%s, %s", name(), from->name());
            operand(BINARY_OP_ADD, this, 1);
            break;
        case UNARY_OP_LOGICAL_NEGATION:
            static int branchIndex = 0;
            CodeGenStream("beqz\t%s, _TrueNot%d", from->name(), branchIndex);
            load(0);
            CodeGenStream("j\t_EndNot%d", branchIndex);
            CodeGenStream("_TrueNot%d:", branchIndex);
            load(1);
            CodeGenStream("_EndNot%d:", branchIndex);
            branchIndex ++;
            break;
        default:
            ;
    }
    this->dirty = true;
    this->modified = true;
}

void Register::branch(const char *format, ...) const{
    char label[1024];
    va_list args;
    va_start(args, format);
    vsprintf(label, format, args);
    va_end(args);

    const char *regName = name();
    if(type() == FLOAT_TYPE){
        Register *tmp = regSystem.getReg(INT_TYPE, RegforceCaller);
        tmp->load(this);
        regName = tmp->name();
    }
    CodeGenStream("beqz\t%s, %s", regName, label);
}

void Register::branch2(const char *format, ...) const{
    char label[1024];
    va_list args;
    va_start(args, format);
    vsprintf(label, format, args);
    va_end(args);

    const char *regName = name();
    if(type() == FLOAT_TYPE){
        Register *tmp = regSystem.getReg(INT_TYPE, RegforceCaller);
        tmp->load(this);
        regName = tmp->name();
    }
    CodeGenStream("bne\t%s, $zero, %s", regName, label);
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
void Register::load(const char *label){
    if(type() == INT_TYPE)
        CodeGenStream("la\t%s, %s", name(), label);
    else 
        CodeGenStream("la\t%s, %s", name(), label);
    this->dirty = true;
}
void Register::load(Register *from){
    if(from == this) return;
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
    this->modified = true;

    if(from->targetAddr->_volatile)
        from->clear();
}
void Register::load(const Register *from){
    if(from == this) return;
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
    this->modified = true;

}
void Register::load(const Address &addr){
    ASSERT(&addr != NULL, "error: register: %s load empty addr", name());

    if(addr.loadType == LOADWORD){
        if(type() == INT_TYPE)
            CodeGenStream("lw\t%s, %s", name(), addr.getName());
        else 
            CodeGenStream("l.s\t%s, %s", name(), addr.getName());
    } 
    else if(addr.loadType == LOADADDR){
        if(type() == INT_TYPE){
            DebugInfo("addr.hasReg: %d", addr.hasReg());
            if(addr.hasReg()){
                int offset = addr.getOffset();
                CodeGenStream("move\t%s, %s", name(), addr.getRegName());
                if(offset != 0)operand(BINARY_OP_ADD, this, offset);
            } else {
                CodeGenStream("la\t%s, %s", name(), addr.getName());
            }
        }
        else 
            ASSERT(0, "error: can't use float register %s load addr from %s", name(), addr.getName());
    }
    this->dirty = true;
}

void Register::save(){
    if(targetType == 0){
        //Temporary value
        ;
    } else {
        //local variable
        if(this->modified == true){
            this->save(*targetAddr);
            this->modified = false;
        }
    }
    this->dirty = false;
}
void Register::save(const Address &addr){
#ifdef DEBUG
    fprintf(stderr, "\t\t\t\t\e[35mregister '%s' save to addr: %s\e[m\n", name(), addr.getName());
#endif
    if(type() == FLOAT_TYPE)
        CodeGenStream("s.s\t%s, %s", name(), addr.getName());
    else
        CodeGenStream("sw\t%s, %s", name(), addr.getName());

    if(targetAddr->_volatile == true) 
        this->clear();
}

void Register::setTarget(const AST_NODE *node){
    if(targetType == 1) save(*targetAddr);
    targetType = 0;
    target = node;
}
void Register::setTarget(const Address addr){
    targetType = 1;
    *targetAddr = addr;
}
bool Register::fit(const AST_NODE *node) const{
    if(targetType != 0) return false;
    if(target != node) return false;
    return true;
}
bool Register::fit(const Address &addr) const{
    if(targetType != 1) return false;
    if(*targetAddr == addr) return true;
    else return false;
}

//////////////////////////
// RegisterSystem method
/////////////////////////
RegisterSystem::RegisterSystem(){
    char tmp[10];
    srand(time(NULL));
    for(int i=0; i<8; i++){
        sprintf(tmp, "$s%d", i);
        Register *reg = new Register(tmp, INT_TYPE);
        calleeReg.push_back(reg);
        registers.push_back(reg);
    }

    for(int i=0; i<10; i++){
        sprintf(tmp, "$t%d", i);
        Register *reg = new Register(tmp, INT_TYPE);
        callerReg.push_back(reg);
        registers.push_back(reg);
    }

    for(int i=1; i<30; i++){
        sprintf(tmp, "$f%d", i);
        Register *reg = new Register(tmp, FLOAT_TYPE);
        if(i < 15) floatCallee.push_back(reg);
        else floatCaller.push_back(reg);
        registers.push_back(reg);
    }
    Register *v0 = new Register("$v0", INT_TYPE);
    registers.push_back(v0);
    Register *f0 = new Register("$f0", FLOAT_TYPE);
    registers.push_back(f0);
    Register *ra = new Register("$ra", INT_TYPE);
    registers.push_back(ra);
    Register *fp = new Register("$fp", INT_TYPE);
    registers.push_back(fp);
    Register *sp = new Register("$sp", INT_TYPE);
    registers.push_back(sp);
    Register *a0 = new Register("$a0", INT_TYPE);
    registers.push_back(a0);
    Register *zero = new Register("$zero", INT_TYPE);
    registers.push_back(zero);

    for(std::vector<int>::size_type i=0; i!=registers.size(); i++)
        lockMap[registers[i]] = false;
}

Register *RegisterSystem::getReg(DATA_TYPE type, bool isCaller) {
    if(isCaller){
        if(type == FLOAT_TYPE)
            return floatCaller[findVacant(floatCaller)];
        else
            return callerReg[findVacant(callerReg)];
    }
    else {
        if(type == FLOAT_TYPE)
            return floatCallee[findVacant(floatCallee)];
        else
            return calleeReg[findVacant(calleeReg)];
    }
}

Register *RegisterSystem::getReg(const char *format, ...) {
    char str[1024];
    va_list args;
    va_start(args, format);
    vsprintf(str, format, args);
    va_end(args);

    for(std::vector<Register*>::iterator iter=registers.begin(); iter!=registers.end(); iter++){
        if(!strcmp((*iter)->name(), str)){
            return (*iter);
        }
    }
    fprintf(stderr, "couldn't find register named: %s\n", str);
    return NULL;
}

int RegisterSystem::findVacant(std::vector<Register*> v){
    // find clean register
    for(std::vector<int>::size_type i=0; i!=v.size(); i++)
        if(!v[i]->isDirty() && !isLocked(v[i])) return i;

    // find not modified register
    for(std::vector<int>::size_type i=0; i!=v.size(); i++)
        if(!v[i]->isModified() && !isLocked(v[i])) return i;

    // register full, save existing register and return random index
    while(1){
        int index = random()%v.size();
        if(lockMap[&(*v[index])] == true) {
            DebugInfo("lock reg: %s found, find another", v[index]->name());
            continue;
        }
        v[index]->save();
        v[index]->clear();
        return index;
    }
}

////////////////////////////////
// ARSystem method
////////////////////////////////
ARSystem::ARSystem(){
    sp = regSystem.getReg("$sp");
    fp = regSystem.getReg("$fp");

    clear();
}
void ARSystem::clear(){
    for(std::map<SymbolTableEntry*, Address*>::iterator iter=localVarible.begin(); iter!=localVarible.end(); iter++){
        delete iter->second;
    }
    localVarible.clear();
    totalOffset = 0;
    paramOffset = 8;
}

void ARSystem::addVariable(const char *str){
    // check if declared before
    for(std::map<const char*, const char *>::iterator iter=strConst.begin(); iter!=strConst.end(); iter++){
        if(!strcmp(str, iter->first)) return;
    }

    char *stringName = new char[1024];
    getTag("_STR", stringName);
    strConst[str] = stringName;

    // declare the const str
    CodeGenStream(".data");
    CodeGenStream("%s: .asciiz %s", stringName, str);

    CodeGenStream(".text");
}
void ARSystem::addVariable(SymbolTableEntry *entry, bool isParam){
    // global variable, store in global data field
    if(entry->nestingLevel == 0) {
        globalVariable[entry] = new Address("_%s", entry->name);
        if(entry->attribute->getTypeDes()->kind == ARRAY_TYPE_DESCRIPTOR)
            globalVariable[entry]->loadType = LOADADDR;
    }
    // parameter, store in caller's ar field
    else if(isParam){
        Address *addr = new Address(fp);
        *addr = *addr + paramOffset;
        localVarible[entry] = addr;

        paramOffset += 4;
        DebugInfo("symbol %s in %s\n", entry->name, localVarible[entry]->getName());
    }
    // local variable, store in local ar field
    else {
        TypeDescriptor *type = entry->attribute->getTypeDes();
        totalOffset += type->size();

        Address *addr = new Address(fp);
        *addr = *addr - totalOffset;
        localVarible[entry] = addr;
        DebugInfo("symbol %s in %s\n", entry->name, localVarible[entry]->getName());
    }
}

Address ARSystem::getAddress(AST_NODE *node){
    //if(node in addressMap) return addressMap[node];
    //else allocNewAddress(node);
    if(node->type() == CONST_VALUE_NODE && node->getConType() == STRINGC){
        for(std::map<const char*, const char*>::iterator iter=strConst.begin(); iter!=strConst.end(); iter++){
            if(!strcmp(node->getCharPtrValue(), iter->first)){
                Address addr(iter->second);
                addr.loadType = LOADADDR;
                return addr;
            }
        }
    } else if(node->type() == IDENTIFIER_NODE){
        SymbolTableEntry *entry = node->getSymbol();
        TypeDescriptor *type = entry->attribute->getTypeDes();
        Address addr("NULL");

        // try to get ID from the declared address
        if(entry->nestingLevel == 0)
            addr = *globalVariable[entry];
        else
            addr = *localVarible[entry];

        // if it is a declaration return base address
        if(node->parent->type() == DECLARATION_NODE || node->parent->type() ==  PARAM_LIST_NODE) 
            return addr;
        // else calculate the offset for array
        else if(node->getIDKind() == ARRAY_ID){
            if(arrVariable[node] != NULL) return *arrVariable[node];

            Register *offset = regSystem.getReg(INT_TYPE);
            DebugInfo("baseAddr of %s is %s, offset Reg: %s", node->getIDName(), addr.getName(), offset->name());
            regSystem.lock(offset, true);
            AST_NODE *tmp = node->child;
            int paramCount = 0;
            for(int i=0; i<type->getDimension(); i++){
                DebugInfo("genGeneral");
                if(tmp) {
                    paramCount++;
                    genGeneralNode(tmp);
                    regSystem.lock(tmp->getTempReg(), true);
                    if(i==0) offset->load(tmp->getTempReg());
                    else {
                        offset->operand(BINARY_OP_MUL, offset, type->getArrayDimensionSize(i));
                        offset->operand(BINARY_OP_ADD, offset, tmp->getTempReg());
                    }
                    regSystem.lock(tmp->getTempReg(), false);
                    tmp = tmp->rightSibling;
                }
                else
                    offset->operand(BINARY_OP_MUL, offset, type->getArrayDimensionSize(i));
            }
            offset->operand(BINARY_OP_MUL, offset, 4);

            Register *finalReg = regSystem.getReg(INT_TYPE, RegforceCaller);

            // If it is declared function parameter
            if(addr.hasReg() && addr.getOffset() > 0) 
                addr.loadType = LOADWORD;
            else 
                addr.loadType = LOADADDR;

            DebugInfo("paramDimension: %d", paramCount);
            finalReg->load(addr);
            finalReg->operand(BINARY_OP_ADD, finalReg, offset);
            regSystem.lock(offset, false);

            arrVariable[node] = new Address(finalReg);
            arrVariable[node]->_volatile = true;

            if(paramCount != type->getDimension())
                arrVariable[node]->loadType = LOADADDR;
            return *arrVariable[node];
        }
        return addr;

        if(!strcmp("NULL", addr.getName()))
            DebugInfo("Error, Can't find address for node@%d", node->linenumber);
    }

    Address *addr = new Address("NULL");
    return *addr;
}

void ARSystem::prologue(const char* funcName){
    strcpy(this->funcName, funcName);

    CodeGenStream(".text");
    char startag[1024];
    getStartTag(startag);
    CodeGenStream("%s:", startag);

    Register *ra = regSystem.getReg("$ra");
    Address address(sp);

    ra->save(address);
    fp->save(address - 4);
    fp->operand(BINARY_OP_ADD, sp, -4);
    sp->operand(BINARY_OP_ADD, sp, -8);
    Register *tmp = regSystem.getReg(INT_TYPE, RegforceCaller);
    tmp->load(Address("_framesize_%s", funcName));
    sp->operand(BINARY_OP_SUB, sp, tmp);

    /* naive save Caller Save Register */
    std::vector<Register*> callee = regSystem.getCallee();
    for(std::vector<int>::size_type i=0; i!=callee.size(); i++)
        callee[i]->save(Address(fp) - 4*i - 4);

    CodeGenStream("_begin_%s:", funcName);
    if(!strcmp(funcName, "main")){
        for(std::vector<char **>::iterator iter=initroutine.begin(); iter!=initroutine.end(); iter++){
            CodeGenStream("j %s", (*iter)[0]);
            CodeGenStream("%s:", (*iter)[1]);
        }
        initroutine.clear();
    }
    totalOffset = 4*callee.size();
    regSystem.clearRegRecord();
}
void ARSystem::epilogue(){
    CodeGenStream("# epilogue sequence");
    char endTag[1024];
    getEndTag(endTag);
    CodeGenStream("%s:", endTag);
    /* aive restore Caller Save Register */
    std::vector<Register*> callee = regSystem.getCallee();
    for(std::vector<int>::size_type i=0; i!=callee.size(); i++)
        callee[i]->load(Address(fp) - 4*i - 4);

    Register *ra = regSystem.getReg("$ra");
    Address address(fp);

    ra->load(address + 4);
    sp->operand(BINARY_OP_ADD, fp, 4);
    fp->load(address);
    if (strcmp(funcName, "main") == 0)
        genSyscall(EXIT, NULL);
    else
        CodeGenStream("jr\t$ra");

    CodeGenStream(".data");
    CodeGenStream("_framesize_%s: .word %d", funcName, totalOffset);

    clear();
    regSystem.clear();
}

void ARSystem::globalInitRoutine(const char *start, const char *end){
    char **routine = new char*[2];
    routine[0] = strdup(start);
    routine[1] = strdup(end);
    initroutine.push_back(routine);
}

void ARSystem::getStartTag(char *outStr) const{
    sprintf(outStr, "%s", funcName);
}
void ARSystem::getEndTag(char *outStr) const{
    sprintf(outStr, "_end_%s", funcName);
}
void ARSystem::getTag(const char* prefix, char *outStr) const{
    static int index = 0;
    sprintf(outStr, "_%s_%s%d", prefix, funcName, index++);
}


//////////////////////////////
// AST NODE register method
/////////////////////////////
void AST_NODE::setRegister(Register *tmp, bool autoload){
    reg = tmp;
    if(type() == IDENTIFIER_NODE){
        Address addr = ar.getAddress(this);
        if(autoload) reg->load(addr);
        reg->setTarget(addr);
        DebugInfo("\t\t\t\t\e[35mcant find any, alloc a new register: %s to ID: %s(%s)\e[m\n", reg->name(), getSymbol()->name, addr.getName());
    }
    else 
        reg->setTarget(this);
}

Register *AST_NODE::getTempReg(int option){
    bool isReset = option & RegReset;
    bool isCaller = option & RegforceCaller;
    bool isDisableload = option & RegDisableload;

    if(!isReset && reg != NULL && reg->fit(this)) return reg;

    bool isID = (type() == IDENTIFIER_NODE && getSymbol()->attribute->getKind() == VARIABLE_ATTRIBUTE);
    Address address = ar.getAddress(this);
    if(!isReset && isID && regSystem.getFit(address) != NULL){
        reg = regSystem.getFit(address);
        DebugInfo("\t\t\t\t\e[35mfound register: %s in same symbol: %s with addr: %s\e[m", reg->name(), this->getSymbol()->name, address.getName());
    } else {
        DebugInfo("try to get new register");
        // DATA_TYPE type = getDataType();
        Register *tmp = regSystem.getReg(getDataType(), isCaller);
        setRegister(tmp, !isDisableload);
    }
    return reg;
}

const char *AST_NODE::getTempName(){
    Register *reg = getTempReg();
    return reg->name();
}

DATA_TYPE AST_NODE::getDataType() const {
    if(type() == IDENTIFIER_NODE && getSymbol()->attribute->getKind() == VARIABLE_ATTRIBUTE)
        return getSymbol()->attribute->getTypeDes()->getDataType();
    else if(type() == EXPR_NODE && getExprKind() == BINARY_OPERATION && getBinaryOp() > BINARY_OP_DIV)
        return INT_TYPE;
    return dataType;
}
