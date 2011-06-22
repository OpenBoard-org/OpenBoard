
#include <QtGui>

#include "UBPageNumberPalette.h"

#include "core/memcheck.h"

UBPageNumberPalette::UBPageNumberPalette(QWidget *parent)
    : UBFloatingPalette(Qt::BottomRightCorner, parent)
{
    mLayout = new QHBoxLayout(this);
    mButton = new QPushButton(parent);
    mButton->setStyleSheet(QString("QPushButton { color: white; background-color: transparent; border: none; font-family: Arial; font-weight: bold; font-size: 20px }"));
    mButton->setFocusPolicy(Qt::NoFocus);
    connect(mButton, SIGNAL(clicked(bool)), this, SLOT(buttonClicked(bool)));

    mLayout->setContentsMargins(radius() + 15, 4, radius() + 15, 4);

    mLayout->addWidget(mButton);

    setPageNumber(0, 0);
}

UBPageNumberPalette::~UBPageNumberPalette()
{
    delete mButton;
    delete mLayout;
}

void UBPageNumberPalette::buttonClicked(bool checked)
{
    Q_UNUSED(checked);

    emit clicked();
}

int UBPageNumberPalette::radius()
{
    return 10;
}

void UBPageNumberPalette::setPageNumber(int current, int total)
{
    mButton->setText(QString("%1 / %2").arg(current).arg(total));
    adjustSizeAndPosition();
}
