#ifndef UBACTIONABLEWIDGET_H
#define UBACTIONABLEWIDGET_H

#include <QWidget>
#include <QPaintEvent>

#define ACTIONSIZE  16

typedef enum{
    eAction_Close,
    eAction_MoveUp,
    eAction_MoveDown
}eAction;

class UBActionableWidget : public QWidget
{
    Q_OBJECT
public:
    UBActionableWidget(QWidget* parent=0, const char* name="UBActionableWidget");
    ~UBActionableWidget();
    void addAction(eAction act);
    void removeAction(eAction act);
    void removeAllActions();
    void setActionsVisible(bool bVisible);
    bool shouldClose(QPoint p);

protected:
    void paintEvent(QPaintEvent* ev);
    QVector<eAction> mActions;

private:
    bool mShowActions;
};

#endif // UBACTIONABLEWIDGET_H
