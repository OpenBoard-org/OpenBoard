#include <QDebug>

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
  , mpLicenseCombox(NULL)
  , mpKeywords(NULL)
  , mpLevel(NULL)
  , mpTopic(NULL)
  , mpAuthor(NULL)
  , mpKeywordLabel(NULL)
  , mpLevelLabel(NULL)
  , mpTopicLabel(NULL)
  , mpAuthorLabel(NULL)
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
    mpTitleLabel->setObjectName("UBTeacherBarPreviewSubtitle");
    mContainerLayout.addWidget(mpTitleLabel, 0);
    mpTitle = new QLineEdit(mpContainer);
    mpTitle->setObjectName("DockPaletteWidgetLineEdit");
    mContainerLayout.addWidget(mpTitle, 0);
    mContainerLayout.addWidget(&mTitleSeparator);

    // Target
    mpTargetLabel = new QLabel(tr("Session Target"), mpContainer);
    mpTargetLabel->setObjectName("UBTeacherBarPreviewSubtitle");
    mContainerLayout.addWidget(mpTargetLabel, 0);
    mpTarget = new QTextEdit(mpContainer);
    mpTarget->setObjectName("UBTeacherBarTargetBox");
    mContainerLayout.addWidget(mpTarget, 1);
    mContainerLayout.addWidget(&mTargetSeparator);

    // Metadata
    mpMetadataLabel = new QLabel(tr("Metadata"), mpContainer);
    mpMetadataLabel->setObjectName("UBTeacherBarPreviewSubtitle");
    mpMetadataLabel->setAlignment(Qt::AlignLeft);
    mContainerLayout.addWidget(mpMetadataLabel, 0);
    mpKeywords = new QLineEdit(this);
    mpKeywords->setObjectName("DockPaletteWidgetLineEdit");
    mpLevel = new QComboBox(this);
    mpLevel->setObjectName("DockPaletteWidgetComboBox");
    mpTopic = new QComboBox(this);
    mpTopic->setObjectName("DockPaletteWidgetComboBox");
    mpAuthor = new QLineEdit(this);
    mpAuthor->setObjectName("DockPaletteWidgetLineEdit");
    mpKeywordLabel = new QLabel(tr("Keywords:"), this);
    mpLevelLabel = new QLabel(tr("Level:"), this);
    mpTopicLabel = new QLabel(tr("Topic:"), this);
    mpAuthorLabel = new QLabel(tr("Author"), this);

    mKeywordLayout.addWidget(mpKeywordLabel, 0);
    mKeywordLayout.addWidget(mpKeywords, 1);
    mLevelLayout.addWidget(mpLevelLabel, 0);
    mLevelLayout.addWidget(mpLevel, 1);
    mTopicLayout.addWidget(mpTopicLabel, 0);
    mTopicLayout.addWidget(mpTopic, 1);
    mAuthorLayout.addWidget(mpAuthorLabel, 0);
    mAuthorLayout.addWidget(mpAuthor, 1);

    mContainerLayout.addLayout(&mKeywordLayout, 0);
    mContainerLayout.addLayout(&mLevelLayout, 0);
    mContainerLayout.addLayout(&mTopicLayout, 0);
    mContainerLayout.addLayout(&mAuthorLayout, 0);

    mContainerLayout.addWidget(&mLicenseSeparator);

    // License
    mpLicenseLabel = new QLabel(tr("License"), mpContainer);
    mpLicenseLabel->setAlignment(Qt::AlignLeft);
    mpLicenseLabel->setObjectName("UBTeacherBarPreviewSubtitle");
    mContainerLayout.addWidget(mpLicenseLabel, 0);
    mpLicenseCombox = new QComboBox(this);
    mpLicenseCombox->setObjectName("DockPaletteWidgetComboBox");
    QStringList qslLicenses;
    qslLicenses << "CC BY";
    qslLicenses << "CC BY-ND";
    qslLicenses << "CC BY-NC-SA";
    qslLicenses << "CC BY-SA";
    qslLicenses << "CC BY-NC";
    qslLicenses << "CC BY-NC-ND";
    mpLicenseCombox->addItems(qslLicenses);
    mContainerLayout.addWidget(mpLicenseCombox);

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
    connect(mpLicenseCombox, SIGNAL(currentIndexChanged(int)), this, SLOT(onLicenseCurrentIndexChanged(int)));
    connect(mpKeywords, SIGNAL(textChanged(QString)), this, SLOT(onKeywordChanged(QString)));
    connect(mpLevel, SIGNAL(currentIndexChanged(QString)), this, SLOT(onLevelChanged(QString)));
    connect(mpTopic, SIGNAL(currentIndexChanged(QString)), this, SLOT(onTopicChanged(QString)));
    connect(mpAuthor, SIGNAL(textChanged(QString)), this, SLOT(onAuthorChanged(QString)));
}

UBTBDocumentEditWidget::~UBTBDocumentEditWidget()
{
    DELETEPTR(mpTitleLabel);
    DELETEPTR(mpTitle);
    DELETEPTR(mpTargetLabel);
    DELETEPTR(mpTarget);
    DELETEPTR(mpMetadataLabel);
    DELETEPTR(mpKeywordLabel);
    DELETEPTR(mpLevelLabel);
    DELETEPTR(mpTopicLabel);
    DELETEPTR(mpAuthorLabel);
    DELETEPTR(mpKeywords);
    DELETEPTR(mpLevel);
    DELETEPTR(mpTopic);
    DELETEPTR(mpAuthor);
    DELETEPTR(mpLicenseLabel);
    DELETEPTR(mpLicenseCombox);
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

void UBTBDocumentEditWidget::onLicenseCurrentIndexChanged(int selection)
{
    mpDataMgr->setSessionLicence((eLicense)selection);
    emit valueChanged();
}

void UBTBDocumentEditWidget::updateFields()
{
    mpTitle->setText(mpDataMgr->sessionTitle());
    mpTarget->setPlainText(mpDataMgr->sessionTarget());
    mpKeywords->setText(mpDataMgr->keywords());
    // TODO: retrieve the level
    // TODO retrieve the topic
    mpAuthor->setText(mpDataMgr->authors());
}

void UBTBDocumentEditWidget::clearFields()
{
    mpTitle->setText("");
    mpTarget->setPlainText("");
}

void UBTBDocumentEditWidget::onKeywordChanged(const QString &kw)
{
    mpDataMgr->setKeywords(kw);
    qDebug() << ">>> KEYWORD CHANGED: " << kw;
    emit valueChanged();
}

void UBTBDocumentEditWidget::onLevelChanged(const QString &level)
{
    mpDataMgr->setLevel(level);
    emit valueChanged();
}

void UBTBDocumentEditWidget::onTopicChanged(const QString &topic)
{
    mpDataMgr->setTopic(topic);
    emit valueChanged();
}

void UBTBDocumentEditWidget::onAuthorChanged(const QString &authors)
{
    mpDataMgr->setAuthors(authors);
    emit valueChanged();
}
