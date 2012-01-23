#ifndef UBTBDOCUMENTEDITWIDGET_H
#define UBTBDOCUMENTEDITWIDGET_H

#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>

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

private:
    QVBoxLayout mLayout;
    QHBoxLayout mPageLayout;
    QHBoxLayout mPreviewLayout;
    QVBoxLayout mContainerLayout;
    QPushButton* mpPageViewButton;
    QPushButton* mpPreviewButton;

    QWidget* mpContainer;
    QLabel* mpTitleLabel;
    QLineEdit* mpTitle;
    QLabel* mpTargetLabel;
    QTextEdit* mpTarget;
    QLabel* mpMetadataLabel;
    QLabel* mpLicenseLabel;

    UBTeacherBarDataMgr* mpDataMgr;
};

#endif // UBTBDOCUMENTEDITWIDGET_H
