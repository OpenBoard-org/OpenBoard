
#if defined(WIN32) && defined(_DEBUG)
   #include <cstdlib>
   #define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
   #define new DEBUG_NEW
#endif
