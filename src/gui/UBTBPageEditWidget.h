#ifndef UBTBPAGEEDITWIDGET_H
#define UBTBPAGEEDITWIDGET_H

#include <QString>
#include <QTextEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>

#include "core/UBPersistenceManager.h"
#include "customWidgets/UBWidgetList.h"
#include "interfaces/IDropable.h"
#include "UBTeacherBarDataMgr.h"

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

class UBTBPageEditWidget : public QWidget
{
    Q_OBJECT
public:
    UBTBPageEditWidget(UBTeacherBarDataMgr* pDataMgr, QWidget* parent=0, const char* name="UBTBPageEditWidget");
    ~UBTBPageEditWidget();

    void saveInfos(sTeacherBarInfos* infos);
    void loadInfos(sTeacherBarInfos* infos);

signals:
    void valueChanged();
    void changeTBState(eTeacherBarState state);

private slots:
    void onValueChanged();
    void onActionButton();
    void onLinkButton();
    void onMediaDropped(const QString& url);
    void onDocumentEditClicked();
    void onPagePreviewClicked();

private:
    QVBoxLayout mLayout;
    QHBoxLayout mTitleLayout;
    QVBoxLayout mContainerLayout;
    QHBoxLayout mActionLayout;
    QHBoxLayout mLinkLayout;
    QHBoxLayout mDocumentViewLayout;
    QHBoxLayout mPagePreviewLayout;

    UBTeacherBarDataMgr* mpDataMgr;
    QLabel* mpTitleLabel;
    QLineEdit* mpTitle;
    QLabel* mpMediaLabel;
    UBTBMediaContainer* mpMediaContainer;
    QLabel* mpActionLabel;
    UBWidgetList* mpActions;
    QPushButton* mpActionButton;
    QLabel* mpLinkLabel;
    UBWidgetList* mpLinks;
    QPushButton* mpLinkButton;
    QLabel* mpCommentLabel;
    QTextEdit* mpComments;
    QPushButton* mpDocumentEditbutton;
    QPushButton* mpPagePreviewButton;
    QWidget* mpContainer;
};

#endif // UBTBPAGEEDITWIDGET_H
