#ifndef UBTBDOCUMENTEDITWIDGET_H
#define UBTBDOCUMENTEDITWIDGET_H

#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>

#include "UBTeacherBarDataMgr.h"

class UBTBDocumentEditWidget : public QWidget
{
    Q_OBJECT
public:
    UBTBDocumentEditWidget(UBTeacherBarDataMgr* pDataMgr, QWidget* parent=0, const char* name="UBTBDocumentEditWidget");
    ~UBTBDocumentEditWidget();
    void updateFields();
    void clearFields();

signals:
    void changeTBState(eTeacherBarState state);
    void valueChanged();

private slots:
    void onPageView();
    void onPreview();
    void onSessionTitleChanged();
    void onSessionTargetChanged();
    void onLicenseCurrentIndexChanged(int selection);
    void onKeywordChanged(const QString& kw);
    void onLevelChanged(const QString& level);
    void onTopicChanged(const QString& topic);
    void onAuthorChanged(const QString& authors);

private:
    QVBoxLayout mLayout;
    QHBoxLayout mPageLayout;
    QHBoxLayout mPreviewLayout;
    QVBoxLayout mContainerLayout;
    QPushButton* mpPageViewButton;
    QPushButton* mpPreviewButton;
    UBTBSeparator mTitleSeparator;
    UBTBSeparator mTargetSeparator;
    UBTBSeparator mLicenseSeparator;

    QHBoxLayout mKeywordLayout;
    QHBoxLayout mLevelLayout;
    QHBoxLayout mTopicLayout;
    QHBoxLayout mAuthorLayout;

    QWidget* mpContainer;
    QLabel* mpTitleLabel;
    QLineEdit* mpTitle;
    QLabel* mpTargetLabel;
    QTextEdit* mpTarget;
    QLabel* mpMetadataLabel;
    QLabel* mpLicenseLabel;
    QComboBox* mpLicenseCombox;
    QLineEdit* mpKeywords;
    QComboBox* mpLevel;
    QComboBox* mpTopic;
    QLineEdit* mpAuthor;
    QLabel* mpKeywordLabel;
    QLabel* mpLevelLabel;
    QLabel* mpTopicLabel;
    QLabel* mpAuthorLabel;

    UBTeacherBarDataMgr* mpDataMgr;
};

#endif // UBTBDOCUMENTEDITWIDGET_H
