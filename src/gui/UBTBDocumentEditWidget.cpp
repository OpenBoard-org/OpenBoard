#include "UBTBDocumentEditWidget.h"
#include "customWidgets/UBGlobals.h"

UBTBDocumentEditWidget::UBTBDocumentEditWidget(UBTeacherBarDataMgr* pDataMgr, QWidget *parent, const char *name):QWidget(parent)
  , mpPageViewButton(NULL)
  , mpPreviewButton(NULL)
  , mpTitleLabel(NULL)
  , mpTitle(NULL)
  , mpTargetLabel(NULL)
  , mpTarget(NULL)
  , mpMetadataLabel(NULL)
  , mpLicenseLabel(NULL)
{
    setObjectName(name);

    mpDataMgr = pDataMgr;

    setLayout(&mLayout);
    mLayout.setContentsMargins(0, 0, 0, 0);

    mpContainer = new QWidget(this);
    mpContainer->setObjectName("DockPaletteWidgetBox");
    mLayout.addWidget(mpContainer, 1);
    mpContainer->setLayout(&mContainerLayout);

    // Title
    mpTitleLabel = new QLabel(tr("Session Title"), mpContainer);
    mpTitleLabel->setAlignment(Qt::AlignLeft);
    //mpTitleLabel->setObjectName("UBTeacherBarPreviewSubtitle");
    mContainerLayout.addWidget(mpTitleLabel, 0);
    mpTitle = new QLineEdit(mpContainer);
    mpTitle->setObjectName("DockPaletteWidgetLineEdit");
    mContainerLayout.addWidget(mpTitle, 0);

    // Target
    mpTargetLabel = new QLabel(tr("Session Target"), mpContainer);
    //mpTargetLabel->setObjectName("UBTeacherBarPreviewSubtitle");
    mContainerLayout.addWidget(mpTargetLabel, 0);
    mpTarget = new QTextEdit(mpContainer);
    mpTarget->setObjectName("UBTeacherBarTargetBox");
    mContainerLayout.addWidget(mpTarget, 1);

    // Metadata
    mpMetadataLabel = new QLabel(tr("Metadata"), mpContainer);
    mpMetadataLabel->setAlignment(Qt::AlignLeft);
    //mpMetadataLabel->setObjectName("UBTeacherBarPreviewSubtitle");
    mContainerLayout.addWidget(mpMetadataLabel, 0);

    // License
    mpLicenseLabel = new QLabel(tr("License"), mpContainer);
    mpLicenseLabel->setAlignment(Qt::AlignLeft);
    //mpLicenseLabel->setObjectName("UBTeacherBarPreviewSubtitle");
    mContainerLayout.addWidget(mpLicenseLabel, 0);

    mpPageViewButton = new QPushButton(tr("Page View"), this);
    mpPageViewButton->setObjectName("DockPaletteWidgetButton");
    mPreviewLayout.addWidget(mpPageViewButton, 0);
    mpPreviewButton = new QPushButton(tr("Preview"), this);
    mpPreviewButton->setObjectName("DockPaletteWidgetButton");
    mPreviewLayout.addWidget(mpPreviewButton, 0);
    mPreviewLayout.addStretch(1);
    mLayout.addLayout(&mPreviewLayout, 0);

    connect(mpPageViewButton, SIGNAL(clicked()), this, SLOT(onPageView()));
    connect(mpPreviewButton, SIGNAL(clicked()), this, SLOT(onPreview()));
    connect(mpTitle, SIGNAL(textChanged(QString)), this, SLOT(onSessionTitleChanged()));
    connect(mpTarget, SIGNAL(textChanged()), this, SLOT(onSessionTargetChanged()));
}

UBTBDocumentEditWidget::~UBTBDocumentEditWidget()
{
    DELETEPTR(mpTitleLabel);
    DELETEPTR(mpTitle);
    DELETEPTR(mpTargetLabel);
    DELETEPTR(mpTarget);
    DELETEPTR(mpMetadataLabel);
    DELETEPTR(mpLicenseLabel);
    DELETEPTR(mpPageViewButton);
    DELETEPTR(mpPreviewButton);
}

void UBTBDocumentEditWidget::onPageView()
{
    emit changeTBState(eTeacherBarState_PageEdit);
}

void UBTBDocumentEditWidget::onPreview()
{
    emit changeTBState(eTeacherBarState_DocumentPreview);
}

void UBTBDocumentEditWidget::onSessionTitleChanged()
{
    mpDataMgr->setSessionTitle(mpTitle->text());
    emit valueChanged();
}

void UBTBDocumentEditWidget::onSessionTargetChanged()
{
    mpDataMgr->setSessionTarget(mpTarget->document()->toPlainText());
    emit valueChanged();
}

void UBTBDocumentEditWidget::updateFields()
{
    mpTitle->setText(mpDataMgr->sessionTitle());
    mpTarget->setPlainText(mpDataMgr->sessionTarget());
}

void UBTBDocumentEditWidget::clearFields()
{
    mpTitle->setText("");
    mpTarget->setPlainText("");
}


