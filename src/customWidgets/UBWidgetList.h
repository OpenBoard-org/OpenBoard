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

protected:
    void resizeEvent(QResizeEvent* ev);

private:
    void updateSize(bool widgetAdded, QWidget* widget);
    void updateAllWidgetsize(float scale);
    QLayout* mpLayout;
    QWidget* mpContainer;
    eWidgetListOrientation mOrientation;
    int mMargin;
    QVector<QWidget*> mWidgets;
    QLabel* mpEmptyLabel;
};

#endif // UBWIDGETLIST_H
