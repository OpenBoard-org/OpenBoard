
#ifndef UBPOWERPOINTAPPLICATION_H_
#define UBPOWERPOINTAPPLICATION_H_

#if defined(Q_OS_WIN)
#include "UBPowerPointApplication_win.h"
#elif defined(Q_OS_MAC)
#include "UBPowerPointApplication_mac.h"
#else
//TODO Linux
#endif

#endif /* UBPOWERPOINTAPPLICATION_H_ */
