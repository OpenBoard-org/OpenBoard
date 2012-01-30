#ifndef UBTEACHERBARWIDGET_H
#define UBTEACHERBARWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QLineEdit>
#include <QCheckBox>
#include <QTabWidget>
#include <QButtonGroup>
#include <QPushButton>
#include <QComboBox>
#include <QStackedWidget>

#include "UBDockPaletteWidget.h"
#include "customWidgets/UBWidgetList.h"
#include "interfaces/IDropable.h"
#include "UBTeacherBarDataMgr.h"
#include "UBTBDocumentPreviewWidget.h"
#include "UBTBPageEditWidget.h"
#include "UBTeacherBarPreviewWidget.h"
#include "UBTBDocumentEditWidget.h"

#define LABEL_MINWIDHT      80

class UBTeacherBarWidget : public UBDockPaletteWidget
{
    Q_OBJECT
public:
    UBTeacherBarWidget(QWidget* parent=0, const char* name="UBTeacherBarWidget");
    ~UBTeacherBarWidget();

    bool visibleInMode(eUBDockPaletteWidgetMode mode)
    {
        return (mode == eUBDockPaletteWidget_BOARD) || (mode == eUBDockPaletteWidget_DESKTOP);
    }
    
    void loadContent(bool docChanged = false);

public slots:
    void saveContent();

private slots:
    void loadContentInfos();
    void onValueChanged();
    void onShowEditMode();
    void onTBStateChanged(eTeacherBarState state);
    void onActiveDocumentChanged();

private:
    bool isEmpty();

    QVBoxLayout mLayout;
    QStackedWidget* mpStackWidget;
    eTeacherBarState mState;

    UBTBPageEditWidget* mpPageEditWidget;
    UBTeacherBarPreviewWidget* mpPreview;
    UBTBDocumentPreviewWidget* mpDocPreviewWidget;
    UBTBDocumentEditWidget* mpDocEditWidget;

    UBTeacherBarDataMgr mData;
};

#endif // UBTEACHERBARWIDGET_H
