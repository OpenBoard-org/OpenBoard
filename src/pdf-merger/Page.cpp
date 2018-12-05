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




#include "Page.h"
#include "Document.h"

#include "ContentHandler.h"
#include "Exception.h"
#include "MediaBoxElementHandler.h"
#include "CropBoxElementHandler.h"
#include "TypeElementHandler.h"
#include "RemoveHimSelfHandler.h"
#include "AnnotsHandler.h"
#include "RotationHandler.h"
#include "FlateDecode.h"
#include "Utils.h"
#include "Rectangle.h"
#include "Filter.h"
#include <iostream>
#include <string.h>
#include "Parser.h"

#include "core/memcheck.h"

using namespace merge_lib;


Page::Page(unsigned int pageNumber): _root(NULL),_pageNumber(pageNumber), _rotation(0)
{

}



Page::~Page()
{

}


std::string & Page::getPageContent()
{
   return _root->getObjectContent();
}

const Object::Children &  Page::getPageRefs()
{
   return _root->getChildren();
}


void Page::recalculateObjectNumbers(unsigned int & newNumber)
{
   _root->recalculateObjectNumbers(newNumber);
}

Object * Page::pageToXObject(std::vector<Object *> & allObjects, std::vector<Object *> & annots, bool isCloneNeeded)
{
   Object * xObject = (isCloneNeeded) ? _root->getClone(allObjects) : _root;
   return _pageToXObject(xObject, annots);
}


Object * Page::_pageToXObject(Object *& page, std::vector<Object *> & annots)
{   
   RemoveHimselfHandler   * removeParent = new RemoveHimselfHandler(page, "/Parent");
   RemoveHimselfHandler   * removeBleedBox = new RemoveHimselfHandler(page, "/BleedBox");
   RemoveHimselfHandler   * removeTrimBox = new RemoveHimselfHandler(page, "/TrimBox");
   RemoveHimselfHandler   * removeArtBox = new RemoveHimselfHandler(page, "/ArtBox");
   RemoveHimselfHandler   * removeBoxColorInfo = new RemoveHimselfHandler(page, "/BoxColorInfo");
   RemoveHimselfHandler   * removeRotate = new RemoveHimselfHandler(page, "/Rotate");
   RemoveHimselfHandler   * removeThumb = new RemoveHimselfHandler(page, "/Thumb");
   RemoveHimselfHandler   * removeB = new RemoveHimselfHandler(page, "/B");
   RemoveHimselfHandler   * removeDur = new RemoveHimselfHandler(page, "/Dur");
   RemoveHimselfHandler   * removeTrans = new RemoveHimselfHandler(page, "/Trans");
   AnnotsHandler          * removeAnnots = new AnnotsHandler(page, "/Annots", annots);
   RemoveHimselfHandler   * removeAA = new RemoveHimselfHandler(page, "/AA");
   RemoveHimselfHandler   * removeID = new RemoveHimselfHandler(page, "/ID");
   RemoveHimselfHandler   * removePZ = new RemoveHimselfHandler(page, "/PZ");
   RemoveHimselfHandler   * removeSeparationInfo = new RemoveHimselfHandler(page, "/SeparationInfo");
   RemoveHimselfHandler   * removeTabs = new RemoveHimselfHandler(page, "/Tabs");
   RemoveHimselfHandler   * removeTemplateInstantiated = new RemoveHimselfHandler(page, "/TemplateInstantiated");
   RemoveHimselfHandler   * removePresSteps = new RemoveHimselfHandler(page, "/PresSteps");
   RemoveHimselfHandler   * removeUserUnit = new RemoveHimselfHandler(page, "/UserUnit");
   RemoveHimselfHandler   * removeVP = new RemoveHimselfHandler(page, "/VP");
   ContentHandler         * contentHandler = new ContentHandler(page, "/Contents");   
   CropBoxElementHandler  * cropBoxElementHandler = new CropBoxElementHandler(page);   
   MediaBoxElementHandler * mediaBoxElementHandler = new MediaBoxElementHandler(page);
   TypeElementHandler     * typeElementHandler = new TypeElementHandler(page);

   cropBoxElementHandler->addNextHandler(mediaBoxElementHandler);
   mediaBoxElementHandler->addNextHandler(removeParent);
   removeParent->addNextHandler(removeBleedBox);
   removeBleedBox->addNextHandler(removeTrimBox);
   removeTrimBox->addNextHandler(removeArtBox);
   removeArtBox->addNextHandler(removeBoxColorInfo);
   removeBoxColorInfo->addNextHandler(removeRotate);
   removeRotate->addNextHandler(removeThumb);
   removeThumb->addNextHandler(removeB);
   removeB->addNextHandler(removeDur);
   removeDur->addNextHandler(removeTrans);
   removeTrans->addNextHandler(removeAnnots);
   removeAnnots->addNextHandler(removeAA);
   removeAA->addNextHandler(removeID);
   removeID->addNextHandler(removePZ);
   removePZ->addNextHandler(removeSeparationInfo);
   removeSeparationInfo->addNextHandler(removeTabs);
   removeTabs->addNextHandler(removeTemplateInstantiated);
   removeTemplateInstantiated->addNextHandler(removePresSteps);
   removePresSteps->addNextHandler(removeUserUnit);
   removeUserUnit->addNextHandler(removeVP);
   removeVP->addNextHandler(typeElementHandler);
   typeElementHandler->addNextHandler(contentHandler);   

   cropBoxElementHandler->processObjectContent();
   cropBoxElementHandler->changeObjectContent();

   delete cropBoxElementHandler;
   return page;
}


