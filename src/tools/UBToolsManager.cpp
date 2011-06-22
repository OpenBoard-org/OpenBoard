/*
 * UBToolsManager.cpp
 *
 *  Created on: 11 nov. 2009
 *      Author: Luc
 */

#include "UBToolsManager.h"

#include "core/memcheck.h"

UBToolsManager* UBToolsManager::sManager = 0;

UBToolsManager::UBToolsManager(QObject *parent)
    : QObject(parent)
{

    mask.id = "uniboardTool://uniboard.mnemis.com/mask";
    mask.icon = QPixmap(":/images/toolPalette/maskTool.png");
    mask.label = tr("Mask");
    mask.version = "1.0";
    mToolsIcon.insert(mask.id, ":/images/toolPalette/maskTool.png");
    mDescriptors << mask;

    ruler.id = "uniboardTool://uniboard.mnemis.com/ruler";
    ruler.icon = QPixmap(":/images/toolPalette/rulerTool.png");
    ruler.label = tr("Ruler");
    ruler.version = "1.0";
    mToolsIcon.insert(ruler.id, ":/images/toolPalette/rulerTool.png");
    mDescriptors << ruler;

    compass.id = "uniboardTool://uniboard.mnemis.com/compass";
    compass.icon = QPixmap(":/images/toolPalette/compassTool.png");
    compass.label = tr("Compass");
    compass.version = "1.0";
    mToolsIcon.insert(compass.id, ":/images/toolPalette/compassTool.png");
    mDescriptors << compass;

    protractor.id = "uniboardTool://uniboard.mnemis.com/protractor";
    protractor.icon = QPixmap(":/images/toolPalette/protractorTool.png");
    protractor.label = tr("Protractor");
    protractor.version = "1.0";
    mToolsIcon.insert(protractor.id,":/images/toolPalette/protractorTool.png");
    mDescriptors << protractor;

    triangle.id = "uniboardTool://uniboard.mnemis.com/triangle";
    triangle.icon = QPixmap(":/images/toolPalette/triangleTool.png");
    triangle.label = tr("Triangle");
    triangle.version = "1.0";
    mToolsIcon.insert(triangle.id,":/images/toolPalette/triangleTool.png");
    mDescriptors << triangle;

}

UBToolsManager::~UBToolsManager()
{
    // NOOP
}