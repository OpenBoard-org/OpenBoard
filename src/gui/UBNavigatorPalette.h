#ifndef UBNAVIGATORPALETTE_H
#define UBNAVIGATORPALETTE_H

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QTimerEvent>
#include <QLabel>
#include <QString>

#include "UBDockPalette.h"
#include "UBDocumentNavigator.h"
#include "document/UBDocumentProxy.h"


class UBNavigatorPalette : public UBDockPalette
{
    Q_OBJECT
public:
    UBNavigatorPalette(QWidget* parent=0, const char* name="navigatorPalette");
    ~UBNavigatorPalette();

    void setDocument(UBDocumentProxy* document);
    void refresh();

public slots:
    void setPageNumber(int current, int total);

protected:
    virtual void resizeEvent(QResizeEvent *event);
    virtual void timerEvent(QTimerEvent *event);

private:
    void updateTime();

    /** The thumbnails navigator widget */
    UBDocumentNavigator* mNavigator;
    /** The layout */
    QVBoxLayout* mLayout;
    QHBoxLayout* mHLayout;
    QLabel* mPageNbr;
    QLabel* mClock;
    QString mTimeFormat;
    int mTimerID;

private slots:
    void changeCurrentPage();
};


#endif // UBNAVIGATORPALETTE_H
