#include "customWidgets/UBGlobals.h"
#include "UBTBDocumentPreviewWidget.h"

UBTBDocumentPreviewWidget::UBTBDocumentPreviewWidget(UBTeacherBarDataMgr *pDataMgr, QWidget *parent, const char *name):QWidget(parent)
  , mpPageViewButton(NULL)
  , mpEditButton(NULL)
  , mpSessionLabel(NULL)
  , mpSessionTitle(NULL)
  , mpAuthorLabel(NULL)
  , mpAuthors(NULL)
  , mpCreationDate(NULL)
  , mpTargetLabel(NULL)
  , mpTarget(NULL)
  , mpMetadataLabel(NULL)
  , mpKeywordLabel(NULL)
  , mpKeyword(NULL)
  , mpLevelLabel(NULL)
  , mpLevel(NULL)
  , mpTopicLabel(NULL)
  , mpTopic(NULL)
  , mpLicense(NULL)
  , mpContainer(NULL)
{
    setObjectName(name);
    mpDataMgr = pDataMgr;

    setLayout(&mLayout);
    mLayout.setContentsMargins(0, 0, 0, 0);

    mpContainer = new QWidget(this);
    mpContainer->setLayout(&mContainerLayout);
    mpContainer->setObjectName("UBTBPreviewWidget");

    // Session Title
    mpSessionLabel = new QLabel(tr("Session"), mpContainer);
    mpSessionLabel->setAlignment(Qt::AlignRight);
    mpSessionLabel->setObjectName("UBTeacherBarPreviewSubtitle");
    mContainerLayout.addWidget(mpSessionLabel, 0);
    mpSessionTitle = new QLabel(mpContainer);
    mpSessionTitle->setWordWrap(true);
    mpSessionTitle->setAlignment(Qt::AlignRight);
    mpSessionTitle->setObjectName("UBTeacherBarPreviewTitle");
    mContainerLayout.addWidget(mpSessionTitle, 0);
    mContainerLayout.addWidget(&mTitleSeparator, 0);

    // Author(s)
    mpAuthorLabel = new QLabel(tr("Author(s)"), mpContainer);
    mpAuthorLabel->setObjectName("UBTeacherBarPreviewSubtitle");
    mContainerLayout.addWidget(mpAuthorLabel, 0);
    mpAuthors = new QLabel(mpContainer);
    mpAuthors->setWordWrap(true);
    mpAuthors->setStyleSheet("padding-left:5px;");
    mContainerLayout.addWidget(mpAuthors, 0);
    mContainerLayout.addWidget(&mAuthorSeparator, 0);

    // Dates
    mpCreationDate = new QLabel(tr("Created on: "), mpContainer);
    mpCreationDate->setStyleSheet("padding-left:5px;");
    mContainerLayout.addWidget(mpCreationDate);
    mContainerLayout.addWidget(&mDateSeparator, 0);

    // Target
    mpTargetLabel = new QLabel(tr("Target"), mpContainer);
    mpTargetLabel->setObjectName("UBTeacherBarPreviewSubtitle");
    mContainerLayout.addWidget(mpTargetLabel,0);
    mpTarget = new QTextEdit(mpContainer);
    mpTarget->setReadOnly(true);
    mContainerLayout.addWidget(mpTarget, 1);
    mContainerLayout.addWidget(&mTargetSeparator, 0);

    // Metadata
    mpMetadataLabel = new QLabel(tr("Metadata"), mpContainer);
    mpMetadataLabel->setObjectName("UBTeacherBarPreviewSubtitle");
    mContainerLayout.addWidget(mpMetadataLabel, 0);

    // Keyword
    mpKeywordLabel = new QLabel(tr("<b>Keywords:</b>"), mpContainer);
    mpKeywordLabel->setStyleSheet("padding-left:5px;");
    mpKeyword = new QLabel(mpContainer);
    mpKeyword->setWordWrap(true);
    mKeywordLayout.addWidget(mpKeywordLabel, 0);
    mKeywordLayout.addWidget(mpKeyword, 1);
    mContainerLayout.addLayout(&mKeywordLayout, 0);

    // Level
    mpLevelLabel = new QLabel(tr("<b>Level:</b>"), mpContainer);
    mpLevelLabel->setStyleSheet("padding-left:5px;");
    mpLevel = new QLabel(mpContainer);
    mpLevel->setWordWrap(true);
    mLevelLayout.addWidget(mpLevelLabel, 0);
    mLevelLayout.addWidget(mpLevel, 1);
    mContainerLayout.addLayout(&mLevelLayout, 0);

    // Topic
    mpTopicLabel = new QLabel(tr("<b>Topic:</b>"), mpContainer);
    mpTopicLabel->setStyleSheet("padding-left:5px;");
    mpTopic = new QLabel(mpContainer);
    mpTopic->setWordWrap(true);
    mTopicLayout.addWidget(mpTopicLabel, 0);
    mTopicLayout.addWidget(mpTopic, 1);
    mContainerLayout.addLayout(&mTopicLayout, 0);
    mContainerLayout.addWidget(&mMetadataSeparator, 0);

    // License
    mpLicense = new UBTBLicenseWidget(mpContainer);
    mContainerLayout.addWidget(mpLicense, 0);

    mLayout.addWidget(mpContainer, 1);

    mpPageViewButton = new QPushButton(tr("Page View"), this);
    mpPageViewButton->setObjectName("DockPaletteWidgetButton");
    mpEditButton = new QPushButton(tr("Edit"), this);
    mpEditButton->setObjectName("DockPaletteWidgetButton");

    mButtonsLayout.addWidget(mpPageViewButton, 0);
    mButtonsLayout.addWidget(mpEditButton, 0);
    mButtonsLayout.addStretch(1);

    mLayout.addLayout(&mButtonsLayout, 0);

    connect(mpPageViewButton, SIGNAL(clicked()), this, SLOT(onPageView()));
    connect(mpEditButton, SIGNAL(clicked()), this, SLOT(onEdit()));
}

UBTBDocumentPreviewWidget::~UBTBDocumentPreviewWidget()
{

}

void UBTBDocumentPreviewWidget::onEdit()
{
    emit changeTBState(eTeacherBarState_DocumentEdit);
}

void UBTBDocumentPreviewWidget::onPageView()
{
    emit changeTBState(eTeacherBarState_PagePreview);
}

void UBTBDocumentPreviewWidget::updateFields()
{
    mpSessionTitle->setText(mpDataMgr->sessionTitle());
    mpAuthors->setText(mpDataMgr->authors());
    mpCreationDate->setText(tr("<b>Creation Date:</b> %0").arg(mpDataMgr->creationDate()));
    mpTarget->setText(mpDataMgr->sessionTarget());
    mpLicense->setLicense(mpDataMgr->sessionLicence());
    mpKeyword->setText(mpDataMgr->keywords());
    mpLevel->setText(mpDataMgr->level());
    mpTopic->setText(mpDataMgr->topic());
}

void UBTBDocumentPreviewWidget::clearFields()
{
    mpSessionTitle->setText("");
    mpAuthors->setText("");
    mpCreationDate->setText("");
    mpTarget->setText("");
    mpKeyword->setText("");
    mpLevel->setText("");
    mpTopic->setText("");
}
