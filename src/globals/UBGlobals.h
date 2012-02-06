#ifndef UBGLOBALS_H
#define UBGLOBALS_H

#define DELETEPTR(ptr) if(NULL != ptr){ \
    delete ptr; \
    ptr = NULL; \
    }

#ifdef Q_WS_WIN

#define WARNINGS_DISABLE __pragma(warning(push, 0));
#define WARNINGS_ENABLE __pragma(warning(pop));

#ifdef NO_THIRD_PARTY_WARNINGS
// disabling warning level to 0 and save old state
#define THIRD_PARTY_WARNINGS_DISABLE WARNINGS_DISABLE
#else 
// just save old state (needs for not empty define)
#define THIRD_PARTY_WARNINGS_DISABLE __pragma(warning(push));
#endif //#ifdef NO_THIRD_PARTY_WARNINGS
// anyway on WIN
#define THIRD_PARTY_WARNINGS_ENABLE WARNINGS_ENABLE

#else //#ifdef Q_WS_WIN

#define WARNINGS_DISABLE _Pragma("GCC diagnostic push"); \
_Pragma("GCC diagnostic ignored \"-Wunused-parameter\""); \
_Pragma("GCC diagnostic ignored \"-Wunused-variable\""); \
_Pragma("GCC diagnostic ignored \"-Wsign-compare\""); 

#define WARNINGS_ENABLE _Pragma("GCC diagnostic pop"); 

#ifdef NO_THIRD_PARTY_WARNINGS
//disabling some warnings
#define THIRD_PARTY_WARNINGS_DISABLE WARNINGS_DISABLE
#define THIRD_PARTY_WARNINGS_ENABLE WARNINGS_ENABLE
#else
// just save old state (needs for not empty define)
#define THIRD_PARTY_WARNINGS_ENABLE WARNINGS_ENABLE

#endif //#ifdef NO_THIRD_PARTY_WARNINGS

#endif //#ifdef Q_WS_WIN

#endif // UBGLOBALS_H
        