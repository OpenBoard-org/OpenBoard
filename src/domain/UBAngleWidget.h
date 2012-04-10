#ifndef UBANGLEWIDGET_H
#define UBANGLEWIDGET_H

#include <QWidget>
#include <QBitmap>

class UBAngleWidget : public QWidget
{
    Q_OBJECT

public:
    UBAngleWidget(QWidget *parent = 0);
    ~UBAngleWidget();

    void setText(QString);

protected:
    void paintEvent(QPaintEvent *event);

private:
    QString text;
    QBitmap bmpMask;
};

#endif // UBANGLEWIDGET_H
