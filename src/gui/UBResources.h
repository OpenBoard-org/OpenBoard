
#ifndef UBRESOURCES_H_
#define UBRESOURCES_H_

#include <QtGui>

class UBResources : public QObject
{
    Q_OBJECT;

    public:
         static UBResources* resources();

    private:
         UBResources(QObject* pParent = 0);
         virtual ~UBResources();

         void init();

         static UBResources* sSingleton;

    public:

         QCursor penCursor;
         QCursor eraserCursor;
         QCursor markerCursor;
         QCursor pointerCursor;
         QCursor handCursor;
         QCursor zoomInCursor;
         QCursor zoomOutCursor;
         QCursor arrowCursor;
         QCursor textCursor;
         QCursor rotateCursor;
};

#endif /* UBRESOURCES_H_ */
