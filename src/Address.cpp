#include "Address.h"
#include "Register.h"

void Address::setName() {
    if(hasReg()) sprintf(addrName, "%d(%s)", _offset, reg->name());
}

const char *Address::getRegName() const{ 
    assert(reg != NULL);
    return reg->name(); 
}

bool Address::hasReg() const { 
    return reg != NULL; 
}
