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

typedef enum{
    eDuration_Quarter,
    eDuration_Half,
    eDuration_ThreeQuarter
}eDuration;

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
    QStringList mediaList() {return mMediaList;}
    void reloadMedia(QStringList pList);
    void cleanMedias();
    bool empty();

private:
    void addMedia(QString pMediaPath);
    QStringList mMediaList;
    QList<QWidget*>mWidgetList;
    UBWidgetList* mWidget;
    QVBoxLayout mLayout;


protected:
    void dragEnterEvent(QDragEnterEvent* pEvent);
    void dropEvent(QDropEvent *pEvent);
    void dragMoveEvent(QDragMoveEvent* pEvent);
    void dragLeaveEvent(QDragLeaveEvent* pEvent);
};



class UBTeacherBarPreviewMedia : public QWidget
{
    Q_OBJECT
public:
    UBTeacherBarPreviewMedia(QWidget* parent=0, const char* name="UBTeacherBarPreviewMedia");
    ~UBTeacherBarPreviewMedia();
    void loadMedia(QStringList pMedias);
    void loadWidgets(QList<QWidget*> pWidgetList);
    void cleanMedia();

private:
    UBWidgetList* mWidget;
    QVBoxLayout mLayout;
    QMap<QWidget*,QString>mWidgetList;
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

class UBTeacherBarPreviewWidget : public QWidget
{
    Q_OBJECT
public:
    UBTeacherBarPreviewWidget(QWidget* parent=0, const char* name="UBTeacherBarPreviewWidget");
    ~UBTeacherBarPreviewWidget();
    UBTeacherBarPreviewMedia* mediaViewer() {return &mMediaViewer;}
    void setTitle(const QString& title);
    void setDuration(eDuration duration);
    void setComments(const QString& comments);

signals:
    void showEditMode();

private slots:
    void onEdit();

private:
    QVBoxLayout mLayout;
    QHBoxLayout mEditLayout;
    QHBoxLayout mTitleDurationLayout;
    QHBoxLayout mActionLabelLayout;
    QHBoxLayout mMediaLabelLayout;
    QHBoxLayout mCommentsLabelLayout;
    UBTeacherBarPreviewMedia mMediaViewer;

    QPushButton* mpEditButton;
    QLabel* mpTitle;
    QLabel* mpDuration;
    QLabel* mpActionsLabel;
    QLabel* mpMediaLabel;
    QLabel* mpCommentsLabel;
    QLabel* mpComments;
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
    void onShowEditMode();

private:
    void clearWidgetLists();
    bool isEmpty();
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
    UBTeacherBarPreviewWidget* mpPreview;

    QVector<UBTeacherStudentAction*> mActionList;
    QVector<UBUrlWidget*> mUrlList;
};

#endif // UBTEACHERBARWIDGET_H
