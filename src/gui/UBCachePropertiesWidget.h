#ifndef UBCACHEPROPERTIESWIDGET_H
#define UBCACHEPROPERTIESWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSlider>
#include <QVector>

#include "UBDockPaletteWidget.h"
#include "tools/UBGraphicsCache.h"

#define MAX_SHAPE_WIDTH     200

class UBCachePropertiesWidget : public UBDockPaletteWidget
{
    Q_OBJECT
public:
    UBCachePropertiesWidget(QWidget* parent=0, const char* name="UBCachePropertiesWidget");
    ~UBCachePropertiesWidget();

    bool visibleInMode(eUBDockPaletteWidgetMode mode)
    {
        return mode == eUBDockPaletteWidget_BOARD;
    }

public slots:
    void updateCurrentCache();

signals:
    void cacheListEmpty();

private slots:
    void onCloseClicked();
    void updateCacheColor(QColor color);
    void onColorClicked();
    void updateShapeButtons();
    void onSizeChanged(int newSize);
    void onCacheEnabled();

private:
    QVBoxLayout* mpLayout;
    QLabel* mpCachePropertiesLabel;
    QLabel* mpColorLabel;
    QLabel* mpShapeLabel;
    QLabel* mpSizeLabel;
    QPushButton* mpColor;
    QPushButton* mpSquareButton;
    QPushButton* mpCircleButton;
    QPushButton* mpCloseButton;
    QSlider* mpSizeSlider;
    QHBoxLayout* mpColorLayout;
    QHBoxLayout* mpShapeLayout;
    QHBoxLayout* mpSizeLayout;
    QHBoxLayout* mpCloseLayout;
    QWidget* mpProperties;
    QVBoxLayout* mpPropertiesLayout;
    QColor mActualColor;
    eMaskShape mActualShape;
    UBGraphicsCache* mpCurrentCache;

};

#endif // UBCACHEPROPERTIESWIDGET_H
