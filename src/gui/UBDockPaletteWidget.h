#ifndef UBDOCKPALETTEWIDGET_H
#define UBDOCKPALETTEWIDGET_H

#include <QWidget>
#include <QPixmap>
#include <QString>

class UBDockPaletteWidget : public QWidget
{
    Q_OBJECT
public:
    UBDockPaletteWidget(QWidget* parent=0, const char* name="UBDockPaletteWidget");
    ~UBDockPaletteWidget();

    QPixmap iconToRight();
    QPixmap iconToLeft();
    QString name();

signals:
    void hideTab(const QString& widgetName);
    void showTab(const QString& widgetName);

protected:
    QPixmap mIconToRight;   // arrow like this: >
    QPixmap mIconToLeft;    // arrow like this: <
    QString mName;

};

#endif // UBDOCKPALETTEWIDGET_H
