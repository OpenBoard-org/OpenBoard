/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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

#ifndef UBWINDOWCAPTURE_H_
#define UBWINDOWCAPTURE_H_


#include <QtGui>

class UBDesktopAnnotationController;

class UBWindowCapture : public QObject
{
    Q_OBJECT;

    public:
        UBWindowCapture(UBDesktopAnnotationController *parent = 0);
        virtual ~UBWindowCapture();
        int execute();
        const QPixmap getCapturedWindow();

    private:
        QPixmap mWindowPixmap;
        UBDesktopAnnotationController* mParent;
};
#endif /* UBWINDOWCAPTURE_H_ */
