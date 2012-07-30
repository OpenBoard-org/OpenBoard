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

#ifndef UBWEBKITUTILS_H_
#define UBWEBKITUTILS_H_

#include <QtWebKit>

class UBWebKitUtils
{
    public:
        UBWebKitUtils();
        virtual ~UBWebKitUtils();

        class HtmlObject
        {

            public:
                HtmlObject(const QString& pSource, int pWidth, int pHeight)
                    : source(pSource)
                    , width(pWidth)
                    , height(pHeight)
                {
                    // NOOP
                }

                QString source;
                int width;
                int height;

        };

        static QList<UBWebKitUtils::HtmlObject> objectsInFrame(QWebFrame* frame);

};

#endif /* UBWEBKITUTILS_H_ */
