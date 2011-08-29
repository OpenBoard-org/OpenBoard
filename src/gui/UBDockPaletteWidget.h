#ifndef UBDOCKPALETTEWIDGET_H
#define UBDOCKPALETTEWIDGET_H

#include <QWidget>
#include <QIcon>
#include <QString>

class UBDockPaletteWidget : public QWidget
{
public:
    UBDockPaletteWidget(const char* name="UBDockPaletteWidget", QWidget* parent=0);
    ~UBDockPaletteWidget();

    QWidget* widget();
    QIcon icon();
    QIcon collapsedIcon();
    QString name();

protected:
    QWidget* mpWidget;
    QIcon mIcon;
    QIcon mCollapsedIcon;
    QString mName;

};

#endif // UBDOCKPALETTEWIDGET_H
