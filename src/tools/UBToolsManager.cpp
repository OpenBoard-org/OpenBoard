/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
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

//    cache.id = "uniboardTool://uniboard.mnemis.com/cache";
//    cache.icon = QPixmap(":/images/toolPalette/cacheTool.png");
//    cache.label = tr("Cache");
//    cache.version = "1.0";
//    mToolsIcon.insert(cache.id, ":/images/toolPalette/cacheTool.png");
//    mDescriptors << cache;
}

UBToolsManager::~UBToolsManager()
{
    // NOOP
}
