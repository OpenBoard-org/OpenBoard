#ifndef UBGLOBALS_H
#define UBGLOBALS_H

#define DELETEPTR(ptr) if(NULL != ptr){ \
    delete ptr; \
    ptr = NULL; \
    }

#endif // UBGLOBALS_H
