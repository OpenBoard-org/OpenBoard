#ifndef UBGRAPHICSAUDIOITEMDELEGATE_H
#define UBGRAPHICSAUDIOITEMDELEGATE_H

#include <QtGui>
#include <phonon/MediaObject>

#include "core/UB.h"
#include "UBGraphicsItemDelegate.h"


class QGraphicsItem;
class UBGraphicsAudioItem;


class UBGraphicsAudioItemDelegate : public UBGraphicsItemDelegate
{
    Q_OBJECT;

public:

    UBGraphicsAudioItemDelegate ( UBGraphicsAudioItem *pDelegated, QObject *parent = 0 );
    ~UBGraphicsAudioItemDelegate();

private slots:

    void togglePlayPause();
    void toggleMute();
    void remove ( bool canUndo );
    void updatePlayPauseState();

private:

    DelegateButton* mPlayPauseButton;
    DelegateButton* mStopButton;
    DelegateButton* mMuteButton;

    UBGraphicsAudioItem* mDelegated;

protected:

    virtual void buildButtons();

protected slots:

    void mediaStateChanged ( Phonon::State newstate, Phonon::State oldstate );


};

#endif // UBGRAPHICSAUDIOITEMDELEGATE_H
