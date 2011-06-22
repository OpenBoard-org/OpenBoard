
#include "UBIdleTimer.h"

#include <QApplication>
#include <QInputEvent>
#include <QTimer>
#include <QWidget>

#include "UBApplication.h"
#include "UBApplicationController.h"
#include "board/UBBoardController.h"
#include "board/UBBoardView.h"

#include "core/memcheck.h"

UBIdleTimer::UBIdleTimer(QObject *parent)
     : QObject(parent)
     , mCursorIsHidden(false)
{
    startTimer(100);
}

UBIdleTimer::~UBIdleTimer()
{
    // NOOP
}

bool UBIdleTimer::eventFilter(QObject *obj, QEvent *event)
{
    // if the event is an input event (mouse / tablet / keyboard)
    if (dynamic_cast<QInputEvent*>(event))
    {
        mLastInputEventTime = QDateTime::currentDateTime();
        if (mCursorIsHidden)
        {
            QApplication::restoreOverrideCursor();
            mCursorIsHidden = false;
        }
    }

    return QObject::eventFilter(obj, event);
}

void UBIdleTimer::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);

        bool hasWaitCuror = (UBApplication::overrideCursor() &&
                (UBApplication::overrideCursor()->shape() == Qt::WaitCursor));

    if (!mCursorIsHidden
            && (mLastInputEventTime.secsTo(QDateTime::currentDateTime()) >= 2)
                        && UBApplication::boardController
                        && UBApplication::boardController->controlView()
                        && UBApplication::boardController->controlView()->hasFocus()
                        && !hasWaitCuror)
    {
        QApplication::setOverrideCursor(QCursor(Qt::BlankCursor));
        mCursorIsHidden = true;
    }
}
