#include "UBCellPropertiesPalette.h"
#include <QPainter>
#include <QDebug>

UBCellPropertiesPalette::UBCellPropertiesPalette(QWidget *parent) :
    UBFloatingPalette(Qt::TopLeftCorner, parent)
    , mVLayout(new QVBoxLayout(this))
    , mWidthLayout(new QHBoxLayout(this))
    , mWidthLabel(new QLabel(tr("Width (%) :"), this))
    , mWidthInput(new QSpinBox(this))
    , mValidateButton(new QPushButton(tr("Validate"),this))
    , mClosePixmap(QPixmap(":/images/close.svg"))
    , mMinimumTableSize(1)
{
    mWidthInput->setMinimum(mMinimumTableSize);

    mWidthLayout->addWidget(mWidthLabel);
    mWidthLayout->addWidget(mWidthInput);

    mVLayout->addItem(mWidthLayout);
    mVLayout->addWidget(mValidateButton);

    mVLayout->setMargin(mClosePixmap.width());

    setGeometry(QRect(QPoint(), mVLayout->sizeHint()));

    setBackgroundBrush(QColor(183, 183, 183));

    connect(mValidateButton, SIGNAL(clicked()), this, SIGNAL(validationRequired()));
}

void UBCellPropertiesPalette::setPos(QPoint pos)
{
    move(pos);
}

UBCellPropertiesPalette::~UBCellPropertiesPalette()
{

}

int UBCellPropertiesPalette::width() const
{
    return mWidthInput->value();
}

void UBCellPropertiesPalette::setWidth(int value)
{
    mWidthInput->setValue(value);
}

void UBCellPropertiesPalette::paintEvent(QPaintEvent *event)
{
    UBFloatingPalette::paintEvent(event);

    QPainter painter(this);
    painter.drawPixmap(0, 0, mClosePixmap);
}

void UBCellPropertiesPalette::mouseReleaseEvent(QMouseEvent * event)
{
    if (event->pos().x() >= 0 && event->pos().x() < QPixmap(":/images/close.svg").width()
        && event->pos().y() >= 0 && event->pos().y() < QPixmap(":/images/close.svg").height())
    {
        event->accept();
        hide();
    }

    UBFloatingPalette::mouseReleaseEvent(event);
}

void UBCellPropertiesPalette::keyPressEvent(QKeyEvent * event)
{
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return){
        validationRequired();
    }
    else if (event->key() == Qt::Key_Escape){
        close();
    }
}

