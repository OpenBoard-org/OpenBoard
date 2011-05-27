/*
 * UBToolsManager.cpp
 *
 *  Created on: 11 nov. 2009
 *      Author: Luc
 */

#include "UBToolsManager.h"

UBToolsManager* UBToolsManager::sManager = 0;

UBToolsManager::UBToolsManager(QObject *parent)
    : QObject(parent)
{

    mask.id = "uniboardTool://uniboard.mnemis.com/mask";
    mask.icon = QPixmap(":/images/toolPalette/maskTool.png");
    mask.label = tr("Mask");
    mask.version = "1.0";
    mToolsIcon.insert(mask.id, mask.icon);
    mDescriptors << mask;

    ruler.id = "uniboardTool://uniboard.mnemis.com/ruler";
    ruler.icon = QPixmap(":/images/toolPalette/rulerTool.png");
    ruler.label = tr("Ruler");
    ruler.version = "1.0";
    mToolsIcon.insert(ruler.id, ruler.icon);
    mDescriptors << ruler;

    compass.id = "uniboardTool://uniboard.mnemis.com/compass";
    compass.icon = QPixmap(":/images/toolPalette/compassTool.png");
    compass.label = tr("Compass");
    compass.version = "1.0";
    mToolsIcon.insert(compass.id, compass.icon);
    mDescriptors << compass;

    protractor.id = "uniboardTool://uniboard.mnemis.com/protractor";
    protractor.icon = QPixmap(":/images/toolPalette/protractorTool.png");
    protractor.label = tr("Protractor");
    protractor.version = "1.0";
    mToolsIcon.insert(protractor.id, protractor.icon);
    mDescriptors << protractor;

}

UBToolsManager::~UBToolsManager()
{
    // NOOP
}