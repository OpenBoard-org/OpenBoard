
#include "UBDesktopServices.h"

QString UBDesktopServices::storageLocation(StandardLocation type)
{
    // Windows implementation of QDesktopServices is fine
    return QDesktopServices::storageLocation(type);
}
