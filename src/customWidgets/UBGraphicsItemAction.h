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



#ifndef UBGRAPHICSITEMSACTIONS_H
#define UBGRAPHICSITEMSACTIONS_H

#include <QObject>
#include <phonon>


enum eUBGraphicsItemMovePageAction {
    eMoveToFirstPage = 0,
    eMoveToLastPage,
    eMoveToPreviousPage,
    eMoveToNextPage,
    eMoveToPage
} ;

enum eUBGraphicsItemLinkType
{
    eLinkToAudio = 0,
    eLinkToPage,
    eLinkToWebUrl
};

class UBGraphicsItemAction : public QObject
{
    Q_OBJECT
public:
    UBGraphicsItemAction(eUBGraphicsItemLinkType linkType,QObject* parent = 0);
    virtual void play() = 0;
    virtual QStringList save() = 0;
    virtual void actionRemoved();
    virtual QString path() {return "";}
    eUBGraphicsItemLinkType linkType() { return mLinkType;}

signals:

public slots:
private:
    eUBGraphicsItemLinkType mLinkType;

};


class UBGraphicsItemPlayAudioAction : public UBGraphicsItemAction
{
    Q_OBJECT

public:
    UBGraphicsItemPlayAudioAction(QString audioFile, bool onImport = true, QObject* parent = 0);
    UBGraphicsItemPlayAudioAction();
    ~UBGraphicsItemPlayAudioAction();
    void play();
    QStringList save();
    void actionRemoved();
    QString path() {return mAudioPath;}
    void setPath(QString audioPath);
    QString fullPath();

public slots:
    void onSourceHide();

private:
    QString mAudioPath;
    Phonon::MediaObject *mMediaObject;
    Phonon::AudioOutput *mAudioOutput;
    bool mIsLoading;
    QString mFullPath;
};


class UBGraphicsItemMoveToPageAction : public UBGraphicsItemAction
{
    Q_OBJECT

public:
    UBGraphicsItemMoveToPageAction(eUBGraphicsItemMovePageAction actionType, int page = 0, QObject* parent = 0);
    void play();
    QStringList save();
    int page(){return mPage;}
    eUBGraphicsItemMovePageAction actionType(){return mActionType;}

private:
    eUBGraphicsItemMovePageAction mActionType;
    int mPage;

};

class UBGraphicsItemLinkToWebPageAction : public UBGraphicsItemAction
{
    Q_OBJECT

public:
    UBGraphicsItemLinkToWebPageAction(QString url, QObject* parent = 0);
    void play();
    QStringList save();
    QString url(){return mUrl;}

private:
    QString mUrl;
};

#endif // UBGRAPHICSITEMSACTIONS_H
