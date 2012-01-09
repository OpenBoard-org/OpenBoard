#ifndef UBWIDGETLIST_H
#define UBWIDGETLIST_H

#include <QWidget>
#include <QScrollArea>
#include <QLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QResizeEvent>
#include <QVector>
#include <QLabel>

typedef enum{
    eWidgetListOrientation_Vertical,
    eWidgetListOrientation_Horizontal
}eWidgetListOrientation;


class UBWidgetList : public QScrollArea
{
    Q_OBJECT

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
    void resizeEvent(QResizeEvent* ev);

private:
    int scaleWidgets(QSize pSize);
    void scaleContainer(QSize pSize, int updateValue);
    void updateView(QSize pSize);
    QLayout* mpLayout;
    QWidget* mpContainer;
    eWidgetListOrientation mOrientation;
    int mMargin;
    int mListElementsSpacing;
    QMap<QWidget*,QSize> mWidgetInfo;
    QLabel* mpEmptyLabel;
};

#endif // UBWIDGETLIST_H
