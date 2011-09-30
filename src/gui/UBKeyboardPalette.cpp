/*
 * UBKeyboardPalette.cpp
 *
 *  Created on: Feb 10, 2011
 *      Author: tolik
 */

#include <QtGui>
#include <QList>
#include <QSize>

#include "UBKeyboardPalette.h"
#include "core/UBSettings.h"

#include "core/UBApplication.h"
#include "gui/UBMainWindow.h"

#include "core/memcheck.h"

/*

            UBKeyboardPalette

*/


UBKeyboardPalette::UBKeyboardPalette(QWidget *parent)
        : UBActionPalette(Qt::TopRightCorner, parent)
{
    setCustomCloseProcessing(true);
    setCustomPosition(true);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    setFocusPolicy(Qt::NoFocus);
    setClosable(true);
    setGrip(false);

    capsLock = false;
    languagePopupActive = false;
    keyboardActive = false;
    btnWidth = btnHeight = 16;
    strSize = "16x16";
    currBtnImages = new BTNImages("16", btnWidth, btnHeight);

    buttons = new UBKeyButton*[47];
    for (int i=0; i<47; i++)
    {
        buttons[i] = new UBKeyButton(this);
    }

    locales = UBPlatformUtils::getKeyboardLayouts(this->nLocalesCount);

    createCtrlButtons();

    nCurrentLocale = 0;
    setInput(locales[nCurrentLocale]);

    setContentsMargins( 22, 22, 22, 22 );

    connect(this, SIGNAL(keyboardActivated(bool)), this, SLOT(onActivated(bool)));
}

QList<UBKeyboardPalette*> UBKeyboardPalette::instances;
UBKeyboardPalette* UBKeyboardPalette::create(QWidget *parent)
{
    //------------------------------//

    if (!UBPlatformUtils::hasVirtualKeyboard())
        return NULL;

    //------------------------------//

    UBKeyboardPalette* firstKeyboard = NULL;
    // if we already have keyboards inside, get it position and show/hide status, for new keyboard
    if(instances.size() > 0)
        firstKeyboard = instances.at(0);

    //------------------------------//

    UBKeyboardPalette* instance = new UBKeyboardPalette(parent);
    instance->setKeyButtonSize(UBSettings::settings()->boardKeyboardPaletteKeyBtnSize->get().toString());

    instance->m_isVisible = firstKeyboard ? firstKeyboard->m_isVisible : false;
    instance->setVisible(instance->m_isVisible);

    if( firstKeyboard )
        instance->move(firstKeyboard->m_pos);

    connect(UBSettings::settings()->boardKeyboardPaletteKeyBtnSize, SIGNAL(changed(QVariant)), instance, SLOT(keyboardPaletteButtonSizeChanged(QVariant)));
    connect(UBApplication::mainWindow->actionVirtualKeyboard, SIGNAL(triggered(bool)), instance, SLOT(showKeyboard(bool)));
//    connect(instance, SIGNAL(moved(const QPoint&)), instance, SLOT(syncPosition(const QPoint&)));
    connect(instance, SIGNAL(closed()), instance, SLOT(hideKeyboard()));

    //------------------------------//

    instances.append(instance);
    foreach(UBKeyboardPalette* inst, instances)
    {
        connect(inst, SIGNAL(moved(const QPoint&)), instance, SLOT(syncPosition(const QPoint&)));
        connect(instance, SIGNAL(moved(const QPoint&)), inst, SLOT(syncPosition(const QPoint&)));

        connect(inst, SIGNAL(localeChanged(int)), instance, SLOT(syncLocale(int)));
        connect(instance, SIGNAL(localeChanged(int)), inst, SLOT(syncLocale(int)));

//        connect(instance, SIGNAL(closed()), inst, )
    }

    //------------------------------//

    return instance;
}

void UBKeyboardPalette::showKeyboard(bool show)
{
    m_isVisible = show;
}


void UBKeyboardPalette::hideKeyboard()
{
    UBApplication::mainWindow->actionVirtualKeyboard->activate(QAction::Trigger);
}

void UBKeyboardPalette::syncPosition(const QPoint & pos)
{
    m_pos = pos;
    move(pos);
}