std::string _getContentOfContentObject(MergePageDescription & description)
{
   std::string content("<<\n/Length ");
   std::string stream = "";
   if(!description.skipBasePage)
   {
      stream.append("1.000000 0 0 1.000000 0 0 cm\n"
         "q\n");
      stream.append(description.basePageTransformation.getCMT());
      stream.append("/OriginalPage2 Do\n"
         "Q\n");

   }
   if(!description.skipOverlayPage)
   {    
      stream.append("1.000000 0 0 1.000000 0 0 cm\n"
         "q\n");
      stream.append(description.overlayPageTransformation.getCMT());
      stream.append("/OriginalPage1 Do\n"
         "Q\n");
   }
   FlateDecode encoder;
   encoder.encode(stream);
   content.append(Utils::uIntToStr(stream.size()));
   content.append("\n/Filter /FlateDecode\n>>\nstream\n");
   content.append(stream);
   content.append("endstream\n");
   return content;
}

void _recalculateAnnotsCoordinates(Object * annotation, 
                                   const Rectangle & basePagesRectangle,
                                   const Rectangle & outputPagesRectangle,
                                   const MergePageDescription & description)
{
   Q_UNUSED(outputPagesRectangle);
   Q_UNUSED(basePagesRectangle);
   std::string annotsRectangleName("/Rect");
   Object * objectWithRectangle;
   unsigned int fake;
   annotation->findObject(annotsRectangleName, objectWithRectangle, fake);
   std::string annotContent = objectWithRectangle->getObjectContent();

   Rectangle annotsRectangle(annotsRectangleName.c_str(), annotContent);

   //we move annotation from base page to output page
   //that's way annotation should be scaled before all transformations.
   //Annotation's coordinates should be recalculated according to new 
   //page width and height

   annotsRectangle.recalculateInternalRectangleCoordinates(description.basePageTransformation.getAnnotsTransformations());
   annotsRectangle.updateRectangle(annotation, " ");
}

// function updates parent reference of annotation with new page object
static void _updateAnnotParentPage(Object *annotation,Object *newParentPage)
{
   if( annotation )
   {
      std::string strP = "/P";
      std::string &annotContent = annotation->getObjectContent();

      size_t startOfP = Parser::findTokenName(annotContent,strP);
      if((int) startOfP == -1 )
      {
         return;
      }
      size_t endOfP = Parser::findEndOfElementContent(annotContent,startOfP + strP.size());
      
      // lets find object with reference to parent
      std::vector<Object *> children = annotation->getChildrenByBounds(startOfP,endOfP);
      if( children.size() == 0 ) 
      {
         return;
      }

      Object *childWithP = children[0];
      if( childWithP )
      {
         Object::ReferencePositionsInContent pagePosition = annotation->removeChild(childWithP);
         annotation->eraseContent(startOfP,endOfP-startOfP);
         std::stringstream strout;
         strout <<"/P "<<newParentPage->getObjectNumber()<<" "<<newParentPage->getgenerationNumber()<<" R\n";
         // to compensate posible deviation
         for(size_t i =strout.str().size();i<endOfP-startOfP;i++)
         {
            strout<<" ";
         }
         annotation->insertToContent(startOfP,strout.str());
         annotation->addChild(newParentPage,pagePosition);
      }
   }
}

