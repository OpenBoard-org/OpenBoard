#ifndef UBACTIONABLEWIDGET_H
#define UBACTIONABLEWIDGET_H

#include <QWidget>
#include <QPaintEvent>
#include <QToolButton>
#include <QPushButton>

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

signals:
    void close(QWidget* w);

protected:
    void setActionsParent(QWidget* parent);
    void unsetActionsParent();
    QVector<eAction> mActions;
    QPushButton mCloseButtons;

private slots:
    void onCloseClicked();

private:
    bool mShowActions;

};

#endif // UBACTIONABLEWIDGET_H