void UBKeyboardPalette::syncLocale(int nLocale)
{
    nCurrentLocale = nLocale;
    setInput(locales[nCurrentLocale]);
}

void UBKeyboardPalette::keyboardPaletteButtonSizeChanged(QVariant size)
{
    setKeyButtonSize(size.toString());
}

void UBKeyboardPalette::setInput(const UBKeyboardLocale* locale)
{
    if (locale!=NULL)
    {
        for (int i=0; i<47; i++)
            buttons[i]->setKeyBt((*locale)[i]);
    }
    else
    {
        this->hide();
    }
}

UBKeyboardPalette::~UBKeyboardPalette()
{
    //for (int i=0; i<47; i++)
    //    delete buttons[i];
    delete [] buttons;

    //for (int i=0; i<8; i++)
    //    delete ctrlButtons[i];
    delete [] ctrlButtons;

    //if (locales!=NULL)
    //{
    //    for (int i=0; i<nLocalesCount; i++)
    //        delete locales[i];
    //    delete [] locales;
    //}

    if(currBtnImages != NULL)
    {
        delete currBtnImages;
        currBtnImages = NULL;
    }
}

QSize  UBKeyboardPalette::sizeHint () const
{
    int w = contentsMargins().left() + 15 * btnWidth + contentsMargins().right();
    int h = contentsMargins().top() + 5 * btnHeight + contentsMargins().bottom();
    return QSize(w, h);
}

const QString* UBKeyboardPalette::getLocaleName()
{
    return locales == NULL ? NULL : &(locales[nCurrentLocale]->name);
}

void UBKeyboardPalette::setLocale(int nLocale)
{
    if (locales != NULL)
    {
        nCurrentLocale = nLocale;

        setInput(locales[nCurrentLocale]);
        onLocaleChanged(locales[nCurrentLocale]);
        update();
    }
    emit localeChanged(nLocale);
}

void UBKeyboardPalette::setKeyButtonSize(const QString& _strSize)
{
    QStringList strs = _strSize.split('x');

    if (strs.size()==2)
    {
        strSize = _strSize;
        btnWidth = strs[0].toInt();
        btnHeight = strs[1].toInt();

        if(currBtnImages != NULL)
            delete currBtnImages;
        currBtnImages = new BTNImages(strs[1], btnWidth, btnHeight);

        adjustSizeAndPosition();
    }
}

void UBKeyboardPalette::enterEvent ( QEvent * )
{
    if (keyboardActive)
        return;

    keyboardActive = true;

    adjustSizeAndPosition();

    emit keyboardActivated(true);
}

void UBKeyboardPalette::leaveEvent ( QEvent * )
{
    if (languagePopupActive || !keyboardActive || mIsMoving)
        return;

    keyboardActive = false;

    adjustSizeAndPosition();

    emit keyboardActivated(false);
}

void  UBKeyboardPalette::moveEvent ( QMoveEvent * event )
{
    UBActionPalette::moveEvent(event);
    emit moved(event->pos());
}

void UBKeyboardPalette::adjustSizeAndPosition(bool pUp)
{
    QSize rSize = sizeHint();
    if (rSize != size())
    {
        int dx = (rSize.width() - size().width()) /2;
        int dy = rSize.height() - size().height();

        this->move(x()-dx, y() - dy);
        this->resize(rSize.width(), rSize.height());
    }
    UBActionPalette::adjustSizeAndPosition(pUp);
}

