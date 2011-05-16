#ifndef UBLIBNAVIGATORWIDGET_H
#define UBLIBNAVIGATORWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QSlider>
#include <QDropEvent>

#include "UBLibraryWidget.h"
#include "UBLibPathViewer.h"
#include "core/UBSetting.h"

class UBLibNavigatorWidget : public QWidget
{
    Q_OBJECT
public:
    UBLibNavigatorWidget(QWidget* parent=0, const char* name="UBLibNavigatorWidget");
    ~UBLibNavigatorWidget();
    void dropMe(const QMimeData* _data);

signals:
    void propertiesRequested(UBLibElement* elem);

private slots:
    void onNavigbarUpate(UBLibElement* pElem);
    void onPathItemClicked(UBChainedLibElement *elem);
    void onPropertiesRequested(UBLibElement* elem);
    void updateThumbnailsSize(int newSize);

private:
    void removeNextChainedElements(UBChainedLibElement* fromElem);


    QVBoxLayout* mLayout;
    UBLibraryWidget* mLibWidget;
    UBLibPathViewer* mPathViewer;
    QSlider* mSlider;
    UBSetting* mSliderWidthSetting;
};

#endif // UBLIBNAVIGATORWIDGET_H
