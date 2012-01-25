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

class UBTeacherStudentAction : public QWidget
{
    Q_OBJECT

public:
    UBTeacherStudentAction(QWidget* parent=0, const char* name="UBTeacherStudentAction");
    ~UBTeacherStudentAction();
    QString text();
    QString comboValue();
    void setComboValue(int value);
    void setText(const QString& text);

private:
    QTextEdit* mpText;
    QHBoxLayout* mpLayout;
    QVBoxLayout* mpComboLayout;
    QComboBox* mpCombo;
};

class UBUrlWidget : public QWidget
{
public:
    UBUrlWidget(QWidget* parent=0, const char* name="UBUrlWidget");
    ~UBUrlWidget();

    QString url();
    void setUrl(const QString& url);

    QString title(){return mpTitle->text();}
    void setTitle(const QString& title){mpTitle->setText(title);}

private:
    QVBoxLayout* mpLayout;
    QHBoxLayout* mpLabelLayout;
    QHBoxLayout* mpTitleLayout;
    QLabel* mpUrlLabel;
    QLineEdit* mpUrl;

    QLabel* mpTitleLabel;
    QLineEdit* mpTitle;
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

class UBTBPageEditWidget : public QWidget
{
    Q_OBJECT
public:
    UBTBPageEditWidget(UBTeacherBarDataMgr* pDataMgr, QWidget* parent=0, const char* name="UBTBPageEditWidget");
    ~UBTBPageEditWidget();
    void saveFields();
    void updateFields();
    void clearFields();

signals:
    void valueChanged();
    void changeTBState(eTeacherBarState state);

private slots:
    void onTitleChanged();
    void onCommentsChanged();
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

    QVector<UBTeacherStudentAction*> mActions;
    QVector<UBUrlWidget*> mUrls;
    QVector<QWidget*> mMedias;
    QStringList mMediaUrls;
    bool mClearingFields;
};

#endif // UBTBPAGEEDITWIDGET_H
