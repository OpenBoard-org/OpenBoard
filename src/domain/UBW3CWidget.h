
/*
 * UBW3CWidget.h
 *
 *  Created on: 7 April 2009
 *      Author: Luc
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

        void votingSystemError(const QString&);

};

#endif /* UBW3CWIDGET_H_ */
