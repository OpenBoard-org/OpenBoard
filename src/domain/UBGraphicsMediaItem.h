#ifndef UBGRAPHICSMEDIAITEM_H
#define UBGRAPHICSMEDIAITEM_H

#include "UBGraphicsProxyWidget.h"
#include <phonon/AudioOutput>
#include <phonon/MediaObject>


class UBGraphicsMediaItem : public UBGraphicsProxyWidget
{
    Q_OBJECT

public:

    UBGraphicsMediaItem(const QUrl& pMediaFileUrl, QGraphicsItem *parent = 0);
    ~UBGraphicsMediaItem();

    void hasMediaChanged(bool hasMedia);
    void showOnDisplayChanged(bool shown);

    virtual QUrl mediaFileUrl() const
    {
        return mMediaFileUrl;
    }

    Phonon::MediaObject* mediaObject() const
    {
        return mMediaObject;
    }

    void setInitialPos(qint64 p) {
        mInitialPos = p;
    }
    qint64 initialPos() {
        return mInitialPos;
    }

    bool isMuted() const
    {
        return mMuted;
    }

    virtual UBGraphicsScene* scene();

public slots:

    void toggleMute();
    void activeSceneChanged();

protected:

    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

    Phonon::MediaObject *mMediaObject;
    Phonon::AudioOutput *mAudioOutput;

private:

    bool mMuted;
    bool mMutedByUserAction;
    static bool sIsMutedByDefault;

    QUrl mMediaFileUrl;
    QString mMediaSource;

    qint64 mInitialPos;

};

#endif // UBGRAPHICSMEDIAITEM_H