// function performs adjusting of some color parameters of annotation
// to avoid interference with overlay content
static void _updateAnnotFormColor(Object *annotation )
{
   std::string &objectContent = annotation->getObjectContent();
   if((int) objectContent.find("/Widget") == -1 )
   {
      return;
   }
   size_t startOfAP = Parser::findTokenName(objectContent,"/AP");
   if((int) startOfAP == -1 )
   {
      return;
   }
   size_t endOfAP = objectContent.find(">>", startOfAP);
   std::vector<Object *>  aps = annotation->getChildrenByBounds(startOfAP, endOfAP);
   for(size_t i = 0; i < aps.size(); ++i)
   {
      Object * childWithAP = aps[i];
      if( !childWithAP->hasStream() )
      {
         continue;
      }
      // first lets obtain and decode stream of Annotation appearrence stream
      std::string & content = childWithAP->getObjectContent();

      Filter filter(childWithAP);
      std::string decodedStream;
      filter.getDecodedStream(decodedStream);

      // lets iterate over stream and find operator f and remove it!
      size_t beg = 0;
      size_t found = 0;
      std::string token;
      while(Parser::getNextWord(token,decodedStream,beg,&found))
      {
         if( token == "f" || token == "F" )
         {
             if((int) found != -1 )
            {
               decodedStream[found] = ' ';
            }
            break;
         }
      }
      // Then we need to update Filter section (if any)
      std::string filterStr = "/Filter";
      size_t startOfFlate = Parser::findTokenName(content,filterStr);
      if((int) startOfFlate != -1 )
      {
         size_t endOfFlate = Parser::findEndOfElementContent(content,startOfFlate+filterStr.size());
         childWithAP->eraseContent(startOfFlate,endOfFlate-startOfFlate);
         //encode and put new stream to object content
         childWithAP->insertToContent(startOfFlate,"/Filter /FlateDecode ");
         FlateDecode flate;
         flate.encode(decodedStream);
      }

      // update the length field
      std::string lengthStr = "/Length";
      size_t startOfLength = Parser::findTokenName(content,lengthStr,0);
      if((int) startOfLength != -1 )
      {
         size_t endOfLength = Parser::findEndOfElementContent(content,startOfLength + lengthStr.size());
         childWithAP->eraseContent(startOfLength,endOfLength-startOfLength);
         std::stringstream ostr;
         ostr<<"/Length "<< decodedStream.size()<<"\n";
         childWithAP->insertToContent(startOfLength,ostr.str());

         // update the stream of object with new content
         std::string stream("stream");
         size_t leftBoundOfContentStream = content.find(stream);
         if((int) leftBoundOfContentStream != -1 )
         {
            size_t rightBoundOfContentStream = content.find("endstream", leftBoundOfContentStream);
            if((int) rightBoundOfContentStream == -1 )
            {
               rightBoundOfContentStream = content.size() - 1;
            }
            childWithAP->eraseContent(leftBoundOfContentStream, rightBoundOfContentStream - leftBoundOfContentStream);
            decodedStream.insert(0,"\nstream\n");
            childWithAP->insertToContent(leftBoundOfContentStream,decodedStream);
            childWithAP->appendContent("endstream\n");
            childWithAP->forgetStreamInFile();
         }
      }
   }
}
// sometimes page object does not have resources,  
// they are inherited from parent object
// this method processes such cases and insert resources from parent to page 
// for correct X-Object transformation
static void processBasePageResources(Object *basePage)
{
   if( basePage == NULL )
   {
      return;
   }
   std::string resourceToken = "/Resources";
   if((int) Parser::findTokenName(basePage->getObjectContent(),resourceToken) == -1 )
   {
      // it seems base page does not have resources, they can be located in parent!
      Object *resource = basePage->findPatternInObjOrParents(resourceToken);
      if( resource )
      {
         std::string &resContStr = resource->getObjectContent();
         size_t startOfRes = Parser::findTokenName(resContStr,resourceToken);
         if((int) startOfRes == -1 )
         {
            // no resources at all
            return;
         }
         size_t endOfRes = Parser::findEndOfElementContent(resContStr,startOfRes + resourceToken.size());
         if((int) endOfRes == -1 )
         {
            return; // broken resources
         }
         std::string resourceContent = resContStr.substr(startOfRes,endOfRes-startOfRes);

         size_t positionToInsert = basePage->getObjectContent().find("<<");
         if((int) positionToInsert == -1 )
         {
            positionToInsert = 0;
            resourceContent.insert(0,"<<");
            resourceContent.append(">>");
         }
         else
         {
            positionToInsert += strlen("<<");
         }
         // insert obtained resources to base page
         basePage->insertToContent(positionToInsert,resourceContent);

         // if resource contains childs, then we need to add reference to them to current object
         std::vector<Object *>  resChilds = resource->getChildrenByBounds(startOfRes, endOfRes);
         std::vector<Object *>::const_iterator objectIt(resChilds.begin());

         const Object::Children & children = resource->getChildren();

         for(; objectIt != resChilds.end(); objectIt++ )
         {
            Object::Children::const_iterator  childrenIt = children.find( (*objectIt)->getObjectNumber());
            if( childrenIt != children.end() )
            {
               Object::ReferencePositionsInContent refPositionInCont = (*childrenIt).second.second;

               Object::ReferencePositionsInContent::iterator positionIt(refPositionInCont.begin());

               Object::ReferencePositionsInContent newPositions;
               for( ;positionIt != refPositionInCont.end(); positionIt++ )
               {
                  newPositions.push_back( (*positionIt) - startOfRes + positionToInsert );
               }

               basePage->addChild( (*objectIt),newPositions );
            }
         }
      }
   }
}

