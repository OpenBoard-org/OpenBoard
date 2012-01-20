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
    QHBoxLayout mPageLayout;
    QHBoxLayout mPreviewLayout;
    QPushButton* mpPageViewButton;
    QPushButton* mpEditButton;

    UBTeacherBarDataMgr* mpDataMgr;
};

#endif // UBTBDOCUMENTPREVIEWWIDGET_H
