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

#ifndef UBW3CWIDGET_H_
#define UBW3CWIDGET_H_

#include <QtGui>
#include <QtWebKit>
#include <QtXml>

/*
 * wrapper for w3c widget 1.0 as define in http://www.w3.org/TR/widgets/
 */

#include "UBAbstractWidget.h"

class UBItem;

class UBW3CWidget : public UBAbstractWidget
{
        Q_OBJECT;

    public:
        UBW3CWidget(const QUrl& pWidgetUrl, QWidget *parent = 0);

        virtual ~UBW3CWidget();

        static QString createNPAPIWrapper(const QString& url,
            const QString& pMimeType = QString(), const QSize& sizeHint = QSize(300, 150),
            const QString& pName = QString());

        static QString createNPAPIWrapperInDir(const QString& url, const QDir& pDir,
            const QString& pMimeType = QString(), const QSize& sizeHint = QSize(300, 150),
            const QString& pName = QString());

        static QString createHtmlWrapperInDir(const QString& html, const QDir& pDir,
            const QSize& sizeHint,  const QString& pName);

        static bool hasNPAPIWrapper(const QString& pMimeType);

        class PreferenceValue
        {
            public:

                PreferenceValue()
                {
                    // NOOP
                }
                

                PreferenceValue(const QString& pValue, bool pReadonly)
                {
                    value = pValue;
                    readonly = pReadonly;
                }

                QString value;
                bool readonly;
         };

        class Metadata
        {
            public:
                QString id;
                QString name;
                QString description;
                QString author;
                QString authorEmail;
                QString authorHref;
                QString version;
        };


        QMap<QString, PreferenceValue> preferences()
        {
            return mPreferences;
        }

        Metadata metadatas() const
        {
            return mMetadatas;
        }

    private:

        QMap<QString, PreferenceValue> mPreferences;

        static bool sTemplateLoaded;

        static QMap<QString, QString> sNPAPIWrapperTemplates;

        static QString sNPAPIWrappperConfigTemplate;

        static void loadNPAPIWrappersTemplates();

        static QString textForSubElementByLocale(QDomElement rootElement, QString subTagName, QLocale locale);

        // w3c metadata widget attributes
        //
        // http://www.w3.org/TR/widgets/#widget0

        Metadata mMetadatas;

    private slots:

        void javaScriptWindowObjectCleared();

};

#endif /* UBW3CWIDGET_H_ */
