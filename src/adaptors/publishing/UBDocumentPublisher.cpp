#include "UBDocumentPublisher.h"

#include "frameworks/UBPlatformUtils.h"
#include "frameworks/UBFileSystemUtils.h"
#include "frameworks/UBStringUtils.h"

#include "network/UBNetworkAccessManager.h"
#include "network/UBServerXMLHttpRequest.h"

#include "core/UBDocumentManager.h"
#include "core/UBApplication.h"
#include "core/UBPersistenceManager.h"

#include "gui/UBMainWindow.h"

#include "document/UBDocumentProxy.h"

#include "domain/UBGraphicsWidgetItem.h"

#include "quazip.h"
#include "quazipfile.h"

#include "adaptors/UBExportFullPDF.h"
#include "adaptors/UBExportDocument.h"
#include "adaptors/UBSvgSubsetAdaptor.h"

#include "UBSvgSubsetRasterizer.h"


UBDocumentPublisher::UBDocumentPublisher(UBDocumentProxy* pDocument, QObject *parent)
    : UBAbstractPublisher(parent)
    , mSourceDocument(pDocument)
    , mPublishingDocument(0)
{
    connect(this, SIGNAL(authenticated(const QUuid&, const QString&))
            , this, SLOT(postDocument(const QUuid&, const QString&)));
}


UBDocumentPublisher::~UBDocumentPublisher()
{
    delete mPublishingDocument;
}


void UBDocumentPublisher::publish()
{
    UBAbstractPublisher::authenticate();
}


