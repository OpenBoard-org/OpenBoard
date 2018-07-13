#include "UBPopUp.h"

#include <QPainter>
#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>
#include <QIcon>

UBPopUp::UBPopUp(QWidget *parent, QString Action , QString title, QString pathIcon) : QWidget(parent)
{
    setWindowFlags(Qt::FramelessWindowHint |        // Disable window decoration
                   Qt::Tool |                       // Discard display in a separate window
                   Qt::WindowStaysOnTopHint);       // Set on top of all windows
    setAttribute(Qt::WA_TranslucentBackground);     // Indicates that the background will be transparent
    setAttribute(Qt::WA_ShowWithoutActivating);     // At the show, the widget does not get the focus automatically

    animation.setTargetObject(this);                // Set the target animation
    animation.setPropertyName("popupOpacity");      //
    connect(&animation, &QAbstractAnimation::finished, this, &UBPopUp::hide);

    label.setStyleSheet("QLabel { color : white; "
                        "margin-top: 6px;"
                        "margin-bottom: 6px;"
                        "margin-left: 10px;"
                        "margin-right: 10px; "
                        "font-size: 8pt;"
                        "}");

    action.setText(Action);
    action.setAlignment(Qt::AlignCenter);
    action.setStyleSheet("QLabel { background-color : white; color : black; font-style: italic; font-size: 6pt; }");
    header.setText("<img src=\""+pathIcon+"\"/> "+title+":");
    header.setStyleSheet("QLabel { background-color : white; color : black; font-style: normal;font-size: 12pt;font-weight: bold; }");

    layout.addWidget(&action,0, 0);
    layout.addWidget(&header,1, 0);
    layout.addWidget(&label, 2, 0);
    setLayout(&layout);

    timer = new QTimer();
    connect(timer, &QTimer::timeout, this, &UBPopUp::hideAnimation);
}

void UBPopUp::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRect roundedRect;
    roundedRect.setX(rect().x() + 5);
    roundedRect.setY(rect().y() + 5);
    roundedRect.setWidth(rect().width() - 10);
    roundedRect.setHeight(rect().height() - 10);

    painter.setBrush(QBrush(QColor(0,0,0,180)));
    painter.setPen(Qt::NoPen);

    painter.drawRoundedRect(roundedRect, 10, 10);
}

void UBPopUp::setPopupText(const QString &text)
{
    label.setText(text);    // Set the text in the Label
    adjustSize();           // With the recalculation notice sizes
}

void UBPopUp::show()
{
    setWindowOpacity(0.0);  // Set the transparency to zero

    animation.setDuration(300);     // Configuring the duration of the animation
    animation.setStartValue(0.0);   // The start value is 0 (fully transparent widget)
    animation.setEndValue(1.0);     // End - completely opaque widget

    setGeometry(QApplication::desktop()->availableGeometry().width() - 36 - width() + QApplication::desktop() -> availableGeometry().x(),
                QApplication::desktop()->availableGeometry().height() - 36 - height() + QApplication::desktop() -> availableGeometry().y(),
                width(),
                height());
    QWidget::show();

    animation.start();
    timer->start(3000);
}

void UBPopUp::hideAnimation()
{
    timer->stop();
    animation.setDuration(2000);
    animation.setStartValue(1.0);
    animation.setEndValue(0.0);
    animation.start();
}

void UBPopUp::hide()
{
    // If the widget is transparent, then hide it
    if(getPopupOpacity() == 0.0){
        QWidget::hide();
    }
}

void UBPopUp::setPopupOpacity(float opacity)
{
    popupOpacity = opacity;

    setWindowOpacity(opacity);
}

float UBPopUp::getPopupOpacity() const
{
    return popupOpacity;
}
