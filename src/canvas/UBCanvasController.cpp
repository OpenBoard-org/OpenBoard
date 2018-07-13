#include "UBCanvasController.h"
#include <gui/UBMainWindow.h>
#include <core/UBSettings.h>
#include <core/UBApplication.h>
#include <QCheckBox>
#include <QPushButton>
#include <QTouchDevice>
#include "canvas/UBCanvasPalette.h"

UBCanvasController::UBCanvasController(UBMainWindow *mainWindow)
    : mMainWindow(mainWindow)
    , mControlContainer(0)
    , mControlView (0)
    , numberBoards(4)
{
    QTouchDevice t;
    qWarning()<<" @@@@@ #### ----- MAXIMUM TOUCH POINTS: "<<t.maximumTouchPoints();
    qWarning()<<" @@@@@ #### ----- MAXIMUM TOUCH POINTS: "<<t.capabilities();
    penColors
            << Qt::black //QColor("red")
            << Qt::red //QColor("blue")
            << Qt::blue //QColor("green")
            << Qt::green;// QColor("darkGreen");
    init();
    updateBoards(numberBoards);
}

void UBCanvasController::updateBoards(int n){
    numberBoards = n;
    pbREM->setDisabled(false);
    pbADD->setDisabled(false);
    if(numberBoards == 1)                       pbREM->setDisabled(true);
    if(numberBoards == MAX_NUMBER_OF_BOARDS)    pbADD->setDisabled(true);

    QPixmap p = (new QIcon(":images/canvas/users"+QString::number(numberBoards)+".png"))->pixmap(QSize(48,48)); // load pixmap
    labelInfo->setPixmap(p);


    for (int i=0; i < MAX_NUMBER_OF_BOARDS; i++){
        listOfPalettes.at(i)->hide();
        listPbColors.at(i)->setEnabled(false);
        listPbColors.at(i)->show();
    }
    // Update position of palettes
    for (int i=0; i<numberBoards; i++){
        QRect r = mControlView->getRegion(i);
        listOfPalettes.at(i)->movePalette(i, r.width(), r.height());
        listOfPalettes.at(i)->setWindowFlags(listOfPalettes.at(i)->windowFlags() | Qt::WindowStaysOnTopHint);
        listOfPalettes.at(i)->penToolSelected(true);
        listOfPalettes.at(i)->show();
        listPbColors.at(i)->setEnabled(true);
        listPbColors.at(i)->show();
    }
}

UBCanvasController::~UBCanvasController()
{
}

void UBCanvasController::show()
{
    mControlView->show();
    mMainWindow->canvasToolBar->setDisabled(false);
    mMainWindow->actionCanvas->setDisabled(true);
    mControlView->setDisabled(true);
    updateBoards(numberBoards);
}

void UBCanvasController::hide()
{
    mControlView->hide();
    mMainWindow->canvasToolBar->setDisabled(true);
    mMainWindow->actionCanvas->setDisabled(false);
    mControlView->setDisabled(false);
    for (int i=0; i < MAX_NUMBER_OF_BOARDS; i++){
       listOfPalettes.at(i)->hide();
    }
}

void UBCanvasController::init()
{
    backgroundColor = Qt::white;
    setupViews();
    setupToolbar();
}

void UBCanvasController::setDisabled(bool disable)
{
    mMainWindow->canvasToolBar->setDisabled(disable);
    mControlView->setDisabled(disable);
}