void UBDocumentPublisher::postDocument(const QUuid& tokenUuid, const QString& encryptedBase64Token)
{
    mAuthenticationUuid = tokenUuid;
    mAuthenticationBase64Token = encryptedBase64Token;

    UBDocumentPublishingDialog dialog(UBApplication::mainWindow);

    dialog.videoWarning->setVisible(UBPersistenceManager::persistenceManager()->mayHaveVideo(mSourceDocument));

    dialog.title->setText(mSourceDocument->name());

    QString defaultEMail = UBSettings::settings()->uniboardWebEMail->get().toString();
    dialog.email->setText(defaultEMail);

    QString defaultAuthor = UBSettings::settings()->uniboardWebAuthor->get().toString();
    dialog.author->setText(defaultAuthor);

    if (dialog.exec() == QDialog::Accepted)
    {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        UBApplication::showMessage(tr("Preparing document for upload..."), true);

        mTitle = dialog.title->text();
        mDescription = dialog.description->toPlainText();
        mEMail = dialog.email->text();
        mAuthor = dialog.author->text();

        bool attachPDF = dialog.attachPDF->isChecked();
        bool attachUBZ = dialog.attachUBZ->isChecked();

        mPublishingServiceUrl = UBSettings::settings()->documentsPublishingUrl;

        UBSettings::settings()->uniboardWebEMail->set(mEMail);
        UBSettings::settings()->uniboardWebAuthor->set(mAuthor);

        QDir d;
        d.mkpath(UBFileSystemUtils::defaultTempDirPath());

        QString tmpDir = UBFileSystemUtils::createTempDir();

        if (UBFileSystemUtils::copyDir(mSourceDocument->persistencePath(), tmpDir))
        {
            QUuid publishingUuid = QUuid::createUuid();

            mPublishingDocument = new UBDocumentProxy(tmpDir);
            mPublishingDocument->setPageCount(mSourceDocument->pageCount());

            rasterizeScenes();

            //rasterizePDF(); // not needed as we do not publish svg file anymore

            //rasterizeSVGImages(); // not needed as we do not publish svg file anymore

            upgradeDocumentForPublishing();

            if (attachPDF)
            {
                UBExportFullPDF pdfExporter;
                pdfExporter.setVerbode(false);
                pdfExporter.persistsDocument(mSourceDocument,
                        mPublishingDocument->persistencePath() + "/" + UBStringUtils::toCanonicalUuid(publishingUuid) + ".pdf");
            }

            if (attachUBZ)
            {
                UBExportDocument ubzExporter;
                ubzExporter.setVerbode(false);
                ubzExporter.persistsDocument(mSourceDocument,
                        mPublishingDocument->persistencePath() + "/" + UBStringUtils::toCanonicalUuid(publishingUuid) + ".ubz");
            }

            // remove all useless files

            for (int pageIndex = 0; pageIndex < mPublishingDocument->pageCount(); pageIndex++)
            {
                QString filename = mPublishingDocument->persistencePath() +
                        UBFileSystemUtils::digitFileFormat("/page%1.svg", pageIndex + 1);

                QFile::remove(filename);
            }

            UBFileSystemUtils::deleteDir(mPublishingDocument->persistencePath() + "/" + UBPersistenceManager::imageDirectory);
            UBFileSystemUtils::deleteDir(mPublishingDocument->persistencePath() + "/" + UBPersistenceManager::objectDirectory);
            UBFileSystemUtils::deleteDir(mPublishingDocument->persistencePath() + "/" + UBPersistenceManager::videoDirectory);
            UBFileSystemUtils::deleteDir(mPublishingDocument->persistencePath() + "/" + UBPersistenceManager::audioDirectory);

            QString tempZipFile = UBFileSystemUtils::defaultTempDirPath()
                    + "/" + UBStringUtils::toCanonicalUuid(QUuid::createUuid()) + ".zip";

            //qDebug() << "compressing" << mPublishingDocument->persistencePath() << "in" << tempZipFile;

            QuaZip zip(tempZipFile);
            zip.setFileNameCodec("UTF-8");
            if (!zip.open(QuaZip::mdCreate))
            {
                qWarning() << "Export failed. Cause: zip.open(): " << zip.getZipError() << "," << tempZipFile;
                QApplication::restoreOverrideCursor();
                return;
            }

            QuaZipFile outFile(&zip);

            if (!UBFileSystemUtils::compressDirInZip(mPublishingDocument->persistencePath(), "", &outFile, true))
            {
                qWarning("Export failed. compressDirInZip failed ...");
                zip.close();
                //zip.remove();
                UBApplication::showMessage(tr("Export failed."));
                QApplication::restoreOverrideCursor();
                return;
            }

            if (zip.getZipError() != 0)
            {
                qWarning("Export failed. Cause: zip.close(): %d", zip.getZipError());
                zip.close();
                //zip.remove();
                UBApplication::showMessage(tr("Export failed."));
                QApplication::restoreOverrideCursor();
                return;
            }

            zip.close();

            mPublishingUrl = QUrl(mPublishingServiceUrl + "/documents/publish/"
                + UBStringUtils::toCanonicalUuid(mSourceDocument->uuid()));

            sendZipToUniboardWeb(tempZipFile, publishingUuid);
        }
        else
        {
            UBApplication::showMessage(tr("Export failed ..."));
            QApplication::restoreOverrideCursor();
        }
    }
    else
    {
        UBApplication::showMessage(tr("Export canceled ..."));
        QApplication::restoreOverrideCursor();
    }
}


/*
 * // not needed as we do not publish svg file anymore
 *

void UBDocumentPublisher::rasterizePDF()
{
    if (UBPersistenceManager::persistenceManager()->mayHavePDF(mPublishingDocument))
    {
        UBSvgSubsetAdaptor::convertPDFObjectsToImages(mPublishingDocument);

        QDir objectDir(mPublishingDocument->persistencePath() + "/" + UBPersistenceManager::objectDirectory);

        QStringList filters;
        filters << "*.pdf";

        foreach(QFileInfo fi, objectDir.entryInfoList(filters))
        {
            QFile::remove(fi.absoluteFilePath());
        }
    }
}
*/

