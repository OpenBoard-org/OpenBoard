/*
 * Copyright (C) 2012 Webdoc SA
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation, version 2,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with Open-Sankoré; if not, see
 * <http://www.gnu.org/licenses/>.
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
