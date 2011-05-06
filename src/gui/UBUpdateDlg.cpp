#include <QMessageBox>
#include <QFileDialog>

#include "UBUpdateDlg.h"

UBUpdateDlg::UBUpdateDlg(QWidget *parent, int nbFiles, const QString& bkpPath)
    : QDialog(parent)
    , mpDlgBttn(NULL)
{
    setFixedSize(400, 110);
    setModal(true);
    setWindowTitle(tr("Document updater"));
    setLayout(&mLayout);
    QString str = QString::number(nbFiles);
    str.append(tr(" files require an update."));
    mNbFilesLabel.setText(str);
    mLayout.addWidget(&mNbFilesLabel);
    mBkpLabel.setText(tr("Backup path: "));
    mBkpPath.setText(bkpPath);
    mBrowseBttn.setText(tr("Browse"));
    mHLayout.addWidget(&mBkpLabel);
    mHLayout.addWidget(&mBkpPath, 1);
    mHLayout.addWidget(&mBrowseBttn);
    mLayout.addLayout(&mHLayout);

    mpDlgBttn = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    mLayout.addWidget(mpDlgBttn);

    mpDlgBttn->button(QDialogButtonBox::Ok)->setText(tr("Update"));
    mpDlgBttn->button(QDialogButtonBox::Cancel)->setText("Remind me later");

    QObject::connect(&mBrowseBttn, SIGNAL(clicked()), this, SLOT(onBrowse()));
    QObject::connect(mpDlgBttn, SIGNAL(accepted()), this, SLOT(onUpdate()));
    QObject::connect(mpDlgBttn, SIGNAL(rejected()), this, SLOT(reject()));
}

UBUpdateDlg::~UBUpdateDlg()
{
    if(NULL != mpDlgBttn)
    {
        delete mpDlgBttn;
        mpDlgBttn = NULL;
    }
}

void UBUpdateDlg::onBrowse()
{
    QString qsSelectedDir;
    qsSelectedDir = QFileDialog::getExistingDirectory(this, tr("Select a backup folder"), mBkpPath.text());
    mBkpPath.setText(qsSelectedDir);
}

void UBUpdateDlg::onUpdate()
{
    emit updateFiles();
}

void UBUpdateDlg::onFilesUpdated(bool bResult)
{
    QString qsMsg;

    if(bResult)
    {
        qsMsg = tr("Files update successful!\nPlease reboot the application to access the updated documents.");
    }
    else
    {
        qsMsg = tr("An error occured during the update. The files have not been affected.");
    }
    QMessageBox::information(this, tr("Files update results"), qsMsg, QMessageBox::Ok);
}

QString UBUpdateDlg::backupPath()
{
    return mBkpPath.text();
}
