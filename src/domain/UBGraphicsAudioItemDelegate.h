/*
 * Copyright (C) 2012 Webdoc SA
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open-Sankoré.  If not, see <http://www.gnu.org/licenses/>.
 */



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
