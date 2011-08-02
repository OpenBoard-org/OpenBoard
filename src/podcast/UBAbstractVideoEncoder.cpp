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

#include "UBAbstractVideoEncoder.h"

UBAbstractVideoEncoder::UBAbstractVideoEncoder(QObject *pParent)
    : QObject(pParent)
    , mFramesPerSecond(10)
    , mVideoSize(640, 480)
    , mVideoBitsPerSecond(1700000) // 1.7 Mbps
{
    // NOOP

}

UBAbstractVideoEncoder::~UBAbstractVideoEncoder()
{
    // NOOP
}


void UBAbstractVideoEncoder::newChapter(const QString& pLabel, long timestamp)
{
    Q_UNUSED(pLabel);
    Q_UNUSED(timestamp);
}
