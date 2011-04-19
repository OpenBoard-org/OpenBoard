/*
 * main.cpp
 *
 *  Created on: Feb 10, 2009
 *      Author: julienbachmann
 */

#include <QtGui>
#include "CrashWindow.h"
#include "client/mac/handler/exception_handler.h"

class CRVApplication : public QApplication
{
public:

    CRVApplication(int &argc, char **argv) : QApplication(argc, argv) {}

    bool event(QEvent *e)
    {
        if (e->type() == QEvent::FileOpen)
        {
            window.setDumpFilePath(static_cast<QFileOpenEvent*>(e)->file());
            return true;
        }
        return QApplication::event(e);
    }

    CrashWindow window;
};

int main(int argc, char *argv[])
{
    CRVApplication app(argc, argv);

    app.window.show();

    app.exec();
}
