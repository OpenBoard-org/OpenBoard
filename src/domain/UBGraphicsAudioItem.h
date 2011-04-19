#ifndef UBGRAPHICSAUDIOITEM_H
#define UBGRAPHICSAUDIOITEM_H

#include "UBGraphicsMediaItem.h"
#include "phonon/seekslider.h"
#include "phonon/mediasource.h"
#include "core/UB.h"

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
