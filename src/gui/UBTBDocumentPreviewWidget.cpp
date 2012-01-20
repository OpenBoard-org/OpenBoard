#include "customWidgets/UBGlobals.h"
#include "UBTBDocumentPreviewWidget.h"

UBTBDocumentPreviewWidget::UBTBDocumentPreviewWidget(UBTeacherBarDataMgr *pDataMgr, QWidget *parent, const char *name):QWidget(parent)
  , mpPageViewButton(NULL)
  , mpEditButton(NULL)
{
    setObjectName(name);
    mpDataMgr = pDataMgr;

    setLayout(&mLayout);

    mpPageViewButton = new QPushButton(tr("Page View"), this);
    mpPageViewButton->setObjectName("DockPaletteWidgetButton");
    mPageLayout.addStretch(1);
    mPageLayout.addWidget(mpPageViewButton, 0);
    mPageLayout.addStretch(1);
    mLayout.addLayout(&mPageLayout);

    // TODO : Add the elements here

    mpEditButton = new QPushButton(tr("Edit"), this);
    mpEditButton->setObjectName("DockPaletteWidgetButton");
    mPreviewLayout.addStretch(1);
    mPreviewLayout.addWidget(mpEditButton, 0);
    mPreviewLayout.addStretch(1);
    mLayout.addLayout(&mPreviewLayout);

    connect(mpPageViewButton, SIGNAL(clicked()), this, SLOT(onPageView()));
    connect(mpEditButton, SIGNAL(clicked()), this, SLOT(onEdit()));
}

UBTBDocumentPreviewWidget::~UBTBDocumentPreviewWidget()
{
    DELETEPTR(mpPageViewButton);
    DELETEPTR(mpEditButton);
}

void UBTBDocumentPreviewWidget::onEdit()
{
    emit changeTBState(eTeacherBarState_DocumentEdit);
}

void UBTBDocumentPreviewWidget::onPageView()
{
    emit changeTBState(eTeacherBarState_PagePreview);
}

