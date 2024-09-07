#include "UBv4l2loopVideo.h"

#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

//-------------------------------------------------------------------------
// UBv4l2loopVideoEncoder
//-------------------------------------------------------------------------

UBv4l2loopVideoEncoder::UBv4l2loopVideoEncoder(QObject* parent)
    : UBAbstractVideoEncoder(parent)
    , format({0})
{
}

UBv4l2loopVideoEncoder::~UBv4l2loopVideoEncoder()
{
}

void UBv4l2loopVideoEncoder::setLastErrorMessage(const QString& pMessage)
{
    qWarning() << "v4l2-loopback video encoder:" << pMessage;
    mLastErrorMessage = pMessage;
}


/* send the v4l2_fmt struct to the kernel, and check it was accepted */
bool UBv4l2loopVideoEncoder::update_format(bool check){
    int rc;

	if((rc = ioctl(fd, VIDIOC_S_FMT, &format)) < 0){
		qWarning() << "v4l2 device setformat failed";
		return false;
	}

    if (!check)
        return true;

    struct v4l2_format read_format = format;

	if((rc = ioctl(fd, VIDIOC_G_FMT, &read_format)) < 0){
		qWarning() << "v4l2 device getformat failed";
		return false;
	}

	if(format.fmt.pix.pixelformat != read_format.fmt.pix.pixelformat){
		qWarning() << "v4l2 format not supported";
		return false;
	}

    return true;
}


bool UBv4l2loopVideoEncoder::start()
{
	int rc = 0;

    qWarning() << "called init";

	if((fd = open(getenv("V4L2_DEVICE"), O_RDWR)) < 0){
		qWarning() << "v4l2 device open fail";
		return false;
	}

    format.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	if((rc = ioctl(fd, VIDIOC_G_FMT, &format)) < 0){
		qWarning() << "v4l2 device init getformat fail";
		return false;
	}

    format.fmt.pix.pixelformat = pixelformat;
    return update_format(true);
}

bool UBv4l2loopVideoEncoder::stop()
{
    qDebug() << "Video encoder: stop requested";
    close(fd);
    fd = -1;
    emit encodingFinished(true);
    return true;
}

/**
 * This function should be called every time a new "screenshot" is ready.
 * The image is converted to the right format and sent to the encoder.
 */
void UBv4l2loopVideoEncoder::newPixmap(const QImage &image, long timestamp)
{
    int rc;
    std::size_t image_size = image.sizeInBytes();

    // update the frame size if it changed
    if (image.width() != format.fmt.pix.width || image.height() != format.fmt.pix.height) {
        assert(image_size == image.width() * image.height() * 4);
        format.fmt.pix.width = image.width();
        format.fmt.pix.height = image.height();
        format.fmt.pix.sizeimage = image_size;
        update_format();
    }

    const uchar *image_data = image.bits();
    if ((rc = write(fd, image_data, image_size)) < 0) {
        qWarning() << "write failed in v4l2 encoder";
    }
}

void UBv4l2loopVideoEncoder::onAudioAvailable(QByteArray data)
{
    (void)data;
}

void UBv4l2loopVideoEncoder::finishEncoding()
{
    qDebug() << "VideoEncoder::finishEncoding called";
}
