#ifndef UBLIBITEMPROPERTIES_H
#define UBLIBITEMPROPERTIES_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QPixmap>
#include <QLabel>
#include <QTextEdit>
#include <QToolButton>
#include <QAction>
#include <QShowEvent>

#include "board/UBLibraryController.h"

#define THUMBNAIL_WIDTH 400

class UBLibItemButton : public QPushButton
{
public:
    UBLibItemButton(QWidget* parent=0, const char* name="UBLibItemButton");
    ~UBLibItemButton();
};

class UBLibItemProperties : public QWidget
{
    Q_OBJECT
public:
    UBLibItemProperties(QWidget* parent=0, const char* name="UBLibItemProperties");
    ~UBLibItemProperties();

    void showElement(UBLibElement* elem);

signals:
    void showFolderContent();

protected:
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);

private slots:
    void onAddToPage();
    void onAddToLib();
    void onSetAsBackground();
    void onBack();

private:
    void adaptSize();

    QVBoxLayout* mpLayout;
    QHBoxLayout* mpButtonLayout;
    UBLibItemButton* mpAddPageButton;
    UBLibItemButton* mpAddToLibButton;
    UBLibItemButton* mpSetAsBackgroundButton;
    QLabel* mpObjInfoLabel;
    QTextEdit* mpObjInfos;
    QLabel* mpThumbnail;
    QPixmap* mpOrigPixmap;
    int maxThumbHeight;
    UBLibElement* mpElement;

    // Navigation bar
    QWidget* mpNavigBar;
    QHBoxLayout* mpNavigLayout;
    QToolButton* mpNavigBack;
    QAction* mpNavigBackAction;
};


#endif // UBLIBITEMPROPERTIES_H
