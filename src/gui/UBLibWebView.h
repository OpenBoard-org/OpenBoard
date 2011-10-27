#ifndef UBLIBWEBVIEW_H
#define UBLIBWEBVIEW_H

#include <QWidget>
#include <QWebView>
#include <QWebSettings>
#include <QVBoxLayout>

#include "board/UBLibraryController.h"

class UBLibWebView : public QWidget
{
    Q_OBJECT

public:
    UBLibWebView(QWidget* parent = 0, const char* name = "UBLibWebView");
    ~UBLibWebView();

    void setElement(UBLibElement* elem);

private:
    QWebView* mpView;
    QWebSettings* mpWebSettings;
    QVBoxLayout* mpLayout;
};

#endif // UBLIBWEBVIEW_H
