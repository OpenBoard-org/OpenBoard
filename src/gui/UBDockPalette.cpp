#include <QPoint>
#include <QPointF>
#include <QPainterPath>

#include "UBDockPalette.h"
#include "core/UBSettings.h"
#include "frameworks/UBPlatformUtils.h"

/**
 * \brief The constructor
 */
UBDockPalette::UBDockPalette(QWidget *parent, const char *name)
    :QWidget(parent, Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint)
    , mOrientation(eUBDockOrientation_Left)
    , mPreferredWidth(100)
    , mPreferredHeight(100)
    , mCanResize(false)
    , mResized(false)
    , mCollapseWidth(150)
    , mLastWidth(-1)
{
    setObjectName(name);

    // We let 2 pixels in order to keep a small border for the resizing
    setMinimumWidth(border() + 2);

    if (parent)
    {
	setAttribute(Qt::WA_NoMousePropagation);
	setAttribute(Qt::WA_TranslucentBackground);
    }
    else
    {
	// standalone window
	setAttribute(Qt::WA_TranslucentBackground);

	// !!!! Should be included into Windows after QT recompilation
#ifdef Q_WS_MAC
        setAttribute(Qt::WA_MacNoShadow);
#endif

    }

    mBackgroundBrush = QBrush(UBSettings::paletteColor);

    // This is the only way to set the background as transparent!
    setStyleSheet("QWidget {background-color: transparent}");
}

/**
 * \brief The destructor
 */
UBDockPalette::~UBDockPalette()
{

}

/**
 * \brief Get the current orientation
 * @return the current orientation
 */
eUBDockOrientation UBDockPalette::orientation()
{
    return mOrientation;
}

/**
 * \brief Set the current orientation
 * @param orientation as the given orientation
 */
void UBDockPalette::setOrientation(eUBDockOrientation orientation)
{
    // Set the size
    mOrientation = orientation;

    if(orientation == eUBDockOrientation_Left || orientation == eUBDockOrientation_Right)
    {
	setMaximumHeight(parentWidget()->height());
	setMinimumHeight(maximumHeight());
    }
    else if(orientation == eUBDockOrientation_Top || orientation == eUBDockOrientation_Bottom)
    {
	setMaximumWidth(parentWidget()->width());
	setMinimumWidth(maximumWidth());
    }

}

/**
 * \brief Handle the mouse move event
 * @param as the mouse event
 */
void UBDockPalette::mouseMoveEvent(QMouseEvent *event)
{
    QPoint p = event->pos();

    if(mCanResize && ((mMousePressPos - p).manhattanLength() > QApplication::startDragDistance()))
    {
	switch(mOrientation)
	{
	case eUBDockOrientation_Left:
            if(p.x() < collapseWidth() && p.x() >= minimumWidth())
            {
                resize(border(), height());
                mLastWidth = collapseWidth() + 1;
                mResized = true;
            }
            else if(p.x() <= maximumWidth() && p.x() >= minimumWidth())
	    {
		resize(p.x(), height());
                mResized = true;
	    }
	    break;
	case eUBDockOrientation_Right:
            if((this->x() + p.x() > parentWidget()->width() - collapseWidth()) && (this->x() + p.x() < parentWidget()->width()))
            {
                resize(border(), height());
                mLastWidth = collapseWidth() + 1;
                mResized = true;
            }
            else if((this->x() + p.x() >= parentWidget()->width() - maximumWidth()) && (this->x() + p.x() <= parentWidget()->width() - this->minimumWidth()))
	    {
		resize(parentWidget()->width() - (this->x() + p.x()), height());
                mResized = true;
	    }
	    break;
	case eUBDockOrientation_Top:
	case eUBDockOrientation_Bottom:
	    if(p.y() <= maximumHeight())
	    {
		resize(width(), p.y());
                mResized = true;
	    }
	    break;

	default:
	    break;
	}
    }
}

/**
 * \brief Handle the mouse press event
 * @param as the mouse event
 */
void UBDockPalette::mousePressEvent(QMouseEvent *event)
{
    mClickTime = QTime::currentTime();
    // The goal here is to verify if the user can resize the widget.
    // It is only possible to resize it if the border is selected
    QPoint p = event->pos();
    mMousePressPos = p;
    mResized = false;

    switch(mOrientation)
    {
    case eUBDockOrientation_Left:
	if((p.x() >= width() - border()) && (p.x() <= width()))
	{
	    mCanResize = true;
	}
	break;
    case eUBDockOrientation_Right:
	if((p.x() >= 0) && (p.x() <= border()))
	{
	    mCanResize = true;
	}
	break;
    case eUBDockOrientation_Top:
	// Not supported yet
	break;
    case eUBDockOrientation_Bottom:
	// Not supported yet
	break;
    default:
	break;
    }
}

