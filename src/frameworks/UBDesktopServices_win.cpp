
#include "UBDesktopServices.h"
#include "core/memcheck.h"

QString UBDesktopServices::storageLocation(StandardLocation type)
{
    // Windows implementation of QDesktopServices is fine
    return QDesktopServices::storageLocation(type);
}
