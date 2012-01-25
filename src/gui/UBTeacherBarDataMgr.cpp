#include "UBTeacherBarDataMgr.h"

#include "core/UBApplication.h"
#include "core/UBPersistenceManager.h"

#include "board/UBBoardController.h"

#include "customWidgets/UBGlobals.h"

#include "adaptors/UBMetadataDcSubsetAdaptor.h"


UBTeacherBarDataMgr::UBTeacherBarDataMgr()
{

}

UBTeacherBarDataMgr::~UBTeacherBarDataMgr()
{

}

void UBTeacherBarDataMgr::clearLists()
{
    mActionList.clear();
    mUrlList.clear();
    mMediaList.clear();
    mMediaUrls.clear();
}

void UBTeacherBarDataMgr::saveContent()
{
    // Store the page information in the UBZ
    sTeacherBarInfos infos;

    // Page Title
    infos.title = mPageTitle;
    // Actions
    foreach(sAction action, mActionList){
        infos.actions << QString("%0;%1").arg(action.type).arg(action.content);
    }
    // Media
    foreach(QString media, mMediaUrls){
        infos.medias << media;
    }
    // Links
    foreach(sLink link, mUrlList){
        if("" != link.title && "" != link.link){
            infos.urls << QString("%0;%1").arg(link.title).arg(link.link);
        }
    }
    // Comments
    infos.comments = mComments;

    UBDocumentProxy* documentProxy = UBApplication::boardController->activeDocument();
    if(documentProxy){
        UBPersistenceManager::persistenceManager()->persistTeacherBar(documentProxy, UBApplication::boardController->activeSceneIndex(), infos);

        documentProxy->setSessionTitle(mSessionTitle);
        documentProxy->setSessionTarget(mSessionTarget);
        documentProxy->setSessionLicence(mSessionLicence);

        UBMetadataDcSubsetAdaptor::persist(documentProxy);
    }
}



void UBTeacherBarDataMgr::loadContent(bool docChanged)
{
    clearLists();
    UBDocumentProxy* documentProxy = UBApplication::boardController->activeDocument();

    sTeacherBarInfos nextInfos = UBPersistenceManager::persistenceManager()->getTeacherBarInfos(documentProxy, UBApplication::boardController->activeSceneIndex());
    if(true/*docChanged*/){
        mSessionTitle = documentProxy->sessionTitle();
        mSessionTarget = documentProxy->sessionTarget();
        mSessionLicence = documentProxy->sessionLicence();
    }

    // Page Title
    mPageTitle = nextInfos.title;
    // Actions
    foreach(QString eachAction, nextInfos.actions){
        QStringList qslAction = eachAction.split(";");
        if(2 <= qslAction.size()){
            sAction action;
            action.type = qslAction.at(0).toInt();
            action.content = qslAction.at(1);
            mActionList << action;
        }
    }
    // Media URL
    if((nextInfos.medias.size() == 1) && (nextInfos.medias.at(0) == "")){
        // Do not retrieve it
    }
    else{
        mMediaUrls = nextInfos.medias;
    }

    // Links
    foreach(QString eachUrl, nextInfos.urls){
        QStringList qslUrl = eachUrl.split(';');
        if(2 <= qslUrl.size()){
            sLink link;
            link.title = qslUrl.at(0);
            link.link = qslUrl.at(1);
            mUrlList << link;
        }
    }

    // Comments
    mComments = nextInfos.comments;

    // TODO : Read the document metadata file and populate the metadata infos here

}

