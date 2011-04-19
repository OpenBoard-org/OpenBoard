
#include <QtCore>

#include "UBPlatformUtils.h"


QString UBPlatformUtils::applicationResourcesDirectory()
{
    return QCoreApplication::applicationDirPath() + "/resources";
}
