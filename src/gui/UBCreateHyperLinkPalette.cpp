
#include "UBCreateHyperLinkPalette.h"
#include <QPainter>
#include <QDebug>

UBCreateHyperLinkPalette::UBCreateHyperLinkPalette(QWidget *parent) :
    UBFloatingPalette(Qt::TopLeftCorner, parent)
    , mVLayout(new QVBoxLayout(this))
    , mLinkLayout(new QHBoxLayout(this))
    , mTextLayout(new QHBoxLayout(this))
    , mLinkLabel(new QLabel(tr("Link :"), this))
    , mLinkInput(new QLineEdit(this))
    , mTextLabel(new QLabel(tr("Text :"), this))
    , mTextInput(new QLineEdit(this))
    , mValidateButton(new QPushButton(tr("Validate"),this))
    , mClosePixmap(QPixmap(":/images/close.svg"))
{
    mLinkLayout->addWidget(mLinkLabel);
    mLinkLayout->addWidget(mLinkInput);

    mTextLayout->addWidget(mTextLabel);
    mTextLayout->addWidget(mTextInput);

    mVLayout->addItem(mLinkLayout);
    mVLayout->addItem(mTextLayout);
    mVLayout->addWidget(mValidateButton);

    mVLayout->setMargin(mClosePixmap.width());

    setGeometry(QRect(QPoint(), mVLayout->sizeHint()));

    setBackgroundBrush(QColor(183, 183, 183));

    connect(mValidateButton, SIGNAL(clicked()), this, SIGNAL(validationRequired()));
}

void UBCreateHyperLinkPalette::setPos(QPoint pos)
{
    move(pos);
}

void UBCreateHyperLinkPalette::setText(QString text)
{
    mTextInput->setText(text);
}

void UBCreateHyperLinkPalette::setLink(QString link)
{
    mLinkInput->setText(link);
}

UBCreateHyperLinkPalette::~UBCreateHyperLinkPalette()
{

}

QString UBCreateHyperLinkPalette::link() const
{
    return mLinkInput->text();
}

QString UBCreateHyperLinkPalette::text() const
{
    return mTextInput->text();
}

void UBCreateHyperLinkPalette::paintEvent(QPaintEvent *event)
{
    UBFloatingPalette::paintEvent(event);

    QPainter painter(this);
    painter.drawPixmap(0, 0, mClosePixmap);
}

void UBCreateHyperLinkPalette::mouseReleaseEvent(QMouseEvent * event)
{
    if (event->pos().x() >= 0 && event->pos().x() < QPixmap(":/images/close.svg").width()
        && event->pos().y() >= 0 && event->pos().y() < QPixmap(":/images/close.svg").height())
    {
        event->accept();
        hide();
    }

    UBFloatingPalette::mouseReleaseEvent(event);
}

void UBCreateHyperLinkPalette::keyPressEvent(QKeyEvent * event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter){
        validationRequired();
    }
    if (event->key() == Qt::Key_Escape){
        close();
    }
}