void  UBKeyboardPalette::paintEvent( QPaintEvent* event)
{
    UBActionPalette::paintEvent(event);

    QRect r = this->geometry();

    int lleft, ltop, lright, lbottom;
    getContentsMargins ( &lleft, &ltop, &lright, &lbottom ) ;

    //------------------------------------------------
    // calculate start offset from left, and from top

    int ctrlButtonsId = 0;
    lleft = ( r.width() - btnWidth * 15 ) / 2;
    ltop = ( r.height() - btnHeight * 5 ) / 2;

    //------------------------------------------------
    // set geometry (position) for row 1

    int offX = lleft;
    int offY = ltop;

    //-------------------

    // buttons [`]..[+]
    for (int i = 0; i<13; i++)
    {
        buttons[i]->setGeometry(offX, offY, btnWidth, btnHeight);
        offX += btnWidth;
    }

    // button Backspace
    ctrlButtons[ctrlButtonsId++]->setGeometry(offX, offY, btnWidth * 2, btnHeight);
    offX += btnWidth * 2;

    //------------------------------------------------
    // set geometry (position) for row 2

    offX = lleft;
    offY += btnHeight;
    offX += btnWidth / 2;

    //-------------------

    // button Tab
    ctrlButtons[ctrlButtonsId++]->setGeometry(offX, offY, btnWidth * 2, btnHeight);
    offX += btnWidth * 2;

    // buttons [q]..[]]
    for (int i = 0; i<12; i++)
    {
        buttons[i + 13]->setGeometry(offX, offY, btnWidth, btnHeight);
        offX += btnWidth;
    }

//     // Row 2 Stub
//     ctrlButtons[ctrlButtonsId++]->setGeometry(offX, offY, btnWidth * 1.5, btnHeight);
//     offX += btnWidth * 1.5;

    //------------------------------------------------
    // set geometry (position) for row 3

    offX = lleft;
    offY += btnHeight;

    //-------------------

//     // Row 3 Stub
//     ctrlButtons[ctrlButtonsId++]->setGeometry(offX, offY, btnWidth, btnHeight);
//    offX += btnWidth;

    // buttons [a]..[\]
    for (int i = 0; i < 12; i++)
    {
        buttons[i + 12 + 13]->setGeometry(offX, offY, btnWidth, btnHeight);
        offX += btnWidth;
    }

    // button Enter
    ctrlButtons[ctrlButtonsId++]->setGeometry(offX, offY, btnWidth * 3, btnHeight);
    offX += btnWidth*3;

    //------------------------------------------------
    // set geometry (position) for row 4

    offX = lleft;
    offY += btnHeight;

    //-------------------

    // button LCapsLock
    ctrlButtons[ctrlButtonsId++]->setGeometry(offX, offY, btnWidth*2.5, btnHeight);
    offX += btnWidth*2.5;

    for (int i = 0; i < 10; i++)
    {
        buttons[i + 12 + 12 + 13]->setGeometry(offX, offY, btnWidth, btnHeight);
        offX += btnWidth;
    }

    // button RCapsLock
    ctrlButtons[ctrlButtonsId++]->setGeometry(offX, offY, btnWidth*2.5, btnHeight);
    offX += btnWidth*2.5;

    //------------------------------------------------
    // set geometry (position) for row 5

    offX = lleft;
    offY += btnHeight;

    //-------------------

    ctrlButtons[ctrlButtonsId++]->setGeometry(offX + btnWidth * 1 , offY, btnWidth * 2, btnHeight);
    ctrlButtons[ctrlButtonsId++]->setGeometry(offX + btnWidth * 3 , offY, btnWidth * 9, btnHeight);
    ctrlButtons[ctrlButtonsId++]->setGeometry(offX + btnWidth * 12, offY, btnWidth * 2, btnHeight);

    //------------------------------------------------
}


//-----------------------------------------------------------------------//
// BTNImages Class
//-----------------------------------------------------------------------//

BTNImages::BTNImages(QString strHeight, int width, int height)
{
    m_strHeight = strHeight;
    m_width = width;
    m_height = height;

    m_strLeftPassive    = ":/images/virtual.keyboard/" + strHeight + "/left-passive.png";
    m_strCenterPassive  = ":/images/virtual.keyboard/" + strHeight + "/centre-passive.png";
    m_strRightPassive   = ":/images/virtual.keyboard/" + strHeight + "/right-passive.png";
    m_strLeftActive     = ":/images/virtual.keyboard/" + strHeight + "/left-active.png";
    m_strCenterActive   = ":/images/virtual.keyboard/" + strHeight + "/centre-active.png";
    m_strRightActive    = ":/images/virtual.keyboard/" + strHeight + "/right-active.png";

    m_btnLeftPassive    = QImage(m_strLeftPassive);
    m_btnCenterPassive  = QImage(m_strCenterPassive);
    m_btnRightPassive   = QImage(m_strRightPassive);
    m_btnLeftActive     = QImage(m_strLeftActive);
    m_btnCenterActive   = QImage(m_strCenterActive);
    m_btnRightActive    = QImage(m_strRightActive);
}

