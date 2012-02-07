#ifndef UBTBDOCUMENTPREVIEWWIDGET_H
#define UBTBDOCUMENTPREVIEWWIDGET_H

#include <QVBoxLayout>
#include <QPushButton>

#include "UBTeacherBarDataMgr.h"

class UBTBDocumentPreviewWidget : public QWidget
{
    Q_OBJECT
public:
    UBTBDocumentPreviewWidget(UBTeacherBarDataMgr* pDataMgr, QWidget* parent=0, const char* name="UBTBDocumentPreviewWidget");
    ~UBTBDocumentPreviewWidget();
    void updateFields();
    void clearFields();

signals:
    void changeTBState(eTeacherBarState state);

private slots:
    void onPageView();
    void onEdit();

private:
    QVBoxLayout mLayout;
    QHBoxLayout mButtonsLayout;

    QWidget* mpContainer;
    QVBoxLayout mContainerLayout;
    QPushButton* mpPageViewButton;
    QPushButton* mpEditButton;
    QLabel* mpSessionLabel;
    QLabel* mpSessionTitle;
    UBTBSeparator mTitleSeparator;
    QLabel* mpAuthorLabel;
    QLabel* mpAuthors;
    UBTBSeparator mAuthorSeparator;
    QLabel* mpCreationDate;
    UBTBSeparator mDateSeparator;
    QLabel* mpTargetLabel;
    QTextEdit* mpTarget;
    UBTBSeparator mTargetSeparator;
    QLabel* mpMetadataLabel;
    QHBoxLayout mKeywordLayout;
    QLabel* mpKeywordLabel;
    QLabel* mpKeyword;
    QHBoxLayout mLevelLayout;
    QLabel* mpLevelLabel;
    QLabel* mpLevel;
    QHBoxLayout mTopicLayout;
    QLabel* mpTopicLabel;
    QLabel* mpTopic;
    UBTBSeparator mMetadataSeparator;
    UBTBLicenseWidget* mpLicense;

    UBTeacherBarDataMgr* mpDataMgr;
};

#endif // UBTBDOCUMENTPREVIEWWIDGET_H
