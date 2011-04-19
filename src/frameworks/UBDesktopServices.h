
#ifndef UBDESKTOPSERVICES_H_
#define UBDESKTOPSERVICES_H_

#include <QtCore/qglobal.h>
#include <QDesktopServices>

// TODO Qt 4.6 #error Delete this UBDesktopServices class,
// it was used to work around a bug in Qt 4.4 that was fixed in Qt 4.5 and another that should be fixed
// in 4.5.1

class UBDesktopServices : public QDesktopServices
{
    public:
        static QString storageLocation(StandardLocation type);
};

#endif /* UBDESKTOPSERVICES_H_ */
