#ifndef UBUPDATEDLG_H
#define UBUPDATEDLG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QHBoxLayout>

class UBUpdateDlg : public QDialog
{
    Q_OBJECT

public:
    UBUpdateDlg(QWidget *parent = 0, int nbFiles = 0, const QString& bkpPath = "");
    ~UBUpdateDlg();
    QString backupPath();

public slots:
    void onFilesUpdated(bool bResult);

signals:
    void updateFiles();

private slots:
    void onBrowse();
    void onUpdate();

private:
    QLabel mNbFilesLabel;
    QLabel mBkpLabel;
    QLineEdit mBkpPath;
    QPushButton mBrowseBttn;
    QDialogButtonBox* mpDlgBttn;
    QVBoxLayout mLayout;
    QHBoxLayout mHLayout;
};

#endif // UBUPDATEDLG_H
