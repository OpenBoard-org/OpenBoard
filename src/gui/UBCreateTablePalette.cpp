#include "UBCreateTablePalette.h"
#include <QPainter>
#include <QDebug>

UBCreateTablePalette::UBCreateTablePalette(QWidget *parent) :
    UBFloatingPalette(Qt::TopLeftCorner, parent)
    , mVLayout(new QVBoxLayout(this))
    , mLinesLayout(new QHBoxLayout(this))
    , mColumnsLayout(new QHBoxLayout(this))
    , mLinesLabel(new QLabel(tr("Number of lines :"), this))
    , mLinesInput(new QSpinBox(this))
    , mColumnsLabel(new QLabel(tr("Number of columns :"), this))
    , mColumnsInput(new QSpinBox(this))
    , mValidateButton(new QPushButton(tr("Validate"),this))
    , mClosePixmap(QPixmap(":/images/close.svg"))
    , mMinimumTableSize(1)
{
    mLinesInput->setMinimum(mMinimumTableSize);
    mColumnsInput->setMinimum(mMinimumTableSize);

    mLinesLayout->addWidget(mLinesLabel);
    mLinesLayout->addWidget(mLinesInput);

    mColumnsLayout->addWidget(mColumnsLabel);
    mColumnsLayout->addWidget(mColumnsInput);

    mVLayout->addItem(mColumnsLayout);
    mVLayout->addItem(mLinesLayout);
    mVLayout->addWidget(mValidateButton);

    mVLayout->setMargin(mClosePixmap.width());

    setGeometry(QRect(QPoint(), mVLayout->sizeHint()));

    setBackgroundBrush(QColor(183, 183, 183));

    connect(mValidateButton, SIGNAL(clicked()), this, SIGNAL(validationRequired()));
}

void UBCreateTablePalette::setPos(QPoint pos)
{
    move(pos);
}

UBCreateTablePalette::~UBCreateTablePalette()
{

}

int UBCreateTablePalette::lines() const
{
    return mLinesInput->value();
}

int UBCreateTablePalette::columns() const
{
    return mColumnsInput->value();
}

void UBCreateTablePalette::paintEvent(QPaintEvent *event)
{
    UBFloatingPalette::paintEvent(event);

    QPainter painter(this);
    painter.drawPixmap(0, 0, mClosePixmap);
}

void UBCreateTablePalette::mouseReleaseEvent(QMouseEvent * event)
{
    if (event->pos().x() >= 0 && event->pos().x() < QPixmap(":/images/close.svg").width()
        && event->pos().y() >= 0 && event->pos().y() < QPixmap(":/images/close.svg").height())
    {
        event->accept();
        hide();
    }

    UBFloatingPalette::mouseReleaseEvent(event);
}

void UBCreateTablePalette::keyPressEvent(QKeyEvent * event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter){
        validationRequired();
    }
    else if (event->key() == Qt::Key_Escape){
        close();
    }
}
