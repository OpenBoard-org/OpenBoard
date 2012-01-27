#ifndef UBTEACHERBARDATAMGR_H
#define UBTEACHERBARDATAMGR_H

#include <QString>
#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QComboBox>
#include <QFrame>

typedef enum{
    eTeacherBarState_DocumentEdit,
    eTeacherBarState_DocumentPreview,
    eTeacherBarState_PageEdit,
    eTeacherBarState_PagePreview
}eTeacherBarState;

typedef enum{
    eActionOwner_Teacher,
    eActionOwner_Student
}eActionOwner;

typedef struct{
    int type;
    QString content;
}sAction;

typedef struct{
    QString title;
    QString link;
}sLink;

typedef enum{
    eLicense_CCBY,
    eLicense_CCBYND,
    eLicense_CCBYNCSA,
    eLicense_CCBYSA,
    eLicense_CCBYNC,
    eLicense_CCBYNCND
}eLicense;

class UBTBSeparator : public QFrame
{
public:
    UBTBSeparator(QWidget* parent=0, const char* name="UBTBSeparator");
    ~UBTBSeparator();
};

class UBTBLicenseWidget : public QWidget
{
public:
    UBTBLicenseWidget(QWidget* parent=0, const char* name="UBTBLicenseWidget");
    ~UBTBLicenseWidget();
    void setLicense(eLicense lic);

private:
    QHBoxLayout mLayout;
    QLabel* mpIcon;
    QLabel* mpText;
};

class UBTeacherBarDataMgr
{
public:
    UBTeacherBarDataMgr();
    ~UBTeacherBarDataMgr();

    // Session Title
    void setSessionTitle(const QString& title){mSessionTitle = title;}
    QString sessionTitle(){return mSessionTitle;}

    // Session Target
    void setSessionTarget(const QString& target){mSessionTarget = target;}
    QString sessionTarget(){return mSessionTarget;}

    // Licence
    void setSessionLicence(eLicense licence){mSessionLicence = licence;}
    eLicense sessionLicence(){return mSessionLicence;}

    // Page Title
    void setPageTitle(const QString& title){mPageTitle = title;}
    QString pageTitle(){return mPageTitle;}

    // Actions
    QVector<sAction>* actions(){return &mActionList;}

    // Medias
    QVector<QWidget*>* medias(){return &mMediaList;}
    void addMediaUrl(const QString& url){mMediaUrls << url;}
    QStringList* mediaUrls(){return &mMediaUrls;}

    // Urls
    QVector<sLink>* urls(){return &mUrlList;}

    // Comments
    void setComments(const QString& c){mComments = c;}
    QString comments(){return mComments;}

    // Others
    void clearLists();
    void saveContent();
    void loadContent(bool docChanged = false);

private:
    QString mSessionTitle;
    QString mSessionTarget;
    eLicense mSessionLicence;
    QString mPageTitle;
    QString mComments;

    QVector<sAction> mActionList;
    QVector<sLink> mUrlList;
    QVector<QWidget*> mMediaList;
    QStringList mMediaUrls;
};

#endif // UBTEACHERBARDATAMGR_H
