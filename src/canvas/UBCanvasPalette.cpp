#include "UBCanvasPalette.h"

#include <QtGui>

#include "gui/UBMainWindow.h"

#include "core/UBApplication.h"
#include "core/UBSettings.h"

#include "frameworks/UBPlatformUtils.h"

#include "core/memcheck.h"


UBCanvasPalette::UBCanvasPalette(QWidget *parent, int id, Qt::Orientation orient)
    : UBActionPalette(Qt::TopRightCorner, parent, orient)
    , mLastSelectedId(-1)
{
    idPalette = id;

    QList<QAction*> actions;

    QIcon penIcon;
    penIcon.addPixmap(QPixmap(":images/stylusPalette/pen.png"), QIcon::Normal, QIcon::Off);
    penIcon.addPixmap(QPixmap(":images/stylusPalette/penOn.png"), QIcon::Normal, QIcon::On);
    actionCPen = new QAction(penIcon,tr("Pen"), this);

    QIcon eraserIcon;
    eraserIcon.addPixmap(QPixmap(":images/stylusPalette/eraser.png"), QIcon::Normal, QIcon::Off);
    eraserIcon.addPixmap(QPixmap(":images/stylusPalette/eraserOn.png"), QIcon::Normal, QIcon::On);
    actionCEraser = new QAction(eraserIcon,tr("Eraser"), this);

    QIcon saveIcon;
    saveIcon.addPixmap(QPixmap(":images/stylusPalette/saveRegion.png"), QIcon::Normal, QIcon::Off);
    saveIcon.addPixmap(QPixmap(":images/stylusPalette/saveRegionOn.png"), QIcon::Normal, QIcon::On);
    actionCSave = new QAction(saveIcon,tr("Save"), this);

    QIcon eraseRegionIcon;
    eraseRegionIcon.addPixmap(QPixmap(":images/stylusPalette/clearRegion.png"), QIcon::Normal, QIcon::Off);
    eraseRegionIcon.addPixmap(QPixmap(":images/stylusPalette/clearRegionOn.png"), QIcon::Normal, QIcon::On);
    actionCEraseRegion = new QAction(eraseRegionIcon,tr("Erase region"), this);

    actionCPen->setCheckable(true);
    actionCEraser->setCheckable(true);
    /*actionCSave->setCheckable(true);
    actionCEraseRegion->setCheckable(true);*/

    connect(actionCPen, SIGNAL(triggered(bool)), this, SLOT(penToolSelected(bool)));
    connect(actionCEraser, SIGNAL(triggered(bool)), this, SLOT(eraserToolSelected(bool)));
    connect(actionCSave, SIGNAL(triggered(bool)), this, SLOT(saveToolSelected(bool)));
    connect(actionCEraseRegion, SIGNAL(triggered(bool)), this, SLOT(eraseRegionToolSelected(bool)));

    actionCPen->setChecked(true);
    actionCEraser->setChecked(false);
    /*actionCSave->setChecked(false);
    actionCEraseRegion->setChecked(false);*/

    actions << actionCPen;
    actions << actionCEraser;
    actions << actionCSave;
    actions << actionCEraseRegion;

    setActions(actions);
    setButtonIconSize(QSize(39, 39));

   /* if(!UBPlatformUtils::hasVirtualKeyboard())
    {
            groupActions();
    }
    else
    {
            // VirtualKeyboard and Drawing actions are not in group
            // So, groupping all buttons, except first and last
            mButtonGroup = new QButtonGroup(this);
            for(int i=1; i < mButtons.size()-1; i++)
            {
                    mButtonGroup->addButton(mButtons[i], i);
            }
        connect(mButtonGroup, SIGNAL(buttonClicked(int)), this, SIGNAL(buttonGroupClicked(int)));
    }*/

    adjustSizeAndPosition();

    //initPosition();
    this->hide();

}

void UBCanvasPalette::penToolSelected(bool checked){
        actionCPen->setChecked(true);
        actionCEraser->setChecked(false);
        qWarning()<<"********** penToolSelected";
        /*actionCSave->setChecked(false);
        actionCEraseRegion->setChecked(false);*/
        emit penSelected(idPalette);
}

void UBCanvasPalette::eraserToolSelected(bool checked){
        actionCPen->setChecked(false);
        actionCEraser->setChecked(true);
        qWarning()<<"eraserToolSelected";
        /*actionCSave->setChecked(false);
        actionCEraseRegion->setChecked(false);*/
        emit eraserSelected(idPalette);
}

void UBCanvasPalette::saveToolSelected(bool checked){
        /*actionCPen->setChecked(false);
        actionCEraser->setChecked(false);
        actionCSave->setChecked(true);
        actionCEraseRegion->setChecked(false);*/
        emit saveSelected(idPalette);
}

void UBCanvasPalette::eraseRegionToolSelected(bool checked){
        /*actionCPen->setChecked(false);
        actionCEraser->setChecked(false);
        actionCSave->setChecked(false);
        actionCEraseRegion->setChecked(true);*/
        emit eraseRegionSelected(idPalette);
}

void UBCanvasPalette::bgClicked(bool id)
{
    int i = sender()->property("ID").toInt();
    qWarning()<<"Botón: "<<i;
}

void UBCanvasPalette::initPosition()
{
    if(!UBSettings::settings()->appToolBarOrientationVertical->get().toBool())
    {
        QWidget* pParentW = parentWidget();
        if(NULL != pParentW)
        {
            mCustomPosition = true;
            QPoint pos;
            int parentWidth = pParentW->width();
            int parentHeight = pParentW->height();
            int posX = (parentWidth / 2) - (width() / 2);
            int posY = parentHeight - border() - height();

            pos.setX(posX);
            pos.setY(posY);
            moveInsideParent(pos);
        }
    }
}

void UBCanvasPalette::movePalette(int id, int w, int h){
    if(!UBSettings::settings()->appToolBarOrientationVertical->get().toBool())
    {
        mCustomPosition = true;
        QPoint pos;
        int parentWidth = w;
        int parentHeight = h;
        int posX = (parentWidth / 2) - (width() / 2);
        int posY = parentHeight - border() - height();

        pos.setX((id*w)+posX);
        pos.setY(posY);
        moveInsideParent(pos);
    }
}

UBCanvasPalette::~UBCanvasPalette()
{

}



void UBCanvasPalette::canvasToolDoubleClicked()
{
    //Issue "retours 2.4RC1 - CFA - 20140217 : la drawingPalette créé un décalage d'index entre checkedId et les valeurs correspondantes dans l'enum UBStylusTool
    //on décrémente pour rétablir la correspondance
    emit canvasToolDoubleClicked(mButtonGroup->checkedId() -1 );
}



