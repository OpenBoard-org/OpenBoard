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
#include "interfaces/IDropable.h"

#define LABEL_MINWIDHT      80

typedef enum{
    eTeacherBarState_DocumentEdit,
    eTeacherBarState_DocumentPreview,
    eTeacherBarState_PageEdit,
    eTeacherBarState_PagePreview
}eTeacherBarState;

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
};

class UBTBMediaContainer : public UBWidgetList
    , public IDropable
{
    Q_OBJECT
public:
    UBTBMediaContainer(QWidget* parent=0, const char* name="UBTBMediaContainer");
    ~UBTBMediaContainer();
    QStringList mediaUrls();
    QWidget* generateMediaWidget(const QString& url);
    void cleanMedias();

signals:
    void mediaDropped(const QString& url);

protected:
    void dropEvent(QDropEvent* pEvent);
    void dragEnterEvent(QDragEnterEvent* pEvent);
    void dragMoveEvent(QDragMoveEvent* pEvent);
    void dragLeaveEvent(QDragLeaveEvent* pEvent);

private:
    void addMedia(const QString& mediaPath);

    QStringList mMediaList;
};

class UBTBDocumentEditWidget : public QWidget
{
    Q_OBJECT
public:
    UBTBDocumentEditWidget(QWidget* parent=0, const char* name="UBTBDocumentEditWidget");
    ~UBTBDocumentEditWidget();

signals:
    void onPreviewClicked();
    void onPageViewClicked();

private:
    QVBoxLayout mLayout;
    QHBoxLayout mPageLayout;
    QHBoxLayout mPreviewLayout;
    QPushButton* mpPageViewButton;
    QPushButton* mpPreviewButton;

    QLabel* mpTitleLabel;
    QLineEdit* mpTitle;
    QLabel* mpTargetLabel;
    QTextEdit* mpTarget;
    QLabel* mpMetadataLabel;
    QLabel* mpLicenseLabel;
};

class UBTBDocumentPreviewWidget : public QWidget
{
    Q_OBJECT
public:
    UBTBDocumentPreviewWidget(QWidget* parent=0, const char* name="UBTBDocumentPreviewWidget");
    ~UBTBDocumentPreviewWidget();

signals:
    void onEditClicked();
    void onPageViewClicked();

private:
    QVBoxLayout mLayout;
    QHBoxLayout mPageLayout;
    QHBoxLayout mPreviewLayout;
    QPushButton* mpPageViewButton;
    QPushButton* mpEditButton;
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
    void onMediaDropped(const QString& url);
    void onTBStateChanged(eTeacherBarState state);

private:
    void clearWidgetLists();
    bool isEmpty();
    QVBoxLayout* mpLayout;
    QHBoxLayout* mpTitleLayout;
    QLabel* mpTitleLabel;
    QLineEdit* mpTitle;
    QLabel* mpMediaLabel;
    QWidget* mpContainer;
    QVBoxLayout* mpContainerLayout;
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
    QVector<QWidget*> mMediaList;

    UBTBMediaContainer* mpMediaContainer;
    eTeacherBarState mState;

    UBTBDocumentPreviewWidget* mpDocPreviewWidget;
    UBTBDocumentEditWidget* mpDocEditWidget;
};

#endif // UBTEACHERBARWIDGET_H