/*
 * // not needed as we do not publish svg file anymore


void UBDocumentPublisher::rasterizeSVGImages()
{
    if (UBPersistenceManager::persistenceManager()->mayHaveSVGImages(mPublishingDocument))
    {
        UBSvgSubsetAdaptor::convertSvgImagesToImages(mPublishingDocument);

        QDir objectDir(mPublishingDocument->persistencePath() + "/" + UBPersistenceManager::imageDirectory);

        QStringList filters;
        filters << "*.svg";

        foreach(QFileInfo fi, objectDir.entryInfoList(filters))
        {
            QFile::remove(fi.absoluteFilePath());
        }
    }
}
*/

void UBDocumentPublisher::rasterizeScenes()
{

    for(int pageIndex = 0; pageIndex < mPublishingDocument->pageCount(); pageIndex++)
    {
        UBApplication::showMessage(tr("Converting page %1/%2 ...").arg(pageIndex + 1).arg(mPublishingDocument->pageCount()), true);

        UBSvgSubsetRasterizer rasterizer(mPublishingDocument, pageIndex);

        QString filename = mPublishingDocument->persistencePath() +
            UBFileSystemUtils::digitFileFormat("/page%1.jpg", pageIndex + 1);

        rasterizer.rasterizeToFile(filename);

    }
}


void UBDocumentPublisher::updateGoogleMapApiKey()
{
    QDir widgestDir(mPublishingDocument->persistencePath() + "/" + UBPersistenceManager::widgetDirectory);

    QString uniboardWebGoogleMapApiKey = UBSettings::settings()->uniboardWebGoogleMapApiKey->get().toString();

    foreach(QFileInfo dirInfo, widgestDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot))
    {
        QString config = UBFileSystemUtils::readTextFile(dirInfo.absoluteFilePath() + "/config.xml").toLower();

        if (config.contains("google") && config.contains("map"))
        {
            QDir widgetDir(dirInfo.absoluteFilePath());

            foreach(QFileInfo fileInfo, widgetDir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot))
            {
                QFile file(fileInfo.absoluteFilePath());

                if(file.open(QIODevice::ReadWrite))
                {
                    QTextStream stream(&file);
                    QString content = stream.readAll();

                    if (content.contains("ABQIAAAA6vtVqAUu8kZ_eTz7c8kwSBT9UCAhw_xm0LNFHsWmQxTJAdp5lxSY_5r-lZriY_7sACaMnl80JcX6Og"))
                    {
                        content.replace("ABQIAAAA6vtVqAUu8kZ_eTz7c8kwSBT9UCAhw_xm0LNFHsWmQxTJAdp5lxSY_5r-lZriY_7sACaMnl80JcX6Og",
                                        uniboardWebGoogleMapApiKey);

                        file.resize(0);
                        file.write(content.toUtf8());
                    }
                    file.close();
                }
            }
        }
    }
}


