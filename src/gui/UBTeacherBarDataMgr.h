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

class UBTeacherStudentAction : public QWidget
{
    Q_OBJECT

public:
    UBTeacherStudentAction(QWidget* parent=0, const char* name="UBTeacherStudentAction");
    ~UBTeacherStudentAction();
    QString text();
    QString comboValue();
    void setComboValue(int value);
    void setText(const QString& text);

private:
    QTextEdit* mpText;
    QHBoxLayout* mpLayout;
    QVBoxLayout* mpComboLayout;
    QComboBox* mpCombo;
};

class UBUrlWidget : public QWidget
{
public:
    UBUrlWidget(QWidget* parent=0, const char* name="UBUrlWidget");
    ~UBUrlWidget();

    QString url();
    void setUrl(const QString& url);

private:
    QVBoxLayout* mpLayout;
    QHBoxLayout* mpLabelLayout;
    QHBoxLayout* mpTitleLayout;
    QLabel* mpUrlLabel;
    QLineEdit* mpUrl;

    QLabel* mpTitleLabel;
    QLineEdit* mpTitle;
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

    // Page Title
    void setPageTitle(const QString& title){mPageTitle = title;}
    QString pageTitle(){return mPageTitle;}

    // Actions
    QVector<UBTeacherStudentAction*> actions(){return mActionList;}

    // Medias
    QVector<QWidget*> medias(){return mMediaList;}

    // Urls
    QVector<UBUrlWidget*> urls(){return mUrlList;}

    // Comments
    void setComments(const QString& c){mComments = c;}
    QString comments(){return mComments;}

    // Others
    void clearLists();

private:
    QString mSessionTitle;
    QString mSessionTarget;
    QString mPageTitle;
    QString mComments;

    QVector<UBTeacherStudentAction*> mActionList;
    QVector<UBUrlWidget*> mUrlList;
    QVector<QWidget*> mMediaList;
};

#endif // UBTEACHERBARDATAMGR_H
