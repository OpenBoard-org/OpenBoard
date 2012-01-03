#ifndef UBTEACHERBARWIDGET_H
#define UBTEACHERBARWIDGET_H

class UBMediaPlayer;
class UBVideoPlayer;

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QLineEdit>
#include <QComboBox>

#include "UBDockPaletteWidget.h"

#define LABEL_MINWIDHT      80

class UBTeacherStudentAction : public QWidget
{
    Q_OBJECT

public:
    UBTeacherStudentAction(int actionNumber, QWidget* parent=0, const char* name="UBTeacherStudentAction");
    ~UBTeacherStudentAction();
    QString teacherText();
    QString studentText();
    void setTeacherText(QString text);
    void setStudentText(QString text);
    QTextEdit* teacher();
    QTextEdit* student();

private:
    int mActionNumber;
    QLabel* mpActionLabel;
    QLabel* mpTeacherLabel;
    QLabel* mpStudentLabel;
    QTextEdit* mpTeacher;
    QTextEdit* mpStudent;
    QVBoxLayout* mpLayout;
    QHBoxLayout* mpTeacherLayout;
    QHBoxLayout* mpStudentLayout;
};


class UBTeacherBarDropMediaZone : public QWidget
{
    Q_OBJECT

public:
    UBTeacherBarDropMediaZone(QWidget* parent=0, const char* name="UBTeacherBarDropMediaZone");
    ~UBTeacherBarDropMediaZone();

private:
    QLabel* mpTitleLabel;
    QLabel* mpImageTab;
    UBMediaPlayer* mpVideoTab;
    UBMediaPlayer* mpAudioTab;
    QTabWidget* mpTabWidget;
    QVBoxLayout* mpLayout;

protected:
    void dragEnterEvent(QDragEnterEvent* pEvent);
    void dropEvent(QDropEvent *pEvent);
    void dragMoveEvent(QDragMoveEvent* pEvent);
    void dragLeaveEvent(QDragLeaveEvent* pEvent);
};

class UBTeacherBarWidget : public UBDockPaletteWidget
{
    Q_OBJECT
public:
    UBTeacherBarWidget(QWidget* parent=0, const char* name="UBTeacherBarWidget");
    ~UBTeacherBarWidget();

private slots:
    void saveContent();
    void loadContent();
    void onValueChanged();
	void onTitleTextChanged(const QString& text);
	void onEquipmentTextChanged(const QString& text);

private:
    void populateCombos();

    QVBoxLayout* mpLayout;
    QHBoxLayout* mpTitleLayout;
    QHBoxLayout* mpPhasisLayout;
    QHBoxLayout* mpDurationLayout;
    QHBoxLayout* mpEquipmentLayout;
    QHBoxLayout* mpActivityLayout;
    QLabel* mpTitleLabel;
    QLabel* mpPhasisLabel;
    QLabel* mpDurationLabel;
    QLabel* mpEquipmentLabel;
    QLabel* mpActivityLabel;
    QLineEdit* mpTitle;
    QLineEdit* mpEquipment;
    QComboBox* mpPhasis;
    QComboBox* mpDuration;
    QComboBox* mpActivity;
    UBTeacherStudentAction* mpAction1;
    UBTeacherStudentAction* mpAction2;
    UBTeacherStudentAction* mpAction3;
    UBTeacherBarDropMediaZone* mpDropMediaZone;
    QWidget* mpContainer;
    QVBoxLayout* mpContainerLayout;
};

#endif // UBTEACHERBARWIDGET_H
