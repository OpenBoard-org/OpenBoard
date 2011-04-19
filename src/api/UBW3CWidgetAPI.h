/*
 * UBW3CWidgetAPI.h
 *
 *  Created on: 28 august 2009
 *      Author: Luc
 */

#ifndef UBW3CWIDGETAPI_H_
#define UBW3CWIDGETAPI_H_

#include <QtCore>

class UBGraphicsW3CWidgetItem;
class UBW3CWidgetPreferenceAPI;
class UBW3CWidget;

class UBW3CWidgetAPI : public QObject
{
    Q_OBJECT;

    //w3c widget norma (http://www.w3.org/TR/widgets-apis/#widget)

    Q_PROPERTY(QString id READ id SCRIPTABLE true);
    Q_PROPERTY(QString name READ name SCRIPTABLE true);
    Q_PROPERTY(QString description READ description SCRIPTABLE true);
    Q_PROPERTY(QString author READ author SCRIPTABLE true);

    Q_PROPERTY(QString authorEmail READ authorEmail SCRIPTABLE true);
    Q_PROPERTY(QString authorHref READ authorHref SCRIPTABLE true);
    Q_PROPERTY(QString version READ version SCRIPTABLE true);

    Q_PROPERTY(int width READ width SCRIPTABLE true);
    Q_PROPERTY(int height READ height SCRIPTABLE true);

    Q_PROPERTY(QObject* preferences READ preferences SCRIPTABLE true);

    // Mnemis extensions
    Q_PROPERTY(QString uuid READ uuid SCRIPTABLE true);

    public:

        UBW3CWidgetAPI(UBGraphicsW3CWidgetItem *graphicsWidget);
        UBW3CWidgetAPI(UBW3CWidget *widget);

        virtual ~UBW3CWidgetAPI();

        QString uuid();

        QString id();
        QString name();
        QString description();
        QString author();
        QString authorEmail();
        QString authorHref();
        QString version();

        QObject* preferences();

        int width();
        int height();

        void openURL(const QString& url);

    private:

        UBGraphicsW3CWidgetItem* mGraphicsW3CWidget;
        UBW3CWidget *mW3CWidget;

        UBW3CWidgetPreferenceAPI* mPreferencesAPI;


};


class UBW3CWebStorage : public QObject
{
    Q_OBJECT;

    Q_PROPERTY(int length READ length SCRIPTABLE true);

    public:
        UBW3CWebStorage(QObject *parent = 0)
        : QObject(parent){}
        virtual ~UBW3CWebStorage(){}

    public slots:

        virtual QString key(int index) = 0;
        virtual QString getItem(const QString& key) = 0;
        virtual void setItem(const QString& key, const QString& value) = 0;
        virtual void removeItem(const QString& key) = 0;
        virtual void clear() = 0;

    protected:
        virtual int length() = 0;

};


class UBW3CWidgetPreferenceAPI : public UBW3CWebStorage
{
    Q_OBJECT;

    public:
        UBW3CWidgetPreferenceAPI(UBGraphicsW3CWidgetItem *graphicsWidget);
        UBW3CWidgetPreferenceAPI(UBW3CWidget *widget);

        virtual ~UBW3CWidgetPreferenceAPI();

    public slots:

        virtual QString key(int index);
        virtual QString getItem(const QString& key);
        virtual void setItem(const QString& key, const QString& value);
        virtual void removeItem(const QString& key);
        virtual void clear();

    protected:
        virtual int length();

    private:

        UBGraphicsW3CWidgetItem* mGraphicsW3CWidget;
        UBW3CWidget* mW3CWidget;

};



#endif /* UBW3CWIDGETAPI_H_ */