ContentImage::ContentImage(QString strHeight, int height, QString strContentName)
{
    m_strHeight = strHeight;
    m_height = height;

    m_strContent = ":/images/virtual.keyboard/" + strHeight + "/" + strContentName + ".png";
    m_btnContent = QImage(m_strContent);
}

//-----------------------------------------------------------------------//
// UBKeyboardButton Class
//-----------------------------------------------------------------------//

UBKeyboardButton::UBKeyboardButton(UBKeyboardPalette* parent, QString contentImagePath = "")
        :QWidget(parent),
        keyboard(parent),
        bFocused(false),
        bPressed(false)
{
    m_parent = parent;

    m_contentImagePath = contentImagePath;
    imgContent = NULL;

    setCursor(Qt::PointingHandCursor);
}

UBKeyboardButton::~UBKeyboardButton()
{
    if(imgContent != NULL)
    {
        delete imgContent;
        imgContent = NULL;
    }
}

void UBKeyboardButton::paintEvent(QPaintEvent*)
{

    QPainter painter(this);

    //--------------------------

    if(imgContent != NULL)
    {
        if(imgContent->m_height != m_parent->currBtnImages->m_height)
        {
            delete imgContent;
            if(!m_contentImagePath.isEmpty())
                imgContent = new ContentImage(m_parent->currBtnImages->m_strHeight, m_parent->currBtnImages->m_height, m_contentImagePath);
        }
    }
    else
    if(!m_contentImagePath.isEmpty())
        imgContent = new ContentImage(m_parent->currBtnImages->m_strHeight, m_parent->currBtnImages->m_height, m_contentImagePath);

    //--------------------------

    if (bPressed)
    {
        painter.drawImage( 0,0, m_parent->currBtnImages->m_btnLeftActive, 0,0, m_parent->currBtnImages->m_btnLeftActive.width(), m_parent->currBtnImages->m_btnLeftActive.height() );
        painter.drawImage( QRect(m_parent->currBtnImages->m_btnLeftActive.width(), 0, width() - m_parent->currBtnImages->m_btnLeftActive.width() - m_parent->currBtnImages->m_btnRightActive.width(), height()), m_parent->currBtnImages->m_btnCenterActive );
        painter.drawImage( width() - m_parent->currBtnImages->m_btnRightActive.width(), 0, m_parent->currBtnImages->m_btnRightActive, 0,0, m_parent->currBtnImages->m_btnRightActive.width(), m_parent->currBtnImages->m_btnRightActive.height() );
    }
    else
    {
        painter.drawImage( 0,0, m_parent->currBtnImages->m_btnLeftPassive, 0,0, m_parent->currBtnImages->m_btnLeftPassive.width(), m_parent->currBtnImages->m_btnLeftPassive.height() );
        painter.drawImage( QRect(m_parent->currBtnImages->m_btnLeftPassive.width(), 0, width() - m_parent->currBtnImages->m_btnLeftPassive.width() - m_parent->currBtnImages->m_btnRightPassive.width(), height()), m_parent->currBtnImages->m_btnCenterPassive );
        painter.drawImage( width() - m_parent->currBtnImages->m_btnRightPassive.width(), 0, m_parent->currBtnImages->m_btnRightPassive, 0,0, m_parent->currBtnImages->m_btnRightPassive.width(), m_parent->currBtnImages->m_btnRightPassive.height() );
    }

    //--------------------------

    this->paintContent(painter);

    //--------------------------
}

void  UBKeyboardButton::enterEvent ( QEvent*)
{
    bFocused = true;
    update();
}

void  UBKeyboardButton::leaveEvent ( QEvent*)
{
    bFocused = false;
    update();
}

void  UBKeyboardButton::mousePressEvent ( QMouseEvent * event)
{
    event->accept();
    bPressed = true;
    update();
    this->onPress();
}

