
#include "UBDesktopServices.h"

QString UBDesktopServices::storageLocation(StandardLocation type)
{
    return QDesktopServices::storageLocation(type);
}
