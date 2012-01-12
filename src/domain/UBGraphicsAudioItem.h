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
#ifndef UBGRAPHICSAUDIOITEM_H
#define UBGRAPHICSAUDIOITEM_H

#include "UBGraphicsMediaItem.h"
#include "phonon/seekslider.h"
#include "phonon/mediasource.h"
#include "core/UB.h"
#include "core/UBApplication.h"
#include "board/UBBoardController.h"

class UBGraphicsAudioItem : public UBGraphicsMediaItem
{
    Q_OBJECT

public:

    UBGraphicsAudioItem(const QUrl& pAudioFileUrl, QGraphicsItem *parent = 0);
    ~UBGraphicsAudioItem();

    enum { Type = UBGraphicsItemType::AudioItemType };

    virtual int type() const
    {
        return Type;
    }

    virtual UBItem* deepCopy () const;
    virtual UBGraphicsItemDelegate *Delegate() const {return mDelegate;}

    virtual void clearSource()
    {
        UBGraphicsMediaItem::clearSource();
    }

private slots:

    void tick ( qint64 time );
    void onStateChanged(Phonon::State newState,Phonon::State oldState);

protected:

    QWidget* mAudioWidget;
    QLCDNumber* mTimeLcd;

    Phonon::SeekSlider* mSeekSlider;
private:
    Phonon::MediaSource mSource;

};

#endif // UBGRAPHICSAUDIOITEM_H