void UBCanvasController::setupViews()
{
    mControlContainer = new QWidget(mMainWindow->centralWidget());
    mMainWindow->addCanvasWidget(mControlContainer);

    // Creation of the pens, one per user.
    for (int i=0; i < MAX_NUMBER_OF_BOARDS; i++){
       QPen *pen = new QPen(penColors.at(i), 5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
       listOfPens.append(pen);
    }

    // We know that TOOLBAR HEIGHT is 74, but mMainWindow->canvasToolBar->geometry().height() returns 30 at this point ...
    qWarning()<<"mMainWindow->canvasToolBar->height(): "<<mMainWindow->actionBoard;
    mControlView = new UBCanvasView(numberBoards, 74, backgroundColor, listOfPens, mMainWindow);//(this, mControlContainer, true, false);
    mControlView->hide();
    mControlContainer->setObjectName("ubCanvasControlContainer");
    mMainWindow->addCanvasWidget(mControlContainer);
}

void UBCanvasController::increaseNumberOfBoards(){
    if(numberBoards < MAX_NUMBER_OF_BOARDS){
        mControlView->setupBoards(numberBoards + 1);
        updateBoards(numberBoards + 1);
    }
}

void UBCanvasController::decreaseNumberOfBoards(){
    if(numberBoards > 1){
        mControlView->setupBoards(numberBoards - 1);
        updateBoards(numberBoards - 1);
    }
}

void UBCanvasController::setupToolbar()
{
    UBSettings *settings = UBSettings::settings();

    // Interface to add or remove BOARDs
    boardInfo = new QWidget(mMainWindow->canvasToolBar);
    boardInfoLayout = new QHBoxLayout(mMainWindow->canvasToolBar);
    boardInfoLayout->setSpacing(20);
    labelInfo = new QLabel();

    pbADD = new QPushButton(mMainWindow);
    QPixmap pixADD(":images/canvas/pbADD.png");
    QIcon pbADDicon(pixADD);
    pbADD->setIcon(pbADDicon);
    pbADD->setIconSize(pixADD.rect().size());

    pbREM = new QPushButton(mMainWindow);
    QPixmap pixREM(":images/canvas/pbREM.png");
    QIcon pbREMicon(pixREM);
    pbREM->setIcon(pbREMicon);
    pbREM->setIconSize(pixREM.rect().size());

    boardInfoLayout->addWidget(pbREM);
    boardInfoLayout->addWidget(labelInfo);
    boardInfoLayout->addWidget(pbADD);
    boardInfo->setLayout(boardInfoLayout);
    mMainWindow->canvasToolBar->addWidget(boardInfo);

    connect(pbADD, SIGNAL(pressed()),this, SLOT(increaseNumberOfBoards()));
    connect(pbREM, SIGNAL(pressed()),this, SLOT(decreaseNumberOfBoards()));
    // ---
    // New widget to store the color choice:
    colorButtons = new QWidget(mMainWindow->canvasToolBar);
    colorLayout = new QVBoxLayout(mMainWindow->canvasToolBar);
    QHBoxLayout *layout = new QHBoxLayout(mMainWindow->canvasToolBar);
    layout->setSpacing(1);
    for (int i=0; i < MAX_NUMBER_OF_BOARDS; i++)
    {
        QPushButton *pb = new QPushButton(mMainWindow->canvasToolBar);
        changeButtonColor(pb, penColors.at(i));
        pb->setProperty("ID",i);
        connect(pb, SIGNAL(clicked(bool)),this,SLOT(choiceColorDialog()));
        layout->addWidget(pb);
        listPbColors.append(pb);
        listPbColors.at(i)->setEnabled(false);
        listPbColors.at(i)->show();
    }
    colorLayout->addLayout(layout);
    QLabel *lb = new QLabel(mMainWindow);
    lb->setText(tr("Color"));
    lb->setAlignment(Qt::AlignCenter);
    colorLayout->addWidget(lb);
    colorButtons->setLayout(colorLayout);
    mMainWindow->canvasToolBar->addWidget(colorButtons);
    // ---
    // Creation of the palettes, one per user.
    for (int i=0; i < MAX_NUMBER_OF_BOARDS; i++)
    {
       UBCanvasPalette *palette = new UBCanvasPalette(mMainWindow, i, UBSettings::settings()->appToolBarOrientationVertical->get().toBool() ? Qt::Vertical : Qt::Horizontal);
       palette->hide();
       listOfPalettes.append(palette);
       connect(palette,SIGNAL(penSelected(int)),this,SLOT(penToolSelected(int)));
       connect(palette,SIGNAL(eraserSelected(int)),this,SLOT(eraserToolSelected(int)));
       connect(palette,SIGNAL(saveSelected(int)),this,SLOT(saveToolSelected(int)));
       connect(palette,SIGNAL(eraseRegionSelected(int)),this,SLOT(eraseRegionToolSelected(int)));
    }

    connect(mControlView, SIGNAL(gestureErase(int)), this, SLOT(eraserToolSelectedGesture(int)));
    connect(mControlView, SIGNAL(endGestureErase(int)), this, SLOT(restorePenToolGesture(int)));

    // To move the pre-defined ACTIONS, we need to add them again!!
    mMainWindow->canvasToolBar->addAction(mMainWindow->actionEraseCanvas);
    mMainWindow->canvasToolBar->addAction(mMainWindow->actionSaveCanvas);
    mMainWindow->canvasToolBar->addAction(mMainWindow->actionBoard);
    mMainWindow->canvasToolBar->addAction(mMainWindow->actionCanvas);
    mMainWindow->canvasToolBar->addAction(mMainWindow->actionWeb);
    mMainWindow->canvasToolBar->addAction(mMainWindow->actionDocument);
    mMainWindow->canvasToolBar->addAction(mMainWindow->actionDesktop);
    mMainWindow->canvasToolBar->addAction(mMainWindow->actionMenu);
    UBApplication::app()->insertSpaceToToolbarBeforeAction(mMainWindow->canvasToolBar, mMainWindow->actionEraseCanvas, 10);
    UBApplication::app()->insertSpaceToToolbarBeforeAction(mMainWindow->canvasToolBar, mMainWindow->actionBoard);
    UBApplication::app()->decorateActionMenu(mMainWindow->actionMenu);

    connect(mMainWindow->actionEraseCanvas, SIGNAL(triggered(bool)),this,SLOT(eraseALL()));
    connect(mMainWindow->actionSaveCanvas, SIGNAL(triggered(bool)),this, SLOT(saveCanvas()));
}

void UBCanvasController::eraseALL(){
        mControlView->clearImage();
        mMainWindow->actionEraseCanvas->setChecked(false);
}

void UBCanvasController::saveCanvas()
{
  // To do
    mControlView->saveImage();
}

void UBCanvasController::penToolSelected(int idPalette)
{
    qWarning()<<"clicked on pen-tool action in palette "<<idPalette;
    mControlView->setEraserMode(idPalette, false);
}

void UBCanvasController::eraserToolSelectedGesture(int idPalette)
{
    listOfPalettes.at(idPalette)->eraserToolSelected(true);
    eraserToolSelected(idPalette);
}

void UBCanvasController::restorePenToolGesture(int idPalette)
{
    listOfPalettes.at(idPalette)->penToolSelected(true);
    penToolSelected(idPalette);
}

void UBCanvasController::eraserToolSelected(int idPalette)
{
    qWarning()<<"clicked on eraser-tool action in palette "<<idPalette;
    mControlView->setEraserMode(idPalette, true);
}

void UBCanvasController::saveToolSelected(int idPalette)
{
    qWarning()<<"clicked on save-tool action in palette "<<idPalette;
    mControlView->saveRegionImage(idPalette);
}

void UBCanvasController::eraseRegionToolSelected(int idPalette)
{
    qWarning()<<"clicked on save-tool action in palette "<<idPalette;
    mControlView->clearRegion(idPalette);
}

void UBCanvasController::choiceColorDialog()
{
    int id = sender()->property("ID").toInt();
    QColor color = QColorDialog::getColor(penColors.at(id), mMainWindow);
    if( color.isValid() )
    {
      penColors.replace(id, color);
      listOfPens.at(id)->setColor(color);
      changeButtonColor(listPbColors.at(id),color);
    }
}

void UBCanvasController::changeButtonColor(QPushButton* pb, QColor color)
{
    QPixmap pixPB(12, 12);
    pixPB.fill(color);
    QIcon icon(pixPB);
    pb->setIcon(icon);
}
