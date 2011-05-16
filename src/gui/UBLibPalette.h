#ifndef UBLIBPALETTE_H
#define UBLIBPALETTE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QDragMoveEvent>
#include <QMimeData>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QLabel>

#include "UBDockPalette.h"
#include "UBLibNavigatorWidget.h"
#include "UBLibItemProperties.h"
#include "UBLibActionBar.h"

#define ID_NAVIGATOR    0
#define ID_PROPERTIES   1
#define ID_DROPME       2

class UBDropMeWidget : public QWidget
{
public:
    UBDropMeWidget(QWidget* parent=0, const char* name="dropMeWidget");
    ~UBDropMeWidget();

private:
    QLabel* mpLabel;
    QVBoxLayout* mpLayout;
};

class UBLibPalette : public UBDockPalette
{
    Q_OBJECT
public:
    UBLibPalette(QWidget* parent=0, const char* name="libPalette");
    ~UBLibPalette();

    UBLibActionBar* actionBar(){return mActionBar;}

protected:
    void updateMaxWidth();
    void dragEnterEvent(QDragEnterEvent* pEvent);
    void dropEvent(QDropEvent *pEvent);
    void dragMoveEvent(QDragMoveEvent* pEvent);
    void dragLeaveEvent(QDragLeaveEvent* pEvent);
    void mouseMoveEvent(QMouseEvent *event);
    void resizeEvent(QResizeEvent *event);

private slots:
    void showProperties(UBLibElement* elem);
    void showFolder();

private:
    void processMimeData(const QMimeData* pData);

    /** The layout */
    QVBoxLayout* mLayout;
    /** The stacked layout */
    QStackedWidget* mStackedWidget;
    /** The Navigator widget */
    UBLibNavigatorWidget* mNavigator;
    /** The Properties widget */
    UBLibItemProperties* mProperties;
    /** UBLibActionBar */
    UBLibActionBar* mActionBar;
    /** The 'drop here' indicator */
    UBDropMeWidget* mDropWidget;
    /** The current stack widget index*/
    int miCrntStackWidget;
};

#endif // UBLIBPALETTE_H
