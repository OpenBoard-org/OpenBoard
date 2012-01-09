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

typedef enum{
    eActionOwner_Teacher,
    eActionOwner_Student
}eActionOwner;

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
    int loadMedia(QStringList pMedias);
    void loadWidgets(QList<QWidget*> pWidgetList, bool isResizable = true);
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
    QVBoxLayout* mpLayout;
    QHBoxLayout* mpLabelLayout;
    QHBoxLayout* mpTitleLayout;
    QLabel* mpUrlLabel;
    QLineEdit* mpUrl;

    QLabel* mpTitleLabel;
    QLineEdit* mpTitle;
};

class UBActionPreview : public QWidget
{
public:
    UBActionPreview(QWidget* parent=0, const char* name="UBActionPreview");
    ~UBActionPreview();
    void setOwner(const QString& owner);
    void setContent(const QString& content);

private:
    QLabel* mpOwner;
    QLabel* mpContent;

    QVBoxLayout mLayout;
    QHBoxLayout mOwnerLayout;
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
    void setActions(QStringList actions);
    void setLinks(QStringList links);
    void clean();
    QLabel* mediaLabel() { return mpMediaLabel;}

signals:
    void showEditMode();

private slots:
    void onEdit();

private:
    void hideElements();

    QVBoxLayout mLayout;
    QHBoxLayout mEditLayout;
    QHBoxLayout mTitleDurationLayout;
    UBTeacherBarPreviewMedia mMediaViewer;
    QList<QWidget*> mWidgets;

    QPushButton* mpEditButton;
    QLabel* mpTitle;
    QLabel* mpDuration;
    QLabel* mpActionsLabel;
    QLabel* mpMediaLabel;
    QLabel* mpCommentsLabel;
    QLabel* mpComments;
    QLabel* mpLinksLabel;
    QLabel* mpTmpLink;
    UBActionPreview* mpTmpAction;
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
