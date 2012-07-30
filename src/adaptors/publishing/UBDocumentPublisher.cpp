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
#include <QFileInfo>

#include "UBDocumentPublisher.h"

#include "frameworks/UBPlatformUtils.h"
#include "frameworks/UBFileSystemUtils.h"
#include "frameworks/UBStringUtils.h"

#include "network/UBNetworkAccessManager.h"
#include "network/UBServerXMLHttpRequest.h"

#include "core/UBDocumentManager.h"
#include "core/UBApplication.h"
#include "core/UBPersistenceManager.h"
#include "core/UBApplicationController.h"

#include "board/UBBoardController.h"

#include "gui/UBMainWindow.h"

#include "document/UBDocumentProxy.h"
#include "document/UBDocumentContainer.h"

#include "domain/UBGraphicsWidgetItem.h"

#include "globals/UBGlobals.h"

THIRD_PARTY_WARNINGS_DISABLE
#include "quazip.h"
#include "quazipfile.h"
THIRD_PARTY_WARNINGS_ENABLE

#include "adaptors/UBExportFullPDF.h"
#include "adaptors/UBExportDocument.h"
#include "adaptors/UBSvgSubsetAdaptor.h"

#include "UBSvgSubsetRasterizer.h"

#include "../../core/UBApplication.h"

#include "core/memcheck.h"


UBDocumentPublisher::UBDocumentPublisher(UBDocumentProxy* pDocument, QObject *parent)
        : QObject(parent)
        , mSourceDocument(pDocument)
        , mUsername("")
        , mPassword("")
        , bLoginCookieSet(false)
{
	//NOOP
    init();
}


UBDocumentPublisher::~UBDocumentPublisher()
{
}


void UBDocumentPublisher::publish()
{
            //check that the username and password are stored on preferences
    UBSettings* settings = UBSettings::settings();

    if(settings->communityUsername().isEmpty() || settings->communityPassword().isEmpty()){
        UBApplication::showMessage(tr("Credentials has to not been filled out yet."));
        qDebug() << "trying to connect to community without the required credentials";
        return;
    }

    mUsername = settings->communityUsername();
    mPassword = settings->communityPassword();

    UBPublicationDlg dlg;
    if(QDialog::Accepted == dlg.exec())
    {
        mDocInfos.title = dlg.title();
        mDocInfos.description = dlg.description();

        buildUbwFile();

        UBApplication::showMessage(tr("Uploading Sankore File on Web."));

        sendUbw(mUsername, mPassword);
    }
}

void UBDocumentPublisher::buildUbwFile()
{
    QDir d;
    d.mkpath(UBFileSystemUtils::defaultTempDirPath());

    QString tmpDir = UBFileSystemUtils::createTempDir();

    if (UBFileSystemUtils::copyDir(mSourceDocument->persistencePath(), tmpDir))
    {
        QUuid publishingUuid = QUuid::createUuid();

        mPublishingPath = tmpDir;
        mPublishingSize = mSourceDocument->pageCount();

        rasterizeScenes();

        upgradeDocumentForPublishing();

        UBExportFullPDF pdfExporter;
        pdfExporter.setVerbode(false);
        pdfExporter.persistsDocument(mSourceDocument, mPublishingPath + "/" + UBStringUtils::toCanonicalUuid(publishingUuid) + ".pdf");

        UBExportDocument ubzExporter;
        ubzExporter.setVerbode(false);
        ubzExporter.persistsDocument(mSourceDocument, mPublishingPath + "/" + UBStringUtils::toCanonicalUuid(publishingUuid) + ".ubz");

        // remove all useless files

        for (int pageIndex = 0; pageIndex < mPublishingSize; pageIndex++) {
            QString filename = mPublishingPath + UBFileSystemUtils::digitFileFormat("/page%1.svg",pageIndex);

            QFile::remove(filename);
        }

        UBFileSystemUtils::deleteDir(mPublishingPath + "/" + UBPersistenceManager::imageDirectory);
        UBFileSystemUtils::deleteDir(mPublishingPath + "/" + UBPersistenceManager::objectDirectory);
        UBFileSystemUtils::deleteDir(mPublishingPath + "/" + UBPersistenceManager::videoDirectory);
        UBFileSystemUtils::deleteDir(mPublishingPath + "/" + UBPersistenceManager::audioDirectory);

        mTmpZipFile = UBFileSystemUtils::defaultTempDirPath() + "/" + UBStringUtils::toCanonicalUuid(QUuid::createUuid()) + ".ubw~";

        QuaZip zip(mTmpZipFile);
        zip.setFileNameCodec("UTF-8");
        if (!zip.open(QuaZip::mdCreate))
        {
            qWarning() << "Export failed. Cause: zip.open(): " << zip.getZipError() << "," << mTmpZipFile;
            QApplication::restoreOverrideCursor();
            return;
        }

        QuaZipFile outFile(&zip);

        if (!UBFileSystemUtils::compressDirInZip(mPublishingPath, "", &outFile, true))
        {
            qWarning("Export failed. compressDirInZip failed ...");
            zip.close();
            UBApplication::showMessage(tr("Export failed."));
            QApplication::restoreOverrideCursor();
            return;
        }

        if (zip.getZipError() != 0)
        {
            qWarning("Export failed. Cause: zip.close(): %d", zip.getZipError());
            zip.close();
            UBApplication::showMessage(tr("Export failed."));
            QApplication::restoreOverrideCursor();
            return;
        }

        zip.close();

    }
    else
    {
        UBApplication::showMessage(tr("Export canceled ..."));
        QApplication::restoreOverrideCursor();
    }
}

