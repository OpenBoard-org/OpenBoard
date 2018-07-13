#ifndef UBCANVASCONTROLLER_H
#define UBCANVASCONTROLLER_H

#include <QtGui>

#include <QObject>
#include <QHBoxLayout>
#include "core/UBApplicationController.h"
#include "UBCanvasView.h"
#include "UBCanvasPalette.h"

#define MAX_NUMBER_OF_BOARDS 4

class UBCanvasController: public QObject
{

    Q_OBJECT

public:
    UBCanvasController(UBMainWindow *mainWindow);
    virtual ~UBCanvasController();

    void init();
    void updateBoards(int n);
    void show();
    void hide();
    void setDisabled(bool disable);
    void changeButtonColor(QPushButton* pb, QColor color);

    UBMainWindow *mMainWindow;
    QWidget *mControlContainer;
    UBCanvasView *mControlView;

    // Interface to add or remove BOARDs
    QWidget *boardInfo;
    QHBoxLayout *boardInfoLayout;
    QLabel *labelInfo;
    QPushButton* pbADD;
    QPushButton* pbREM;

    QList<UBCanvasPalette*> listOfPalettes;
    QList<QPen*> listOfPens;
    QList<QColor> penColors;

    QWidget *colorButtons;
    QVBoxLayout *colorLayout;
    QList<QPushButton*> listPbColors;

    QColor backgroundColor;

    int numberBoards;

protected:
    void setupViews();
    void setupToolbar();

public slots:
    void increaseNumberOfBoards();
    void decreaseNumberOfBoards();
    void choiceColorDialog();
    void penToolSelected(int idPalette);
    void eraserToolSelectedGesture(int idPalette);
    void restorePenToolGesture(int idPalette);
    void eraserToolSelected(int idPalette);
    void saveToolSelected(int idPalette);
    void eraseRegionToolSelected(int idPalette);
    void eraseALL();
    void saveCanvas();
};

#endif // UBCANVASCONTROLLER_H
