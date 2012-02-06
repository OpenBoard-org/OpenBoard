#ifndef UBWIDGETLIST_H
#define UBWIDGETLIST_H

#include <QWidget>
#include <QScrollArea>
#include <QBoxLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QResizeEvent>
#include <QVector>
#include <QLabel>

#include "interfaces/IResizeable.h"

typedef enum{
    eWidgetListOrientation_Vertical,
    eWidgetListOrientation_Horizontal
}eWidgetListOrientation;


class UBWidgetList : public QScrollArea
{
    Q_OBJECT

    typedef struct
    {
        QSize size;
        bool isResizable;
    } sWidgetProperties;

public:
    UBWidgetList(QWidget* parent=0, eWidgetListOrientation orientation = eWidgetListOrientation_Vertical, const char* name = "UBWidgetList");
    ~UBWidgetList();
    void addWidget(QWidget* widget);
    void removeWidget(QWidget* widget);
    void setMargin(int margin);
    void setEmptyText(const QString& text);
    int margin();
    bool empty();
    void setListElementSpacing(int margin) { mListElementsSpacing = margin; }
    int listElementsSpacing() {return mListElementsSpacing; }

protected:
    bool mCanRemove;

    void resizeEvent(QResizeEvent* ev);
    void mousePressEvent(QMouseEvent* ev);

private:
    int scaleWidgets(QSize pSize);
    void scaleContainer(QSize pSize, int updateValue);
    void updateView(QSize pSize);
    void updateSizes();
    QBoxLayout* mpLayout;
    QWidget* mpContainer;
    eWidgetListOrientation mOrientation;
    int mMargin;
    int mListElementsSpacing;
    QMap<QWidget*, QSize> mWidgetInfo;
    QLabel* mpEmptyLabel;
    QWidget* mpCurrentWidget;
};

#endif // UBWIDGETLIST_H