void  UBKeyboardButton::mouseReleaseEvent ( QMouseEvent * )
{
    bPressed = false;
    update();
    this->onRelease();
}

UBKeyButton::UBKeyButton(UBKeyboardPalette* parent)
        :UBKeyboardButton(parent),
        keybt(0)
{}

UBKeyButton::~UBKeyButton()
{}

void UBKeyButton::onPress()
{
    if (keybt!=NULL)
        sendUnicodeSymbol(keybt->code1, keybt->code2, capsLock());
}

void UBKeyButton::onRelease()
{}

void UBKeyButton::paintContent(QPainter& painter)
{
    if (keybt)
    {
        QString text(QChar(capsLock() ? keybt->symbol2 : keybt->symbol1));
        QRect textRect(rect().x()+2, rect().y()+2, rect().width()-4, rect().height()-4);
        painter.drawText(textRect, Qt::AlignCenter, text);
    }
}

UBCntrlButton::UBCntrlButton(UBKeyboardPalette* parent, int _code, const QString& _contentImagePath )
        :UBKeyboardButton(parent, _contentImagePath),
        label(""),
        code(_code)
{}


UBCntrlButton::UBCntrlButton(UBKeyboardPalette* parent, const QString& _label, int _code )
        :UBKeyboardButton(parent),
        label(_label),
        code(_code)
{}

UBCntrlButton::~UBCntrlButton()
{}

void UBCntrlButton::onPress()
{
     sendControlSymbol(code);
}

void UBCntrlButton::onRelease()
{}

void UBCntrlButton::paintContent(QPainter& painter)
{
    if(!label.isEmpty())
    {
        painter.drawText(rect(), Qt::AlignCenter, label);
    }
    else
    if(imgContent != NULL)
    {
        painter.drawImage(( rect().width() - imgContent->m_btnContent.width() ) / 2, ( rect().height() - imgContent->m_btnContent.height() ) / 2,
            imgContent->m_btnContent, 0,0, imgContent->m_btnContent.width(), imgContent->m_btnContent.height());
    }
}

UBCapsLockButton::UBCapsLockButton(UBKeyboardPalette* parent, const QString _contentImagePath)
        :UBKeyboardButton(parent, _contentImagePath)
{}

UBCapsLockButton::~UBCapsLockButton()
{}

void UBCapsLockButton::onPress()
{
    keyboard->capsLock = !keyboard->capsLock;
    keyboard->update();
}

void UBCapsLockButton::onRelease()
{}

void UBCapsLockButton::paintContent(QPainter& painter)
{
    if(imgContent != NULL)
    {
        painter.drawImage(( rect().width() - imgContent->m_btnContent.width() ) / 2, ( rect().height() - imgContent->m_btnContent.height() ) / 2,
            imgContent->m_btnContent, 0,0, imgContent->m_btnContent.width(), imgContent->m_btnContent.height());
    }
    else
        painter.drawText(rect(), Qt::AlignCenter, "^");
}


UBLocaleButton::UBLocaleButton(UBKeyboardPalette* parent)
        :UBKeyboardButton(parent)
{
    localeMenu = new QMenu(this);

    for (int i=0; i<parent->nLocalesCount; i++)
    {
        QAction* action = (parent->locales[i]->icon!=NULL) ?
                          localeMenu->addAction(*parent->locales[i]->icon, parent->locales[i]->fullName)
                          : localeMenu->addAction(parent->locales[i]->fullName);
        action->setData(QVariant(i));
    }
}

UBLocaleButton::~UBLocaleButton()
{
    delete localeMenu;
}

void UBLocaleButton::onPress()
{
}

void UBLocaleButton::onRelease()
{
    keyboard->languagePopupActive = true;
    QAction* action = localeMenu->exec(mapToGlobal(QPoint(0,0)));
    keyboard->languagePopupActive = false;
    if (action!=NULL)
    {
        int nLocale = action->data().toInt();
        keyboard->setLocale(nLocale);
    }
}

void UBLocaleButton::paintContent(QPainter& painter)
{
    const QString* localeName = keyboard->getLocaleName();
    if (localeName!=NULL)
        painter.drawText(rect(), Qt::AlignCenter, *localeName);
}
