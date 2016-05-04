#ifndef UBMICROPHONEINPUT_H
#define UBMICROPHONEINPUT_H

#include <QtCore>
#include <QAudioInput>

/**
 * @brief The UBMicrophoneInput class captures uncompressed sound from a microphone
 */
class UBMicrophoneInput : public QObject
{
    Q_OBJECT

public:
    UBMicrophoneInput();
    virtual ~UBMicrophoneInput();

    bool init();
    void start();
    void stop();

    static QStringList availableDevicesNames();
    void setInputDevice(QString name = "");

    int channelCount();
    int sampleRate();
    int sampleSize();
    int sampleFormat();
    QString codec();

    qint64 processUSecs() const;

signals:
    /// Send the new volume, between 0 and 255
    void audioLevelChanged(quint8 level);

    /// Emitted when new audio data is available
    void dataAvailable(QByteArray data);

    void error(QString message);

private slots:
    void onAudioInputStateChanged(QAudio::State state);
    void onDataReady();

private:
    QString getErrorString(QAudio::Error errorCode);

    QAudioInput* mAudioInput;
    QIODevice * mIODevice;
    QAudioDeviceInfo mAudioDeviceInfo;
    QAudioFormat mAudioFormat;

    qint64 mSeekPos;
};

#endif // UBMICROPHONEINPUT_H