void UBDocumentPublisher::upgradeDocumentForPublishing()
{
    for(int pageIndex = 0; pageIndex < mPublishingDocument->pageCount(); pageIndex++)
    {
        UBGraphicsScene *scene = UBSvgSubsetAdaptor::loadScene(mPublishingDocument, pageIndex);

        bool sceneHasWidget = false;

        QList<UBGraphicsW3CWidgetItem*> widgets;

        foreach(QGraphicsItem* item, scene->items())
        {
            UBGraphicsW3CWidgetItem *widgetItem = dynamic_cast<UBGraphicsW3CWidgetItem*>(item);

            if (widgetItem)
            {
                generateWidgetPropertyScript(widgetItem, pageIndex + 1);
                sceneHasWidget = true;
                widgets << widgetItem;
            }
        }

        //if (sceneHasWidget)
        //{
        //    updateSVGForWidget(pageIndex); // not needed as we do not publish svg file anymore
        //}

        QString filename = mPublishingDocument->persistencePath() +
            UBFileSystemUtils::digitFileFormat("/page%1.json", pageIndex + 1);

        QFile jsonFile(filename);
        if (jsonFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
        {
            jsonFile.write("{\n");
            jsonFile.write(QString("  \"scene\": {\n").toUtf8());
            jsonFile.write(QString("    \"x\": %1,\n").arg(scene->normalizedSceneRect().x()).toUtf8());
            jsonFile.write(QString("    \"y\": %1,\n").arg(scene->normalizedSceneRect().y()).toUtf8());
            jsonFile.write(QString("    \"width\": %1,\n").arg(scene->normalizedSceneRect().width()).toUtf8());
            jsonFile.write(QString("    \"height\": %1\n").arg(scene->normalizedSceneRect().height()).toUtf8());
            jsonFile.write(QString("  },\n").toUtf8());

            jsonFile.write(QString("  \"widgets\": [\n").toUtf8());

            bool first = true;

            foreach(UBGraphicsW3CWidgetItem* widget, widgets)
            {
                if (!first)
                    jsonFile.write(QString("    ,\n").toUtf8());

                jsonFile.write(QString("    {\n").toUtf8());
                jsonFile.write(QString("      \"uuid\": \"%1\",\n").arg(UBStringUtils::toCanonicalUuid(widget->uuid())).toUtf8());
                jsonFile.write(QString("      \"id\": \"%1\",\n").arg(widget->metadatas().id).toUtf8());

                jsonFile.write(QString("      \"name\": \"%1\",\n").arg(widget->w3cWidget()->metadatas().name).toUtf8());
                jsonFile.write(QString("      \"description\": \"%1\",\n").arg(widget->w3cWidget()->metadatas().description).toUtf8());
                jsonFile.write(QString("      \"author\": \"%1\",\n").arg(widget->w3cWidget()->metadatas().author).toUtf8());
                jsonFile.write(QString("      \"authorEmail\": \"%1\",\n").arg(widget->w3cWidget()->metadatas().authorEmail).toUtf8());
                jsonFile.write(QString("      \"authorHref\": \"%1\",\n").arg(widget->w3cWidget()->metadatas().authorHref).toUtf8());
                jsonFile.write(QString("      \"version\": \"%1\",\n").arg(widget->w3cWidget()->metadatas().authorHref).toUtf8());

                jsonFile.write(QString("      \"x\": %1,\n").arg(widget->sceneBoundingRect().x()).toUtf8());
                jsonFile.write(QString("      \"y\": %1,\n").arg(widget->sceneBoundingRect().y()).toUtf8());
                jsonFile.write(QString("      \"width\": %1,\n").arg(widget->sceneBoundingRect().width()).toUtf8());
                jsonFile.write(QString("      \"height\": %1,\n").arg(widget->sceneBoundingRect().height()).toUtf8());

                jsonFile.write(QString("      \"nominalWidth\": %1,\n").arg(widget->boundingRect().width()).toUtf8());
                jsonFile.write(QString("      \"nominalHeight\": %1,\n").arg(widget->boundingRect().height()).toUtf8());

                QString url = UBPersistenceManager::widgetDirectory + "/" + widget->uuid().toString() + ".wgt";
                jsonFile.write(QString("      \"src\": \"%1\",\n").arg(url).toUtf8());
                QString startFile = widget->w3cWidget()->mainHtmlFileName();
                jsonFile.write(QString("      \"startFile\": \"%1\",\n").arg(startFile).toUtf8());

                QMap<QString, QString> preferences = widget->preferences();

                jsonFile.write(QString("      \"preferences\": {\n").toUtf8());

                foreach(QString key, preferences.keys())
                {
                    QString sep = ",";
                    if (key == preferences.keys().last())
                        sep = "";

                    jsonFile.write(QString("          \"%1\": \"%2\"%3\n")
                            .arg(key)
                            .arg(preferences.value(key))
                            .arg(sep)
                                .toUtf8());
                }
                jsonFile.write(QString("      },\n").toUtf8());

                jsonFile.write(QString("      \"datastore\": {\n").toUtf8());

                QMap<QString, QString> datastoreEntries = widget->datastoreEntries();

                foreach(QString entry, datastoreEntries.keys())
                {
                    QString sep = ",";
                    if (entry == datastoreEntries.keys().last())
                        sep = "";

                    jsonFile.write(QString("          \"%1\": \"%2\"%3\n")
                            .arg(entry)
                            .arg(datastoreEntries.value(entry))
                            .arg(sep)
                                .toUtf8());
                }
                jsonFile.write(QString("      }\n").toUtf8());

                jsonFile.write(QString("    }\n").toUtf8());

                first = false;
            }

            jsonFile.write("  ]\n");
            jsonFile.write("}\n");
        }
        else
        {
            qWarning() << "Cannot open file" << filename << "for saving page state";
        }

        delete scene;
    }

    updateGoogleMapApiKey();
}


/** // not needed as we do not publish svg file anymore
void UBDocumentPublisher::updateSVGForWidget(int pageIndex)
{
    QString fileName = mPublishingDocument->persistencePath() +
        UBFileSystemUtils::digitFileFormat("/page%1.svg", pageIndex + 1);

    QFile svgFile(fileName);

    if (svgFile.exists())
    {
        if (!svgFile.open(QIODevice::ReadWrite))
        {
            qWarning() << "Cannot open file " << fileName << " for widget upgrade ...";
            return;
        }

        QTextStream stream(&svgFile);
        QStringList lines;

        QString line;
        do
        {
            line = stream.readLine();
            if (!line.isNull())
            {
                if (line.contains("<svg") && line.contains(">")) // TODO UB 4.6, this is naive ... the SVG tag may be on several lines
                {
                    lines << "<?xml-stylesheet type=\"text/css\" href=\"" + UBSettings::settings()->documentPlayerCssUrl + "\" ?>";
                    lines << line;
                    lines << "";
                    lines << "    <script type=\"text/ecmascript\" xlink:href=\"" + UBSettings::settings()->documentPlayerScriptUrl + "\"/>";
                    lines << "    <script type=\"text/ecmascript\" xlink:href=\"" + UBSettings::settings()->documentPlayerPageScriptUrl + "\"/>";
                    lines << "";
                }
                else
                {
                    lines << line;
                }
            }
        }
        while (!line.isNull());

        svgFile.resize(0);
        svgFile.write(lines.join("\n").toUtf8()); // TODO UB 4.x detect real html encoding

        svgFile.close();
    }
}
*/


void UBDocumentPublisher::generateWidgetPropertyScript(UBGraphicsW3CWidgetItem *widgetItem, int pageNumber)
{

    QMap<QString, QString> preferences = widgetItem->preferences();
    QMap<QString, QString> datastoreEntries = widgetItem->datastoreEntries();

    QString startFileName = widgetItem->w3cWidget()->mainHtmlFileName();

    if (!startFileName.startsWith("http://"))
    {
        QString startFilePath = mPublishingDocument->persistencePath() + "/"
                                + UBPersistenceManager::widgetDirectory + "/"
                                + widgetItem->uuid().toString() + ".wgt/" + startFileName;

        QFile startFile(startFilePath);

        if(startFile.exists())
        {
            if(startFile.open(QIODevice::ReadWrite))
            {
                QTextStream stream(&startFile);
                QStringList lines;

                bool addedJs = false;

                QString line;
                do
                {
                    line = stream.readLine();
                    if (!line.isNull())
                    {
                        lines << line;

                        if (!addedJs && line.contains("<head") && line.contains(">") )  // TODO UB 4.6, this is naive ... the HEAD tag may be on several lines
                        {
                            lines << "";
                            lines << "  <script type=\"text/javascript\">";

                            lines << "    var widget = {};";
                            lines << "    widget.id = '" + widgetItem->w3cWidget()->metadatas().id + "';";
                            lines << "    widget.name = '" + widgetItem->w3cWidget()->metadatas().name + "';";
                            lines << "    widget.description = '" + widgetItem->w3cWidget()->metadatas().description + "';";
                            lines << "    widget.author = '" + widgetItem->w3cWidget()->metadatas().author + "';";
                            lines << "    widget.authorEmail = '" + widgetItem->w3cWidget()->metadatas().authorEmail + "';";
                            lines << "    widget.authorHref = '" + widgetItem->w3cWidget()->metadatas().authorHref + "';";
                            lines << "    widget.version = '" + widgetItem->w3cWidget()->metadatas().version + "';";

                            lines << "    widget.uuid = '" + UBStringUtils::toCanonicalUuid(widgetItem->uuid()) + "';";

                            lines << "    widget.width = " + QString("%1").arg(widgetItem->w3cWidget()->width()) + ";";
                            lines << "    widget.height = " + QString("%1").arg(widgetItem->w3cWidget()->height()) + ";";
                            lines << "    widget.openUrl = function(url) { window.open(url); }";
                            lines << "    widget.preferences = new Array()";

                            foreach(QString pref, preferences.keys())
                            {
                                lines << "      widget.preferences['" + pref + "'] = '" + preferences.value(pref) + "';";
                            }

                            lines << "    widget.preferences.key = function(index) {";
                            lines << "      var currentIndex = 0;";
                            lines << "      for(key in widget.preferences){";
                            lines << "        if (currentIndex == index){ return key;}";
                            lines << "        currentIndex++;";
                            lines << "      }";
                            lines << "      return '';";
                            lines << "    }";

                            lines << "    widget.preferences.getItem = function(key) {";
                            lines << "      return widget.preferences[key];";
                            lines << "    }";

                            lines << "    widget.preferences.setItem = function(key, value) {}";
                            lines << "    widget.preferences.removeItem = function(key) {}";
                            lines << "    widget.preferences.clear = function() {}";

                            lines << "    var uniboard = {};";
                            lines << "    uniboard.pageCount = " + QString("%1").arg(mPublishingDocument->pageCount()) + ";";
                            lines << "    uniboard.currentPageNumber = " + QString("%1").arg(pageNumber) + ";";
                            lines << "    uniboard.uuid = '" + UBStringUtils::toCanonicalUuid(widgetItem->uuid()) + "'";
                            lines << "    uniboard.lang = navigator.language;";
                            lines << "    uniboard.locale = function() {return navigator.language}";
                            lines << "    uniboard.messages = {}";
                            lines << "    uniboard.messages.subscribeToTopic = function(topicName){}";
                            lines << "    uniboard.messages.unsubscribeFromTopic = function(topicName){}";
                            lines << "    uniboard.messages.sendMessage = function(topicName, message){}";

                            lines << "    uniboard.datastore = {};";
                            lines << "    uniboard.datastore.document = new Array();";
                            foreach(QString entry, datastoreEntries.keys())
                            {
                                lines << "      uniboard.datastore.document['" + entry + "'] = '" + datastoreEntries.value(entry) + "';";
                            }

                            lines << "    uniboard.datastore.document.key = function(index) {";
                            lines << "      var currentIndex = 0;";
                            lines << "      for(key in uniboard.datastore.document){";
                            lines << "        if (currentIndex == index){ return key;}";
                            lines << "        currentIndex++;";
                            lines << "      }";
                            lines << "      return '';";
                            lines << "    }";

                            lines << "    uniboard.datastore.document.getItem = function(key) {";
                            lines << "      return uniboard.datastore.document[key];";
                            lines << "    }";

                            lines << "    uniboard.datastore.document.setItem = function(key, value) {}";
                            lines << "    uniboard.datastore.document.removeItem = function(key) {}";
                            lines << "    uniboard.datastore.document.clear = function() {}";

                            lines << "    uniboard.setTool = function(tool){}";
                            lines << "    uniboard.setPenColor = function(color){}";
                            lines << "    uniboard.setMarkerColor = function(color){}";

                            lines << "    uniboard.pageThumbnail = function(pageNumber){";
                            lines << "      var nb;";
                            lines << "      if (pageNumber < 10) return 'page00' + pageNumber + '.thumbnail.jpg';";
                            lines << "      if (pageNumber < 100) return 'page0' + pageNumber + '.thumbnail.jpg';";
                            lines << "      return 'page' + pageNumber + '.thumbnail.jpg;'";
                            lines << "    }";

                            lines << "    uniboard.zoom = function(factor, x, y){}";
                            lines << "    uniboard.move = function(x, y){}";
                            lines << "    uniboard.move = function(x, y){}";
                            lines << "    uniboard.moveTo = function(x, y){}";
                            lines << "    uniboard.drawLineTo = function(x, y, width){}";
                            lines << "    uniboard.eraseLineTo = function(x, y, width){}";
                            lines << "    uniboard.clear = function(){}";
                            lines << "    uniboard.setBackground = function(dark, crossed){}";
                            lines << "    uniboard.addObject = function(url, width, height, x, y, background){}";
                            lines << "    uniboard.resize = function(width, height){window.resizeTo(width, height);}";

                            lines << "    uniboard.showMessage = function(message){alert(message);}";
                            lines << "    uniboard.centerOn = function(x, y){}";
                            lines << "    uniboard.addText = function(text, x, y){}";

                            lines << "    uniboard.setPreference = function(key, value){}";
                            lines << "    uniboard.preference = function(key, defValue){";
                            lines << "      var pref = widget.preferences[key];";
                            lines << "      if (pref == undefined) ";
                            lines << "        return defValue;";
                            lines << "      else ";
                            lines << "        return pref;";
                            lines << "    }";
                            lines << "    uniboard.preferenceKeys = function(){";
                            lines << "        var keys = new Array();";
                            lines << "        for(key in widget.preferences){";
                            lines << "            keys.push(key);";
                            lines << "        }";
                            lines << "        return keys;";
                            lines << "    }";

                            lines << "    uniboard.datastore.document.key = function(index) {";
                            lines << "        var currentIndex = 0;";
                            lines << "        for(key in uniboard.datastore.document){";
                            lines << "            if (currentIndex == index){ return key;}";
                            lines << "            currentIndex++;";
                            lines << "        }";
                            lines << "        return '';";
                            lines << "    }";

                            lines << "    uniboard.datastore.document.getItem = function(key) {";
                            lines << "        return uniboard.datastore.document[key];";
                            lines << "    }";

                            lines << "    uniboard.datastore.document.setItem = function(key, value) {}";
                            lines << "    uniboard.datastore.document.removeItem = function(key) {}";
                            lines << "    uniboard.datastore.document.clear = function() {}";

                            lines << "  </script>";
                            lines << "";

                            addedJs = true;
                        }
                    }
                }
                while (!line.isNull());

                startFile.resize(0);
                startFile.write(lines.join("\n").toUtf8()); // TODO UB 4.x detect real html encoding

                startFile.close();
            }
        }
    }
    else
    {
        qWarning() << "Remote Widget start file, cannot inject widget preferences and datastore entries";
    }
}


void UBDocumentPublisher::sendZipToUniboardWeb(const QString& zipFilePath, const QUuid& publishingUuid)
{
    QFile zipFile(zipFilePath);

    if(!zipFile.open(QIODevice::ReadOnly))
    {
        qWarning() << "Cannot open file" << zipFilePath << "for upload to Uniboard Web";
        return;
    }

    QUrl publishingEndpoint = QUrl(mPublishingServiceUrl);

    mUploadRequest = new UBServerXMLHttpRequest(UBNetworkAccessManager::defaultAccessManager()
        , "application/octet-stream");

    mUploadRequest->setVerbose(true);

    connect(mUploadRequest, SIGNAL(progress(qint64, qint64)), this,  SLOT(uploadProgress(qint64, qint64)));
    connect(mUploadRequest, SIGNAL(finished(bool, const QByteArray&)), this, SLOT(postZipUploadResponse(bool, const QByteArray&)));

    mUploadRequest->addHeader("Publishing-UUID", UBStringUtils::toCanonicalUuid(publishingUuid));
    mUploadRequest->addHeader("Document-UUID", UBStringUtils::toCanonicalUuid(mSourceDocument->uuid()));
    mUploadRequest->addHeader("Document-PageCount", QString("%1").arg(mSourceDocument->pageCount()));
    mUploadRequest->addHeader("Document-Title", mTitle);
    mUploadRequest->addHeader("Document-Author", mAuthor);
    mUploadRequest->addHeader("Document-AuthorEMail", mEMail);
    mUploadRequest->addHeader("Document-Description", mDescription);
    mUploadRequest->addHeader("Deletion-Token", UBStringUtils::toCanonicalUuid(QUuid::createUuid()));
    mUploadRequest->addHeader("Token-UUID", UBStringUtils::toCanonicalUuid(mAuthenticationUuid));
    mUploadRequest->addHeader("Token-Encrypted", mAuthenticationBase64Token);

    mUploadRequest->post(publishingEndpoint, zipFile.readAll());

    zipFile.remove();
}


void UBDocumentPublisher::uploadProgress(qint64 bytesSent, qint64 bytesTotal)
{
    int percentage = (((qreal)bytesSent / (qreal)bytesTotal ) * 100);

    if (bytesSent < bytesTotal)
        UBApplication::showMessage(tr("Upload to Uniboard Web in progress %1 %").arg(percentage), true);
    else
        UBApplication::showMessage(tr("Sending document ..."), true);

}


void UBDocumentPublisher::postZipUploadResponse(bool success, const QByteArray& payload)
{
    if (success)
    {
        UBApplication::showMessage(tr("The document has been sent to %1").arg(UBSettings::settings()->uniboardWebUrl->get().toString()), false);
    }
    else
    {
        qWarning() << "error uploading document to Uniboard Web" << QString::fromUtf8(payload);

        QString errorMessage = QString::fromUtf8(payload);

        if (errorMessage.length() == 0)
            UBApplication::showMessage(tr("Error while publishing document to %1")
                                       .arg(UBSettings::settings()->uniboardWebUrl->get().toString()), false);
        else
            UBApplication::showMessage(tr("Error while publishing document to %1 : (%2)")
                                       .arg(UBSettings::settings()->uniboardWebUrl->get().toString())
                                       .arg(errorMessage), false);
    }

    QApplication::restoreOverrideCursor();

    deleteLater();
}


UBDocumentPublishingDialog::UBDocumentPublishingDialog(QWidget *parent)
    : QDialog(parent)
{
    Ui::documentPublishingDialog::setupUi(this);

    connect(dialogButtons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(dialogButtons, SIGNAL(rejected()), this, SLOT(reject()));

    connect(title, SIGNAL(textChanged(const QString&)), this, SLOT(updateUIState(const QString&)));
    connect(email, SIGNAL(textChanged(const QString&)), this, SLOT(updateUIState(const QString&)));

    dialogButtons->button(QDialogButtonBox::Ok)->setEnabled(false);
    dialogButtons->button(QDialogButtonBox::Ok)->setText(tr("Publish"));
}


void UBDocumentPublishingDialog::updateUIState(const QString& string)
{
    Q_UNUSED(string);

    bool ok = title->text().length() > 0
                    &&  email->text().length() > 0;

    dialogButtons->button(QDialogButtonBox::Ok)->setEnabled(ok);
}
