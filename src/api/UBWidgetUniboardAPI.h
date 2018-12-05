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




#ifndef UBWIDGETAPI_H
#define UBWIDGETAPI_H

#include <QtCore>
#include <QGraphicsSceneDragDropEvent>

#include "UBW3CWidgetAPI.h"
#include "core/UBDownloadManager.h"

class UBGraphicsScene;
class UBGraphicsWidgetItem;
class UBGraphicsW3CWidgetItem;

class UBWidgetMessageAPI;
class UBDatastoreAPI;
class UBDocumentDatastoreAPI;

class UBWidgetUniboardAPI : public QObject
{
    Q_OBJECT

    /**
     * The number of pages in the current document
     */
    Q_PROPERTY(int pageCount READ pageCount SCRIPTABLE true)

    /**
     * The page number of the current page
     */
    Q_PROPERTY(int currentPageNumber READ currentPageNumber SCRIPTABLE true)

    /**
     * instance UUID, return a unique identifier for the widget, this value is guaranted to be unique
     * and constant for a widget, deprecated, use window.widget.uuid instead
     */
    Q_PROPERTY(QString uuid READ uuid SCRIPTABLE true)

    /**
     * Returns the language and eventually the country of this locale as a string of the form
     * "language-country" or only "language" if the country is unknown,
     * where language is a lowercase, two-letter ISO 639 language code, and country is an uppercase,
     * two-letter ISO 3166 country code.
     *
     * some potential values are
     *
     * en
     * en-UK
     * fr
     * fr-CH
     * fr-FR
     *
     */
    Q_PROPERTY(QString lang READ lang SCRIPTABLE true)

    Q_PROPERTY(QObject* messages READ messages SCRIPTABLE true)

    Q_PROPERTY(QObject* datastore READ datastore SCRIPTABLE true)

    public:

        UBWidgetUniboardAPI(UBGraphicsScene *pScene, UBGraphicsWidgetItem *widget = 0);
        ~UBWidgetUniboardAPI();

        QObject* messages();

        QObject* datastore();

    public slots:

        void setScene(UBGraphicsScene* pScene)
        {
            mScene = pScene;
        }

        // global

        /**
         * Set the tool (pen, marker, arrow, line)
         */
        void setTool(const QString& toolString);

        /**
         * an HTML color (red, blue, ...) or rgb (#FF0000) or a default Uniboard color (1, 2, 3, 4)
         */
        void setPenColor(const QString& penColor);

        /**
         * an HTML color (red, blue, ...) or rgba (#FF000077) or a default Uniboard color (1, 2, 3, 4)
         */
        void setMarkerColor(const QString& penColor);

        /**
         * return the url of the thumbnail of a page
         */
        QString pageThumbnail(const int pageNumber);

        //view based

        /**
         * Zoom the current view port centered on scene position x/y
         */
        void zoom(const qreal factor, const qreal x, const qreal y);

        /**
         * move the view port of x/y pixels
         */
        void move(const qreal x, const qreal y);


        //scene based

        /**
         * move the control to position x/y in scene coordinate
         */
        void moveTo(const qreal x, const qreal y);

        /**
         * draw a line from current pos to x/y in scene coordinate
         */
        void drawLineTo(const qreal x, const qreal y, const qreal pWidth);

        /**
         * erase any line from current pos to x/y in scene coordinate
         */
        void eraseLineTo(const qreal x, const qreal y, const qreal pWidth);

        /**
         * remove all drawing/object from current scene
         */
        void clear();

        /**
         * set the scene backgroung to black/white with crossing or not
         */
        void setBackground(bool pIsDark, bool pIsCrossed);

        /**
         * add any supported objects (pictures/video/widget) centered at scene position x/y.
         * width and height may be supplied, this is useful for flash (.swf) objects
         * if background is true, the object is not selectable and sits in the lowest z pos possible
         *
         */
        void addObject(QString pUrl, int width = 0, int height = 0, int x = 0, int y = 0, bool background = false);


        /**
         * The widget notify the container to resized to width/height in scene (DOM) coordintates
         */
        void resize(qreal width, qreal height);


        // widget based


        /**
         * Returns the language and eventually the country of this locale as a string of the form
         * "language_country" or only "language" if the country is unknown,
         * where language is a lowercase, two-letter ISO 639 language code, and country is an uppercase,
         * two-letter ISO 3166 country code.
         *
         * some potential values are
         *
         * en
         * en_UK
         * fr
         * fr_CH
         * fr_FR
         *
         * deprecated as it does not folow xml normas, replaced by property 'lang'
         *
         */
        QString locale();

        /**
         * Save a preference for this widget instance
         */
        void setPreference(const QString& key, QString value);


        /**
         * retreive widget instance preference
         */
        QString preference(const QString& key, const QString& pDefault = QString());


        /**
         * retreive a list of widget instance preferences
         */
        QStringList preferenceKeys();


        /**
         * Display a message on the top bottom corner of the board
         */
        void showMessage(const QString& message);


        /**
         * Center the scene coordinates within the display port
         *
         */
        void centerOn(const qreal x, const qreal y);


        /**
         * Write some text on the board
         */
        void addText(const QString& text, const qreal x, const qreal y, const int height = -1, const QString& font = ""
                , bool bold = false, bool italic = false);

        void returnStatus(const QString& method, const QString& status);
        void usedMethods(QStringList methods);
        void response(bool correct);

        /**
          * Give the file metadata to Sankore. The format must be
          * <metbadata>
          *     <key>File Size</<key>
          *     <value>1024</value>
          * </metadata>
          */
        void sendFileMetadata(QString metaData);


        // widget download
        /**
         * If the widget support a the drop of an object it will notify sankore about this.
         */
        void enableDropOnWidget (bool enable = true);

        /**
         * When an object is dropped on a widget, this one send us the informations to download it locally.
         * this method download the object on the widget directory and return the path of the downloaded object
         */
        void ProcessDropEvent(QGraphicsSceneDragDropEvent *);
        bool isDropableData(const QMimeData *pMimeData) const;

private slots:
        void onDownloadFinished(bool pSuccess, sDownloadFileDesc desc, QByteArray pData);

private:
        inline void registerIDWidget(int id){webDownloadIds.append(id);}
        inline bool takeIDWidget(int id);


    private:

        QString uuid();

        QString lang();

        int pageCount();

        int currentPageNumber();
        QString getObjDir();
        QString createMimeText(bool downloaded, const QString &mimeType, const QString &fileName);
        bool supportedTypeHeader(const QString &) const;
        QString boolToStr(bool value) const {return value ? "true" : "false";}

        UBGraphicsScene* mScene;

        UBGraphicsWidgetItem* mGraphicsWidget;

        bool mIsVisible;

        UBWidgetMessageAPI* mMessagesAPI;

        UBDatastoreAPI* mDatastoreAPI;
        QList<int> webDownloadIds;
};


class UBDatastoreAPI : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QObject* document READ document SCRIPTABLE true)

    public:
        UBDatastoreAPI(UBGraphicsW3CWidgetItem *widget);
        virtual ~UBDatastoreAPI(){;}

        QObject* document();

    private:

        UBDocumentDatastoreAPI* mDocumentDatastore;

};


class UBDocumentDatastoreAPI : public UBW3CWebStorage
{
    Q_OBJECT

    public:
        UBDocumentDatastoreAPI(UBGraphicsW3CWidgetItem *graphicsWidget);

        virtual ~UBDocumentDatastoreAPI();

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

#endif // UBWIDGETAPI_H
