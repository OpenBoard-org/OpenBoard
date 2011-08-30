#ifndef UBDOCKPALETTEWIDGET_H
#define UBDOCKPALETTEWIDGET_H

#include <QWidget>
#include <QPixmap>
#include <QString>

class UBDockPaletteWidget : public QWidget
{
public:
    UBDockPaletteWidget(QWidget* parent=0, const char* name="UBDockPaletteWidget");
    ~UBDockPaletteWidget();

    QPixmap iconToRight();
    QPixmap iconToLeft();
    QString name();

protected:
    QPixmap mIconToRight;   // arrow like this: >
    QPixmap mIconToLeft;    // arrow like this: <
    QString mName;

};

#endif // UBDOCKPALETTEWIDGET_H
