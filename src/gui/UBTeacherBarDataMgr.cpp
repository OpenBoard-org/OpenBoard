#include "core/UBApplication.h"
#include "core/UBPersistenceManager.h"
#include "board/UBBoardController.h"
#include "customWidgets/UBGlobals.h"

#include "UBTeacherBarDataMgr.h"

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

    UBPersistenceManager::persistenceManager()->persistTeacherBar(UBApplication::boardController->activeDocument(), UBApplication::boardController->activeSceneIndex(), infos);

    // TODO: Store the document metadata somewhere
    // Session Title
    //... = mSessionTitle;
    // Session Target
    //... = mSessionTarget;

}

void UBTeacherBarDataMgr::loadContent(bool docChanged)
{
    clearLists();

    sTeacherBarInfos nextInfos = UBPersistenceManager::persistenceManager()->getTeacherBarInfos(UBApplication::boardController->activeDocument(), UBApplication::boardController->activeSceneIndex());

    if(docChanged){
        // TODO: Read these information from the metadata file

        // Session Title
        //mSessionTitle = ...;
        // Session Target
        //mSessionTarget = ...;
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

