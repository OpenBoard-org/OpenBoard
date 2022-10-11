/*
 * Copyright (C) 2015-2022 Département de l'Instruction Publique (DIP-SEM)
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

#include "UBForeignObjectsHandler.h"

#include <QtGui>
#include <QtXml>
#include "UBSettings.h"

const QString tVideo = "video";
const QString tAudio = "audio";
const QString tImage = "image";
const QString tForeignObject = "foreignObject";
const QString tTeacherGuide = "teacherGuide";
const QString tMedia = "media";
const QString tGroups = "groups";

const QString aHref = "xlink:href";
const QString aType = "ub:type";
const QString aReqExt = "requiredExtensions";
const QString aSrc = "ub:src";
const QString aMediaType = "mediaType";
const QString aRelativePath = "relativePath";
const QString aActionMedia = "ub:actionFirstParameter";

const QString vText = "text";
const QString vReqExt = "http://ns.adobe.com/pdf/1.3/";

const QString wgtSuff = ".wgt";
const QString thumbSuff = ".png";

const QString scanDirs = "audios,images,videos,teacherGuideObjects,widgets";
const QStringList trashFilter = QStringList() << "*.swf";

#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
typedef Qt::SplitBehaviorFlags SplitBehavior;
#else
typedef QString::SplitBehavior SplitBehavior;
#endif

static QString strIdFrom(const QString &filePath)
{
    if ((filePath).isEmpty()) {
        return QString();
    }

    static const QRegularExpression rx("\\{.(?!.*\\{).*\\}");
    QRegularExpressionMatch match = rx.match(filePath);
    if (!match.hasMatch()) {
        return QString();
    }

    return match.captured();
}

static bool rm_r(const QString &rmPath)
{
    QFileInfo fi(rmPath);
    if (!fi.exists()) {
        qDebug() << rmPath << "does not exist";
        return false;
    } else if (fi.isFile()) {
        if (!QFile::remove(rmPath)) {
            qDebug() << "can't remove file" << rmPath;
            return false;
        }
        return true;
    } else if (fi.isDir()) {
        QFileInfoList fList = QDir(rmPath).entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
        foreach (QFileInfo sub, fList) {
            rm_r(sub.absoluteFilePath());
        }
        if (!QDir().rmdir(rmPath)) {
            qDebug() << "can't remove dir" << rmPath;
            return false;
        }
        return true;
    }
    return false;
}

static bool cp_rf(const QString &what, const QString &where)
{
    QFileInfo whatFi(what);
    QFileInfo whereFi = QFileInfo(where);

    if (!whatFi.exists()) {
        qDebug() << what << "does not exist" << Q_FUNC_INFO;
        return false;
    } else if (whatFi.isFile()) {
        QString whereDir = where.section("/", 0, -2, QString::SectionSkipEmpty | QString::SectionIncludeLeadingSep);
        QString newFilePath = where;
        if (!whereFi.exists()) {
            QDir().mkpath(whereDir);
        } else if (whereFi.isDir()) {
            newFilePath = whereDir + "/" + whatFi.fileName();
        }
        if (QFile::exists(newFilePath)) {
            QFile::remove(newFilePath);
        }
        if (!QFile::copy(what, newFilePath)) {
            qDebug() << "can't copy" << what << "to" << where << Q_FUNC_INFO;
            return false;
        }
        return true;
    } else if (whatFi.isDir()) {

        if (whereFi.isFile() && whereFi.fileName().toLower() == whatFi.fileName().toLower()) {
            qDebug() << "can't copy dir" << what << "to file" << where << Q_FUNC_INFO;
            return false;
        } else if (whereFi.isDir()) {
            rm_r(where);
        }

        QDir().mkpath(where);

        QFileInfoList fList = QDir(what).entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
        foreach (QFileInfo sub, fList) {
            if (!cp_rf(sub.absoluteFilePath(), where + "/" + sub.fileName()))
            return false;
        }
        return true;
    }
    return true;
}

static QString thumbFileNameFrom(const QString &filePath)
{
    if (filePath.isEmpty()) {
        return QString();
    }

    static const QRegularExpression braces("[\\{\\}]");
    QString thumbPath = filePath;
    thumbPath.replace(braces, "").replace(wgtSuff, thumbSuff);

    return thumbPath;
}


QString svgPageName(int pageIndex)
{
    return QString("page%1.svg").arg(pageIndex, 3, 10, QLatin1Char('0'));
}

static QDomDocument createDomFromSvg(const QString &svgUrl)
{
    Q_ASSERT(QFile::exists(svgUrl));
    QString mFoldersXmlStorageName = svgUrl;

    if (QFileInfo(mFoldersXmlStorageName).exists()) {
        QDomDocument xmlDom;
        QFile inFile(mFoldersXmlStorageName);
        if (inFile.open(QIODevice::ReadOnly)) {
            QString domString(inFile.readAll());

            int errorLine = 0; int errorColumn = 0;
            QString errorStr;

            if (xmlDom.setContent(domString, &errorStr, &errorLine, &errorColumn)) {
                return xmlDom;
            } else {
                qDebug() << "Error reading content of " << mFoldersXmlStorageName << '\n'
                         << "Error:" << inFile.errorString()
                         << "Line:" << errorLine
                         << "Column:" << errorColumn;
            }
            inFile.close();
        } else {
            qDebug() << "Error reading" << mFoldersXmlStorageName << '\n'
                     << "Error:" << inFile.errorString();
        }
    }

    return QDomDocument();
}

class Cleaner
{
public:
    void cure(const QUrl &dir)
    {
        mCurrentDir = dir.toLocalFile();
        cleanTrash();

        // Gathering information from svg files
        QFileInfoList svgInfos = QDir(mCurrentDir).entryInfoList(QStringList() << "*.svg", QDir::NoDotAndDotDot | QDir::Files);
        foreach (QFileInfo svgInfo, svgInfos) {
            cureIdsFromSvgDom(createDomFromSvg(svgInfo.absoluteFilePath()));
        }

        fitIdsFromFileSystem();
        QVector<QString> deleteCandidates;
        findRedundandElements(deleteCandidates);

        foreach (QString key, deleteCandidates) {
            QString delPath = mPresentIdsMap.value(key);
            if (delPath.isNull()) {
                continue;
            } else if (delPath.endsWith(wgtSuff)) { //remove corresponding thumb
                QString thumbPath = thumbFileNameFrom(delPath);

                //N/C - NNE - 20140417
                if (QFile::exists(thumbPath)) {
                    rm_r(thumbPath);
                }
            }
            rm_r(delPath);
            // Clear parent dir if empty
            QDir dir(delPath);
            dir.cdUp();
            if (dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot).isEmpty()) {
                dir.rmdir(dir.absolutePath());
            }
        }

        qDebug() << "Ok on cure";
    }

private:
    void cleanTrash()
    {
        QFileInfoList ifs = QDir(mCurrentDir).entryInfoList(trashFilter, QDir::NoDotAndDotDot | QDir::Files);
        foreach (QFileInfo ifo, ifs) {
            rm_r(ifo.absoluteFilePath());
        }
    }

    void cureIdsFromSvgDom(const QDomDocument &dom)
    {
        Q_ASSERT(!dom.isNull());

        QDomElement nextElement = dom.documentElement().firstChildElement();
        while (!nextElement.isNull()) {
            QString nextTag = nextElement.tagName();
            qDebug() << "Tag name of the next parsed element is" << nextTag;
            if (nextTag == tGroups)
            {
               nextElement = nextElement.firstChildElement("group");
            }

            invokeFromText(nextTag, nextElement);
            nextElement = nextElement.nextSiblingElement();
        }
    }

    void fitIdsFromFileSystem()
    {
        QString absPrefix = mCurrentDir + "/";
        QStringList dirsList = scanDirs.split(",", SplitBehavior::SkipEmptyParts);
        foreach (QString dirName, dirsList) {
            QString absPath = absPrefix + dirName;
            if (!QFile::exists(absPath)) {
                continue;
            }
            fitIdsFromDir(absPath);
        }

    }

    void fitIdsFromDir(const QString &scanDir)
    {
        QFileInfoList fileList = QDir(scanDir).entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
        foreach (QFileInfo nInfo, fileList) {
            QString uid = strIdFrom(nInfo.fileName());
            if (uid.isNull()) {
                continue;
            }
            mPresentIdsMap.insert(uid, nInfo.absoluteFilePath());
        }
    }

    void findRedundandElements(QVector<QString> &v)
    {
        // Taking information from the physical file system
        QStringList domIds = mDomIdsMap.keys();
        QStringList presentIds = mPresentIdsMap.keys();
        v.resize(qMax(domIds.count(), presentIds.count()));
        QVector<QString>::iterator it_diff;

        it_diff=std::set_symmetric_difference(domIds.begin(), domIds.end()
                                              , presentIds.begin(), presentIds.end()
                                              , v.begin());
        v.resize(it_diff - v.begin());
    }

    void invokeFromText(const QString &what, const QDomElement &element)
    {
        if (what == tVideo
                || what == tAudio
                || what == tImage) {
            mediaToContainer(element);
        } else if (what == tForeignObject) {
            foreingObjectToContainer(element);

            //N/C - NNE - 20140317
            cleanObjectFolder(element);

            //N/C - NNE - 20140520
            //foreign object may referer resource which are not present in the svg
            addResourceIdToSvg(element);
        } else if (what == tTeacherGuide) {
            teacherGuideToContainer(element);
        }

        pullActionFromElement(element);
    }

    // N/C - NNE - 20140317 : When export, reduce the size of the ubz file
    void cleanObjectFolder(const QDomElement &element)
    {
        //QDomElement preference = element.firstChildElement("ub:preference");

        //N/C - NNE - 20141021
        QDomNodeList childrenNode = element.elementsByTagName("ub:preference");

        QVector<QString> objectsIdUsed;

        for(int i = 0; i < childrenNode.size(); i++){
            QDomElement preference = childrenNode.at(i).toElement();

            if(!preference.isNull()){
                QString value = preference.attribute("value");

                int findPos = value.indexOf("objects/");
                int endPos;

                //find all objects used
                while(findPos != -1){
                    endPos = value.indexOf("\"", findPos);
                    objectsIdUsed << value.mid(findPos, endPos - findPos);
                    findPos = value.indexOf("objects/", endPos);
                }
            }
        }
        //N/C - NNE - 20141021 : END


        QString path = element.attribute(aSrc);
        QString objectsFolderPath = mCurrentDir + "/" + path + "/objects/";

        QDir dir(objectsFolderPath);
        dir.setFilter(QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);

        //then check all files in the objects directory
        //delete the file not used (not in te objectIdUsed variable)
        QFileInfoList list = dir.entryInfoList();
        for (int i = 0; i < list.size(); i++) {
            QFileInfo fileInfo = list.at(i);

            if(!objectsIdUsed.contains("objects/"+fileInfo.fileName())){
                QFile(fileInfo.filePath()).remove();
            }

        }
    }
    // N/C - NNE - 20140317 : END

    //N/C - NNE - 20140520
    void addResourceIdToSvg(const QDomElement& element)
    {
        QDomElement textContent = element.firstChildElement("itemTextContent");

        QString value = textContent.text();

        int findPos = value.indexOf("images/");
        int endPos;

        //find all objects used
        while(findPos != -1){
            endPos = value.indexOf("\"", findPos);

            QString path = value.mid(findPos, endPos - findPos);

            QString uuid = path.split("/").at(1).split(".").at(0);

            mDomIdsMap.insert(uuid, path);

            findPos = value.indexOf("images/", endPos);
        }
    }
    //N/C - NNE - 20140520 : END

    void pullActionFromElement(const QDomElement &element)
    {
        if (!element.hasAttribute(aActionMedia)) {
            return;
        }

        QString path = element.attribute(aActionMedia);
        if (path.isNull()) {
            return;
        }

        QString uid = strIdFrom(path);
        if (uid.isNull()) {
            return;
        }

        mDomIdsMap.insert(uid, path);
    }

    void teacherGuideToContainer(const QDomElement &element)
    {
        QDomElement nMediaElement = element.firstChildElement(tMedia);
        while (!nMediaElement.isNull()) {

            QString path = nMediaElement.attribute(aRelativePath);
            if (path.isNull()) {
                continue;
            }

            QString uid = strIdFrom(path);
            if (uid.isNull()) {
                return;
            }
            mDomIdsMap.insert(uid, path);

            nMediaElement = nMediaElement.nextSiblingElement(tMedia);
        }
    }

    void mediaToContainer(const QDomElement &element)
    {
        QString path = element.attribute(aHref);
        if (path.isNull()) {
            return;
        }
        QString uid = strIdFrom(path);
        if (uid.isNull()) {
            return;
        }
        mDomIdsMap.insert(uid, path);
    }

    void foreingObjectToContainer(const QDomElement &element)
    {
        QString type = element.attribute(aType);
        if (type == vText) { // We don't have to care of the text object
            return;
        }

        QString path = element.attribute(aSrc);
        if (path.isNull()) {
            return;
        }

        QString uid = strIdFrom(path);
        if (uid.isNull()) {
            return;
        }

        mDomIdsMap.insert(uid, path);
    }

private:
    QString mCurrentDir;
    QDomDocument mSvgData;
    QMap<QString, QString> mDomIdsMap;
    QMap<QString, QString> mPresentIdsMap;
};

class PageCopier
{
public:
    void copyPage (const QUrl &fromDir, int fromIndex, const QUrl &toDir, int toIndex)
    {
        mFromDir = fromDir.toLocalFile();
        mToDir = toDir.toLocalFile();
        mFromIndex = fromIndex;
        mToIndex = toIndex;

        QString svgFrom = mFromDir + "/" + svgPageName(fromIndex);
        QString svgTo = toDir.toLocalFile() + "/" + svgPageName(toIndex);
        QDomDocument dd = createDomFromSvg(svgFrom);
        QFile fl(svgTo);
        if (!fl.open(QIODevice::WriteOnly)) {
            qDebug() << Q_FUNC_INFO << "can't open" << fl.fileName() << "for writing";
            return;
        }
        cureIdsFromSvgDom(dd);

        QTextStream str(&fl);
        dd.save(str, 0);
        fl.close();
        qDebug() << Q_FUNC_INFO;
    }

private:
    void cureIdsFromSvgDom(const QDomDocument &dom)
    {
        Q_ASSERT(!dom.isNull());

        QDomElement nextElement = dom.documentElement().firstChildElement();
        while (!nextElement.isNull( )) {
            qDebug() << "Tag name of the next parsed element is" << nextElement.tagName();
            QString nextTag = nextElement.tagName();
            cureFromText(nextTag, nextElement);
            nextElement = nextElement.nextSiblingElement();
        }
    }

    void cureFromText(const QString &tagName, QDomElement element)
    {
        if (tagName == tVideo
                || tagName == tAudio
                || tagName == tImage) {
            QString newRelative = cureNCopy(element.attribute(aHref));
            element.setAttribute(aHref, newRelative);
            if (element.hasAttribute(aActionMedia)) {
                QString newActionPath = cureNCopy(element.attribute(aActionMedia));
                element.setAttribute(aActionMedia, newActionPath);
            }
        } else if (tagName == tForeignObject) {
            //Pdf object is a special case. Detect if it ends with #reference
            QString reqExt = element.attribute(aReqExt);
            if (reqExt == vReqExt) { //pdf reference
                QString ref = element.attribute(aHref);
                int i = ref.indexOf("#page");
                QString dest = ref.replace(i, ref.length()-i, "");
                if (!QFileInfo::exists(dest))
                    cureNCopy(dest, false);
                if (ref.isEmpty()) {
                    return;
                }
                static const QRegularExpression pdfReference("^(.*pdf\\#page\\=).*$");
                ref.replace(pdfReference, QString("\\1%1").arg(mToIndex));
                return;
            }

            QString type = element.attribute(aType);
            if (type == vText) { // We don't have to care of the text object
                if (element.hasAttribute(aActionMedia)) {
                    QString newRelative = cureNCopy(element.attribute(aActionMedia));
                    element.setAttribute(aActionMedia, newRelative);
                }
                return;
            }
            QString newRelative = cureNCopy(element.attribute(aSrc));
            element.setAttribute(aSrc, newRelative);
        } else if (tagName == tTeacherGuide) {
            QDomElement nMediaElement = element.firstChildElement(tMedia);
            while (!nMediaElement.isNull()) {
                QString newRelative = cureNCopy(nMediaElement.attribute(aRelativePath));
                nMediaElement.setAttribute(aRelativePath, newRelative);
                nMediaElement = nMediaElement.nextSiblingElement(tMedia);
            }
        }
    }

    QString cureNCopy(const QString &relativePath, bool createNewUuid=true)
    {
        QString relative = relativePath;
        if (createNewUuid)
        {
            QUuid newUuid = QUuid::createUuid();
            static const QRegularExpression bracedUuid("\\{.*\\}");
            QString newPath = relative.replace(bracedUuid, newUuid.toString());

            cp_rf(mFromDir + "/" + relativePath, mToDir + "/" + newPath);

            return newPath;
        }
        else
        {
            cp_rf(mFromDir + "/" + relativePath, mToDir + "/" + relativePath);
            return relativePath;
        }
    }

private:
    QString mFromDir;
    QString mToDir;
    int mFromIndex;
    int mToIndex;
};

class UBForeighnObjectsHandlerPrivate {
    UBForeighnObjectsHandlerPrivate(UBForeighnObjectsHandler *pq)
        : q(pq)
    {
    }

public:
    void cure(const QUrl &dir)
    {
        Cleaner *cleaner = new Cleaner;
        cleaner->cure(dir);
        delete cleaner;
        cleaner = 0;
    }

    void copyPage (const QUrl &fromDir, int fromIndex, const QUrl &toDir, int toIndex)
    {
        PageCopier *copier = new PageCopier;
        copier->copyPage(fromDir, fromIndex, toDir, toIndex);
        delete copier;
        copier = 0;
    }

private:
    UBForeighnObjectsHandler *q;
    friend class UBForeighnObjectsHandler;
};

UBForeighnObjectsHandler::UBForeighnObjectsHandler()
    : d(new UBForeighnObjectsHandlerPrivate(this))
{

}

UBForeighnObjectsHandler::~UBForeighnObjectsHandler()
{
    delete d;
}

void UBForeighnObjectsHandler::cure(const QList<QUrl> &dirs)
{
    foreach (QUrl dir, dirs) {
        cure(dir);
    }
}

void UBForeighnObjectsHandler::cure(const QUrl &dir)
{
    d->cure(dir);
}

void UBForeighnObjectsHandler::copyPage(const QUrl &fromDir, int fromIndex, const QUrl &toDir, int toIndex)
{
    d->copyPage(fromDir, fromIndex, toDir, toIndex);
}