/**
 * \brief Handle the mouse release event
 * @param event as the mouse event
 */
void UBDockPalette::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    if(!mResized && mClickTime.elapsed() < CLICKTIME)
    {
        // If the clicked position is in the tab, perform the related action
        if(eUBDockOrientation_Left == mOrientation)
        {
            if(mMousePressPos.x() >= width() - 2*border() &&
               mMousePressPos.y() >= border() &&
               mMousePressPos.x() <= width() &&
               mMousePressPos.y() <= border() + TABSIZE)
            {
                tabClicked();
            }
        }
        else if(eUBDockOrientation_Right == mOrientation)
        {
            if(mMousePressPos.x() >= 0 &&
               mMousePressPos.x() <= 2*border() &&
               mMousePressPos.y() >= border() &&
               mMousePressPos.y() <= border() + TABSIZE)
            {
                tabClicked();
            }
        }
    }

    mCanResize = false;
}

void UBDockPalette::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    updateMaxWidth();
    if(parentWidget())
    {
        setMinimumHeight(parentWidget()->height());
    }
	// Set the position
    QPoint origin;
    switch(mOrientation)
    {
    case eUBDockOrientation_Right:
		origin.setX(parentWidget()->width() - this->width());
		origin.setY(0);
	break;
    case eUBDockOrientation_Bottom:
	// Not supported yet
    case eUBDockOrientation_Top:
	// Not supported yet
    case eUBDockOrientation_Left:
    default:
	origin.setX(0);
	origin.setY(0);
	break;
    }
    move(origin.x(), origin.y());
}

/**
 * \brief Handle the mouse enter event
 * @param event as the mouse event
 */
void UBDockPalette::enterEvent(QEvent *event)
{
    Q_UNUSED(event);
    // We want to set the cursor as an arrow everytime it enters the palette
    setCursor(Qt::ArrowCursor);
}

/**
 * \brief Handle the mouse leave event
 * @param event as the mouse event
 */
void UBDockPalette::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    // Restore the cursor to its previous shape
    unsetCursor();
}

/**
 * \brief Draw the palette
 * @param event as the paint event
 */
void UBDockPalette::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(mBackgroundBrush);

    if(mOrientation == eUBDockOrientation_Left)
    {
	QPainterPath path;
	path.setFillRule(Qt::WindingFill);
	path.addRect(0.0, 0.0, width()-border(), height());
        path.addRoundedRect(width()-2*border(), border(), 2*border(), TABSIZE, radius(), radius());
	painter.drawPath(path);
        painter.drawPixmap(width() - border() + 1, border() + 1 , border() - 4, TABSIZE - 2, mIcon);
    }
    else if(mOrientation == eUBDockOrientation_Right)
    {
	QPainterPath path;
	path.setFillRule(Qt::WindingFill);
	path.addRect(border(), 0.0, width()-border(), height());
	path.addRoundedRect(0.0, border(), 2*border(), TABSIZE, radius(), radius());
	painter.drawPath(path);
        painter.drawPixmap(2, border() + 1, border() - 3, TABSIZE - 2, mIcon);
    }
    else
    {
	painter.drawRoundedRect(border(), border(), width() - 2 * border(), height() - 2 * border(), radius(), radius());
    }
}

/**
 * \brief Set the background brush
 * @param brush as the given brush
 */
void UBDockPalette::setBackgroundBrush(const QBrush &brush)
{
    if (mBackgroundBrush != brush)
    {
	mBackgroundBrush = brush;
	update();
    }
}

/**
 * Get the border size
 * @return the border size
 */
int UBDockPalette::border()
{
    return 15;
}

/**
 * \brief Get the radius size
 * @return the radius size
 */
int UBDockPalette::radius()
{
    return 5;
}

/**
 * \brief Set the maximum width
 */
void UBDockPalette::updateMaxWidth()
{
    // Only the inherited class will overload this method
}

/**
 * \brief Get the collapse width value
 * @return the collapse widht value
 */
int UBDockPalette::collapseWidth()
{
    return mCollapseWidth;
}

/**
 * \brief collapse/expand automatically the palette after a click on its tab
 */
void UBDockPalette::tabClicked()
{
    if(mLastWidth == -1)
    {
        // The palette must be collapsed
        mLastWidth = width();
        resize(border(), height());
    }
    else
    {
        // The palette will be expanded
        resize(mLastWidth, height());
        mLastWidth = -1;
    }
}
