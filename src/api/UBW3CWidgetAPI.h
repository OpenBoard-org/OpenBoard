/*
 * Copyright (C) 2015-2018 Département de l'Instruction Publique (DIP-SEM)
 *
 * Copyright (C) 2013 Open Education Foundation
 *
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour
 * l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of OpenBoard.
 *
 * OpenBoard is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * OpenBoard is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenBoard. If not, see <http://www.gnu.org/licenses/>.
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

        UBW3CWidgetAPI(UBGraphicsW3CWidgetItem *graphicsWidget, QObject *parent = 0);

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

        UBW3CWidgetPreferenceAPI* mPreferencesAPI;


};


class UBW3CWebStorage : public QObject
{
    Q_OBJECT;

    Q_PROPERTY(int length READ length SCRIPTABLE true);

    public:
        UBW3CWebStorage(QObject *parent = 0)
        : QObject(parent){/* NOOP */}
        virtual ~UBW3CWebStorage(){/* NOOP */}

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
        UBW3CWidgetPreferenceAPI(UBGraphicsW3CWidgetItem *graphicsWidget, QObject *parent = 0);

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

};



#endif /* UBW3CWIDGETAPI_H_ */
