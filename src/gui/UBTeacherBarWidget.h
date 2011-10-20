#ifndef UBTEACHERBARWIDGET_H
#define UBTEACHERBARWIDGET_H

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

class UBTeacherBarWidget : public UBDockPaletteWidget
{
public:
    UBTeacherBarWidget(QWidget* parent=0, const char* name="UBTeacherBarWidget");
    ~UBTeacherBarWidget();

private slots:
    void saveContent();
    void loadContent();
    void onValueChanged();

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
    QWidget* mpContainer;
    QVBoxLayout* mpContainerLayout;
};

#endif // UBTEACHERBARWIDGET_H
