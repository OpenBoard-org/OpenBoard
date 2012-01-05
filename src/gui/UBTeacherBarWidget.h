#ifndef UBTEACHERBARWIDGET_H
#define UBTEACHERBARWIDGET_H

class UBMediaPlayer;

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QLineEdit>
#include <QCheckBox>
#include <QTabWidget>
#include <QButtonGroup>
#include <QPushButton>
#include <QComboBox>
#include <QStackedWidget>

#include "UBDockPaletteWidget.h"
#include "customWidgets/UBWidgetList.h"

#define LABEL_MINWIDHT      80

class UBTeacherStudentAction : public QWidget
{
    Q_OBJECT

public:
    UBTeacherStudentAction(QWidget* parent=0, const char* name="UBTeacherStudentAction");
    ~UBTeacherStudentAction();
    QString comboValue();
    QString text();
    void setComboValue(int value);
    void setText(const QString& text);

private:
    QTextEdit* mpText;
    QHBoxLayout* mpLayout;
    QVBoxLayout* mpComboLayout;
    QComboBox* mpCombo;
};


class UBTeacherBarDropMediaZone : public QWidget
{
    Q_OBJECT

public:
    UBTeacherBarDropMediaZone(QWidget* parent=0, const char* name="UBTeacherBarDropMediaZone");
    ~UBTeacherBarDropMediaZone();
    bool empty();

private:
    QList<QWidget*>mWidgetList;
    UBWidgetList* mWidget;
    QVBoxLayout mLayout;


protected:
    void dragEnterEvent(QDragEnterEvent* pEvent);
    void dropEvent(QDropEvent *pEvent);
    void dragMoveEvent(QDragMoveEvent* pEvent);
    void dragLeaveEvent(QDragLeaveEvent* pEvent);
};

class UBUrlWidget : public QWidget
{
public:
    UBUrlWidget(QWidget* parent=0, const char* name="UBUrlWidget");
    ~UBUrlWidget();

    QString url();
    void setUrl(const QString& url);

private:
    QHBoxLayout* mpLayout;
    QLabel* mpUrlLabel;
    QLineEdit* mpUrl;
};

class UBTeacherBarWidget : public UBDockPaletteWidget
{
    Q_OBJECT
public:
    UBTeacherBarWidget(QWidget* parent=0, const char* name="UBTeacherBarWidget");
    ~UBTeacherBarWidget();

private slots:
    void saveContent();
    void loadContent();
    void onValueChanged();
    void onTitleTextChanged(const QString& text);
    void onActionButton();
    void onLinkButton();

private:
    void clearWidgetLists();
    QVBoxLayout* mpLayout;
    QHBoxLayout* mpTitleLayout;
    QHBoxLayout* mpDurationLayout;
    QLabel* mpTitleLabel;
    QLabel* mpDurationLabel;
    QLineEdit* mpTitle;
    QLabel* mpMediaLabel;
    UBTeacherBarDropMediaZone* mpDropMediaZone;
    QWidget* mpContainer;
    QVBoxLayout* mpContainerLayout;
    QCheckBox* mpDuration1;
    QCheckBox* mpDuration2;
    QCheckBox* mpDuration3;
    QButtonGroup* mpDurationButtons;
    QLabel* mpActionLabel;
    UBWidgetList* mpActions;
    QPushButton* mpActionButton;
    QHBoxLayout* mpActionLayout;
    QLabel* mpLinkLabel;
    UBWidgetList* mpLinks;
    QPushButton* mpLinkButton;
    QHBoxLayout* mpLinkLayout;
    QLabel* mpCommentLabel;
    QTextEdit* mpComments;
    QStackedWidget* mpStackWidget;

    QVector<UBTeacherStudentAction*> mActionList;
    QVector<UBUrlWidget*> mUrlList;
};

#endif // UBTEACHERBARWIDGET_H