std::string Page::_getMergedPageContent(  unsigned int & contentPosition, 
                                        unsigned int & parentPosition, 
                                        unsigned int & originalPage1Position, 
                                        unsigned int & originalPage2Position,
                                        std::pair<unsigned int, unsigned int> originalPageNumbers, 
                                        const MergePageDescription & description,
                                        Object * basePage,
                                        const std::vector<Object *> & annots,
                                        std::vector <Object::ChildAndItPositionInContent> & annotsPositions
                                        )
{
   std::string content("<<\n/Type /Page\n");   
   content.append("/Contents ");
   contentPosition = content.size();
   //object number 1 will be recalculated during serialization
   content.append("1 0 R\n");
   Rectangle mediaBox("/MediaBox");
   mediaBox.x2 = description.outPageWidth;
   mediaBox.y2 = description.outPageHeight;
   mediaBox.appendRectangleToString(content, " ");
   content.append("/Parent ");
   parentPosition = content.size();
   //object number 1 will be recalculated during serialization
   content.append("1 0 R\n");   
   content.append("/Resources <<\n"
      "/ProcSet [ /PDF /Text /ImageB /ImageC /ImageI ]\n"
      "/XObject <<\n");
   if(!description.skipOverlayPage)
   {
      content.append("/OriginalPage1 ");
      originalPage1Position = content.size();
      content.append(Utils::uIntToStr(originalPageNumbers.first));
      content.append(" 0 R\n");
   }
   if(!description.skipBasePage)
   {
      content.append("/OriginalPage2 ");
      originalPage2Position = content.size();
      content.append(Utils::uIntToStr(originalPageNumbers.second));
      content.append(" ");
      content.append(Utils::uIntToStr(basePage->getgenerationNumber()));
      content.append(" R\n");
   }
   content.append(">>\n>>\n");

   content.append("/Annots [ ");
   if(!description.skipBasePage)
   {
      Rectangle basePageRectangle("/BBox", basePage->getObjectContent());
      for(size_t i = 0; i < annots.size(); ++i)
      {
         _updateAnnotFormColor(annots[i]);
         _recalculateAnnotsCoordinates(annots[i], basePageRectangle, mediaBox, description);
         Object::ReferencePositionsInContent annotationPosition;
         annotationPosition.push_back(content.size());
         Object::ChildAndItPositionInContent annotAndItPosition(annots[i], annotationPosition);
         annotsPositions.push_back(annotAndItPosition);
         content.append(Utils::uIntToStr(annots[i]->getObjectNumber()));
         content.append(" ");
         content.append(Utils::uIntToStr(annots[i]->getgenerationNumber()));
         content.append(" R ");
      }
   }
   content.append("] \n>>\n");

   return content;
}