void UBDocumentPublisher::rasterizeScenes()
{

    for (int pageIndex = 0; pageIndex < mPublishingSize; pageIndex++)
    {
        UBApplication::showMessage(tr("Converting page %1/%2 ...").arg(UBDocumentContainer::pageFromSceneIndex(pageIndex)).arg(mPublishingSize), true);

        UBDocumentProxy publishingDocument(mPublishingPath);
        UBSvgSubsetRasterizer rasterizer(&publishingDocument, pageIndex);

        QString filename = mPublishingPath + UBFileSystemUtils::digitFileFormat("/page%1.jpg",pageIndex);

        rasterizer.rasterizeToFile(filename);

    }
}


void UBDocumentPublisher::updateGoogleMapApiKey()
{
    QDir widgestDir(mPublishingPath + "/" + UBPersistenceManager::widgetDirectory);

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

                if (file.open(QIODevice::ReadWrite))
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
    for (int pageIndex = 0; pageIndex < mPublishingSize; pageIndex++)
    {
        UBDocumentProxy publishingDocument(mPublishingPath);
        UBGraphicsScene *scene = UBSvgSubsetAdaptor::loadScene(&publishingDocument, pageIndex);

        QList<UBGraphicsW3CWidgetItem*> widgets;

        foreach(QGraphicsItem* item, scene->items()){
            UBGraphicsW3CWidgetItem *widgetItem = dynamic_cast<UBGraphicsW3CWidgetItem*>(item);

            if(widgetItem){
                generateWidgetPropertyScript(widgetItem, UBDocumentContainer::pageFromSceneIndex(pageIndex));
                widgets << widgetItem;
            }
        }

        QString filename = mPublishingPath + UBFileSystemUtils::digitFileFormat("/page%1.json",pageIndex);

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


void UBDocumentPublisher::generateWidgetPropertyScript(UBGraphicsW3CWidgetItem *widgetItem, int pageNumber)
{

    QMap<QString, QString> preferences = widgetItem->preferences();
    QMap<QString, QString> datastoreEntries = widgetItem->datastoreEntries();

    QString startFileName = widgetItem->w3cWidget()->mainHtmlFileName();

    if (!startFileName.startsWith("http://"))
    {
        QString startFilePath = mPublishingPath + "/" + UBPersistenceManager::widgetDirectory + "/" + widgetItem->uuid().toString() + ".wgt/" + startFileName;

        QFile startFile(startFilePath);

        if (startFile.exists())
        {
            if (startFile.open(QIODevice::ReadWrite))
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
                            lines << "    uniboard.pageCount = " + QString("%1").arg(mPublishingSize) + ";";
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
    else{
        qWarning() << "Remote Widget start file, cannot inject widget preferences and datastore entries";
    }
}

void UBDocumentPublisher::init()
{
    mCrlf=0x0d;
    mCrlf+=0x0a;
    mDocInfos.title = "";
    mDocInfos.description = "";

    mpCookieJar = new QNetworkCookieJar();
    mpNetworkMgr = new QNetworkAccessManager(this);

    connect(mpNetworkMgr, SIGNAL(finished(QNetworkReply*)), this, SLOT(onFinished(QNetworkReply*)));
}

void UBDocumentPublisher::onFinished(QNetworkReply *reply)
{
    QVariant cookieHeader = reply->rawHeader("Set-Cookie");
    // First we concatenate all the Set-Cookie values (the packet can contains many of them)
    QStringList qslCookie = cookieHeader.toString().split("\n");
    QString qsCookieValue = qslCookie.at(0);
    for (int i = 1; i < qslCookie.size(); i++) {
        qsCookieValue += "; " +qslCookie.at(i);
    }

    // Now we isolate every cookie value
    QStringList qslCookieVals = qsCookieValue.split("; ");

    bool bTransferOk = false;

    for(int j = 0; j < qslCookieVals.size(); j++)
    {
        qDebug() << j;
        if(qslCookieVals.at(j).startsWith("assetStatus"))
        {
            QStringList qslAsset = qslCookieVals.at(j).split("=");
            if(qslAsset.at(1) == "UPLOADED")
            {
                bTransferOk = true;
                break;
            }
        }
    }

    if(bTransferOk)
    {
        UBApplication::showMessage(tr("Document uploaded correctly on the web."));
    }
    else
    {
        UBApplication::showMessage(tr("Failed to upload document on the web."));
    }

    reply->deleteLater();
}

void UBDocumentPublisher::sendUbw(QString username, QString password)
{
    if (QFile::exists(mTmpZipFile))
    {
        QFile f(mTmpZipFile);
        if (f.open(QIODevice::ReadOnly))
        {
            QFileInfo fi(f);
            QByteArray ba = f.readAll();
            QString boundary,data, multipartHeader;
            QByteArray datatoSend;

            boundary = "---WebKitFormBoundaryDKBTgA53MiyWrzLY";
            multipartHeader = "multipart/form-data; boundary="+boundary;

            data="--"+boundary+mCrlf;
            data+="Content-Disposition: form-data; name=\"title\"" + mCrlf + mCrlf + mDocInfos.title + mCrlf;
            data+="--"+boundary+mCrlf;
            data+="Content-Disposition: form-data; name=\"description\"" + mCrlf + mCrlf + mDocInfos.description.remove("\n") + mCrlf;
            data+="--"+boundary+mCrlf;
            data+="Content-Disposition: form-data; name=\"file\"; filename=\""+ fi.fileName() +"\""+mCrlf;
            data+="Content-Type: application/octet-stream"+mCrlf+mCrlf;
            datatoSend=data.toAscii(); // convert data string to byte array for request
            datatoSend += ba;
            datatoSend += mCrlf;
            datatoSend += QString("--%0--%1").arg(boundary).arg(mCrlf);

            QNetworkRequest request(QUrl(QString(DOCPUBLICATION_URL).toAscii().constData()));

            request.setHeader(QNetworkRequest::ContentTypeHeader, multipartHeader);
            request.setHeader(QNetworkRequest::ContentLengthHeader,datatoSend.size());
            QString b64Auth = getBase64Of(QString("%0:%1").arg(username).arg(password));
            request.setRawHeader("Authorization", QString("Basic %0").arg(b64Auth).toAscii().constData());
            request.setRawHeader("Host", "planete.sankore.org");
            request.setRawHeader("Accept", "*/*");
            request.setRawHeader("Accept-Language", "en-US,*");

            mpCookieJar->setCookiesFromUrl(mCookies, QUrl(DOCPUBLICATION_URL));
            mpNetworkMgr->setCookieJar(mpCookieJar);

            // Send the file
            mpNetworkMgr->post(request,datatoSend);
        }
    }
}

QString UBDocumentPublisher::getBase64Of(QString stringToEncode)
{
    return stringToEncode.toAscii().toBase64();
}

// ---------------------------------------------------------
UBProxyLoginDlg::UBProxyLoginDlg(QWidget *parent, const char *name):QDialog(parent)
  , mpLayout(NULL)
  , mpUserLayout(NULL)
  , mpPasswordLayout(NULL)
  , mpButtons(NULL)
  , mpUserLabel(NULL)
  , mpPasswordLabel(NULL)
  , mpUsername(NULL)
  , mpPassword(NULL)
{
    setObjectName(name);
    setFixedSize(400, 150);
    setWindowTitle(tr("Proxy Login"));

    mpLayout = new QVBoxLayout();
    setLayout(mpLayout);
    mpUserLayout = new QHBoxLayout();
    mpLayout->addLayout(mpUserLayout);
    mpPasswordLayout = new QHBoxLayout();
    mpLayout->addLayout(mpPasswordLayout);

    mpUserLabel = new QLabel(tr("Username:"), this);
    mpUsername = new QLineEdit(this);
    mpUserLayout->addWidget(mpUserLabel, 0);
    mpUserLayout->addWidget(mpUsername, 1);

    mpPasswordLabel = new QLabel(tr("Password:"), this);
    mpPassword = new QLineEdit(this);
    mpPasswordLayout->addWidget(mpPasswordLabel, 0);
    mpPasswordLayout->addWidget(mpPassword, 1);

    mpButtons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    mpLayout->addWidget(mpButtons);

    connect(mpButtons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(mpButtons, SIGNAL(rejected()), this, SLOT(reject()));

}

UBProxyLoginDlg::~UBProxyLoginDlg()
{
    if(NULL != mpLayout)
    {
        delete mpLayout;
        mpLayout = NULL;
    }
    if(NULL != mpButtons)
    {
        delete mpButtons;
        mpButtons = NULL;
    }
    if(NULL != mpUserLabel)
    {
        delete mpUserLabel;
        mpUserLabel = NULL;
    }
    if(NULL != mpPasswordLabel)
    {
        delete mpPasswordLabel;
        mpPasswordLabel = NULL;
    }
    if(NULL != mpUsername)
    {
        delete mpUsername;
        mpUsername = NULL;
    }
    if(NULL != mpPassword)
    {
        delete mpPassword;
        mpPassword = NULL;
    }
}

// ---------------------------------------------------------
UBPublicationDlg::UBPublicationDlg(QWidget *parent, const char *name):QDialog(parent)
  , mpLayout(NULL)
  , mpTitleLayout(NULL)
  , mpTitleLabel(NULL)
  , mpTitle(NULL)
  , mpDescLabel(NULL)
  , mpDescription(NULL)
  , mpButtons(NULL)
{
    setObjectName(name);
    setWindowTitle(tr("Publish document on the web"));

    resize(500, 300);

    mpLayout = new QVBoxLayout();
    setLayout(mpLayout);

    mpTitleLabel = new QLabel(tr("Title:"), this);
    mpTitle = new QLineEdit(this);
    mpTitleLayout = new QHBoxLayout();
    mpTitleLayout->addWidget(mpTitleLabel, 0);
    mpTitleLayout->addWidget(mpTitle, 1);
    mpLayout->addLayout(mpTitleLayout, 0);

    mpDescLabel = new QLabel(tr("Description:"), this);
    mpLayout->addWidget(mpDescLabel, 0);

    mpDescription = new QTextEdit(this);
    mpLayout->addWidget(mpDescription, 1);

    mpButtons = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok, Qt::Horizontal, this);
    mpButtons->button(QDialogButtonBox::Ok)->setText(tr("Publish"));
    mpLayout->addWidget(mpButtons);

    mpButtons->button(QDialogButtonBox::Ok)->setEnabled(false);

    connect(mpButtons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(mpButtons, SIGNAL(rejected()), this, SLOT(reject()));
    connect(mpTitle, SIGNAL(textChanged(QString)), this, SLOT(onTextChanged()));
    connect(mpDescription, SIGNAL(textChanged()), this, SLOT(onTextChanged()));
}

UBPublicationDlg::~UBPublicationDlg()
{
    if(NULL != mpTitleLabel)
    {
        delete mpTitleLabel;
        mpTitleLabel = NULL;
    }
    if(NULL != mpTitle)
    {
        delete mpTitle;
        mpTitle = NULL;
    }
    if(NULL != mpDescLabel)
    {
        delete mpDescLabel;
        mpDescLabel = NULL;
    }
    if(NULL != mpDescription)
    {
        delete mpDescription;
        mpDescription = NULL;
    }
    if(NULL != mpButtons)
    {
        delete mpButtons;
        mpButtons = NULL;
    }
    if(NULL != mpTitleLayout)
    {
        delete mpTitleLayout;
        mpTitleLayout = NULL;
    }
    if(NULL != mpLayout)
    {
        delete mpLayout;
        mpLayout = NULL;
    }
}

void UBPublicationDlg::onTextChanged()
{
    bool bPublishButtonState = false;
    if(mpTitle->text() != "" && mpDescription->document()->toPlainText() != "")
    {
        bPublishButtonState = true;
    }
    else
    {
        bPublishButtonState = false;
    }

    mpButtons->button(QDialogButtonBox::Ok)->setEnabled(bPublishButtonState);
}
