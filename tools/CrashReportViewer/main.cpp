/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