void Page::merge(Page * sourcePage, Document * parentDocument, MergePageDescription & description, bool isPageDuplicated)
{   
   if( sourcePage == NULL )
   {
      description.skipBasePage = true;
   }
   if(!description.skipOverlayPage)
   {
      // Lets recalculate final transformation of overlay page
      // before it will be places into XObject
      Rectangle mediaBox("/MediaBox",_root->getObjectContent());
      description.overlayPageTransformation.recalculateTranslation(mediaBox.getWidth(),mediaBox.getHeight());

      std::vector<Object *> fake;
      _pageToXObject(_root, fake);
   }

   std::vector<Object *> toAllObjects;
   std::vector<Object *> annotations;
   Object * sourcePageToXObject = 0;
   if(!description.skipBasePage)
   {  
      RotationHandler rotationHandler(sourcePage->_root, "/Rotate", *this);
      rotationHandler.processObjectContent();
      description.basePageTransformation.addRotation(_rotation);

      if((int) sourcePage->_root->getObjectContent().find("/Annots") != -1 )
      {
         Object *crop = sourcePage->_root->findPatternInObjOrParents("/CropBox");
         if( crop )
         {
            // we need to calculate special compensational shifting
            // for annotations if cropbox is starting not from 0,0
            Rectangle mediaBox("/CropBox", crop->getObjectContent());
            if( !Utils::doubleEquals(mediaBox.x1,0) || !Utils::doubleEquals(mediaBox.y1,0) )
            {
               double shiftX = Utils::doubleEquals(mediaBox.x1,0)?0:-mediaBox.x1;
               double shiftY = Utils::doubleEquals(mediaBox.y1,0)?0:-mediaBox.y1;
               Translation compensation(shiftX,shiftY);
               description.basePageTransformation.addAnnotsTransformation(compensation);
            }
         }
      }
      processBasePageResources(sourcePage->_root);
      sourcePageToXObject = sourcePage->pageToXObject(toAllObjects, annotations, isPageDuplicated);
      Rectangle mediaBox("/BBox", sourcePageToXObject->getObjectContent());
      description.basePageTransformation.recalculateTranslation(mediaBox.getWidth(),mediaBox.getHeight());      
   }

   Object * catalog = 0;
   unsigned int fake;
   if(!parentDocument->getDocumentObject()->findObject(std::string("/Kids"), catalog, fake))
   {
      std::string error("Wrong document ");
      error.append("There is no object with Kids field");
      throw Exception(error);
   }
   Object::ReferencePositionsInContent pagePosition = catalog->removeChild(_root);
   //create merged Page
   unsigned int contentPosition, parentPosition, originalPage1Position, originalPage2Position;
   std::pair<unsigned int, unsigned int> originalPageNumbers(_root->getObjectNumber(), 0);
   if(!description.skipBasePage)
      originalPageNumbers.second = sourcePageToXObject->getObjectNumber();
   std::vector <Object::ChildAndItPositionInContent> annotsAndItPositions;
   std::string mergedPageContent = _getMergedPageContent(contentPosition, 
      parentPosition, 
      originalPage1Position, 
      originalPage2Position, 
      originalPageNumbers, 
      description,
      sourcePageToXObject,
      annotations,
      annotsAndItPositions
      );
   Object * mergedPage = new Object(_root->getObjectNumber(), _root->getgenerationNumber(), mergedPageContent);
   toAllObjects.push_back(mergedPage);
   std::vector < unsigned int > contentPositionVec, parentPositionVec, originalPage1PositionVec, originalPage2PositionVec;
   contentPositionVec.push_back(contentPosition);
   parentPositionVec.push_back(parentPosition);
   originalPage1PositionVec.push_back(originalPage1Position);
   originalPage2PositionVec.push_back(originalPage2Position);
   Object * contentOfMergedPage = new Object(1, 0, _getContentOfContentObject(description));
   toAllObjects.push_back(contentOfMergedPage);
   parentDocument->addToAllObjects(toAllObjects);
   mergedPage->addChild(contentOfMergedPage, contentPositionVec);
   mergedPage->addChild(catalog, parentPositionVec);
   if(!description.skipOverlayPage)
      mergedPage->addChild(_root, originalPage1PositionVec);
   if(!description.skipBasePage)
      mergedPage->addChild(sourcePageToXObject, originalPage2PositionVec);
      // Annotation parent page should be changed, since we moved old page
      // to Xobject
   if( !description.skipBasePage )
   {
      for(size_t i = 0; i< annotations.size();i++)
      {
         _updateAnnotParentPage(annotations[i],mergedPage);
      }
   }
   for(size_t i = 0; i < annotsAndItPositions.size(); ++i)
   {
      mergedPage->addChild(annotsAndItPositions[i].first, annotsAndItPositions[i].second);
   }
   catalog->addChild(mergedPage, pagePosition);
   _root = mergedPage; 
}


