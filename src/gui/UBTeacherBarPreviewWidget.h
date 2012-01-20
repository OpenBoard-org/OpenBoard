#ifndef UBTEACHERBARPREVIEWWIDGET_H
#define UBTEACHERBARPREVIEWWIDGET_H

#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>

#include "core/UBPersistenceManager.h"
#include "customWidgets/UBWidgetList.h"
#include "UBTeacherBarDataMgr.h"

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

class UBTBPreviewContainer : public UBWidgetList
{
public:
    UBTBPreviewContainer(QWidget* parent=0, const char* name="UBTBPreviewContainer");
    ~UBTBPreviewContainer();
};

class UBTeacherBarPreviewWidget : public QWidget
{
    Q_OBJECT
public:
    UBTeacherBarPreviewWidget(UBTeacherBarDataMgr* pDataMgr, QWidget* parent=0, const char* name="UBTeacherBarPreviewWidget");
    ~UBTeacherBarPreviewWidget();
    UBTeacherBarPreviewMedia* mediaViewer() {return &mMediaViewer;}
    void setTitle(const QString& title);
    void setComments(const QString& comments);
    void setActions(QStringList actions);
    void setLinks(QStringList links);
    void clean();
    QLabel* mediaLabel() { return mpMediaLabel;}
    void loadInfos(sTeacherBarInfos* infos);

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
    QList<QWidget*> mStoredWidgets;

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
    UBTBPreviewContainer* mpContentContainer;
    UBTeacherBarDataMgr* mpDataMgr;
};

#endif // UBTEACHERBARPREVIEWWIDGET_H
