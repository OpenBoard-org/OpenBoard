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

/*

			UBKeyboardPalette

*/
UBKeyboardPalette::UBKeyboardPalette(QWidget *parent)
	: UBFloatingPalette(Qt::TopRightCorner, parent)
{
	setCustomPosition(true);
	setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	setFocusPolicy(Qt::NoFocus);
	capsLock = false;
	languagePopupActive = false;
	keyboardActive = false;
	btnWidth = btnHeight = 16;

	buttons = new UBKeyButton*[47];
	for(int i=0; i<47; i++)
	{
		buttons[i] = new UBKeyButton(this);
	}

	locales = UBPlatformUtils::getKeyboardLayouts(this->nLocalesCount);

	createCtrlButtons();

	nCurrentLocale = 0;
	setInput(locales[nCurrentLocale]);

    setContentsMargins(radius() + 15, 4, radius() + 15, 4);

	connect(this, SIGNAL(keyboardActivated(bool)), this, SLOT(onActivated(bool)));
}

QList<UBKeyboardPalette*> UBKeyboardPalette::instances;
UBKeyboardPalette* UBKeyboardPalette::create(QWidget *parent)
{
	UBKeyboardPalette* instance = new UBKeyboardPalette(parent);
	instances.append(instance);

	instance->setKeyButtonSize(UBSettings::settings()->boardKeyboardPaletteKeyBtnSize->get().toString());
	instance->setAutoMinimize(UBSettings::settings()->boardKeyboardPaletteAutoMinimize->get().toBool());
	instance->setVisible(false);
	connect(UBSettings::settings()->boardKeyboardPaletteAutoMinimize , SIGNAL(changed(QVariant)), instance, SLOT(keyboardPaletteAutoMinimizeChanged(QVariant)));
	connect(UBSettings::settings()->boardKeyboardPaletteKeyBtnSize, SIGNAL(changed(QVariant)), instance, SLOT(keyboardPaletteButtonSizeChanged(QVariant)));


	foreach(UBKeyboardPalette* inst, instances)
	{
		connect(inst, SIGNAL(moved(const QPoint&)), instance, SLOT(syncPosition(const QPoint&)));
		connect(instance, SIGNAL(moved(const QPoint&)), inst, SLOT(syncPosition(const QPoint&)));

		connect(inst, SIGNAL(localeChanged(int)), instance, SLOT(syncLocale(int)));
		connect(instance, SIGNAL(localeChanged(int)), inst, SLOT(syncLocale(int)));

	}

	return instance;
}

void UBKeyboardPalette::syncPosition(const QPoint & pos)
{
	move(pos);
}

void UBKeyboardPalette::syncLocale(int nLocale)
{
		nCurrentLocale = nLocale;
		setInput(locales[nCurrentLocale]);
}

void UBKeyboardPalette::keyboardPaletteAutoMinimizeChanged(QVariant b)
{
	setAutoMinimize(b.toBool());
}
void UBKeyboardPalette::keyboardPaletteButtonSizeChanged(QVariant size)
{
	setKeyButtonSize(size.toString());
}

void UBKeyboardPalette::setInput(const UBKeyboardLocale* locale)
{
	if (locale!=NULL)
	{
		for(int i=0; i<47; i++)
			buttons[i]->setKeyBt((*locale)[i]);
	}
	else
	{
		this->hide();
	}
}

UBKeyboardPalette::~UBKeyboardPalette()
{
	for(int i=0; i<47; i++)
		delete buttons[i];
	delete [] buttons;

	for(int i=0; i<8; i++)
		delete ctrlButtons[i];
	delete [] ctrlButtons;

	if (locales!=NULL)
	{
		for(int i=0; i<nLocalesCount; i++)
			delete locales[i];
		delete [] locales;
	}
}

QSize  UBKeyboardPalette::sizeHint () const
{
	if ((autoMinimize && keyboardActive) || !autoMinimize)
	{
		int w = contentsMargins().left() + contentsMargins().right() + 13 + 14.5 * btnWidth;
		int h = contentsMargins().top() + contentsMargins().bottom() + 5 * btnHeight + 4;
		return QSize(w, h);	
	}
	else
	{
		int MIN_WITH = 4,
			MIN_HEIGTH = 4;
		int w = contentsMargins().left() + contentsMargins().right() + 13 + 14.5 * MIN_WITH;
		int h = contentsMargins().top() + contentsMargins().bottom() + 5 * MIN_HEIGTH + 4;
		return QSize(w, h);	
	}
}

const QString* UBKeyboardPalette::getLocaleName()
{
	if (locales==NULL)
		return NULL;
	return &(locales[nCurrentLocale]->name);
}

void UBKeyboardPalette::setLocale(int nLocale)
{
	if (locales!=NULL)
	{
		nCurrentLocale = nLocale;

		setInput(locales[nCurrentLocale]);
		onLocaleChanged(locales[nCurrentLocale]);
		update();
	}
	emit localeChanged(nLocale);
}

void UBKeyboardPalette::setKeyButtonSize(const QString& strSize)
{
	QStringList strs = strSize.split('x');

	if (strs.size()==2)
	{
		btnWidth = strs[0].toInt();
		btnHeight = strs[1].toInt();
		adjustSizeAndPosition();
	}
	
}

void UBKeyboardPalette::setAutoMinimize(bool autoMinimize)
{
	this->autoMinimize = autoMinimize;
	adjustSizeAndPosition();
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
	if (languagePopupActive)
		return;
	if (!keyboardActive)
		return;

	keyboardActive = false;

	adjustSizeAndPosition();

	emit keyboardActivated(false);
}

void  UBKeyboardPalette::moveEvent ( QMoveEvent * event )
{
	UBFloatingPalette::moveEvent(event);
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
	UBFloatingPalette::adjustSizeAndPosition(pUp);
}

void  UBKeyboardPalette::paintEvent( QPaintEvent* event)
{
	UBFloatingPalette::paintEvent(event);

	QRect r = this->geometry();

	int lleft, ltop, lright, lbottom;
	getContentsMargins ( &lleft, &ltop, &lright, &lbottom ) ;

	int btw = (r.width() - lleft - lright - 13) / 14.5;
	int bth = (r.height() - ltop - lbottom -4) / 5;

	lleft = (r.width() - (btw * 14.5 + 13))/2;
	ltop = (r.height() - (bth * 5 + 4)) / 2;

	int off = lleft;;
	for(int i = 0; i<13; i++)
	{
		QRect cr(off, ltop, btw, bth);
		buttons[i]->setGeometry(cr);
		off += btw + 1;
	}
	ctrlButtons[0]->setGeometry(off, ltop, btw * 1.5, bth);
	off += btw * 1.5;
	lright = off;

	ltop += bth + 1;
	int lwidth = 12 * btw + 12 + btw * 1.3;
	off = (r.width() - lwidth) / 2;
	ctrlButtons[1]->setGeometry(off, ltop, btw * 1.3, bth);
	off += btw * 1.3 + 1;
	for(int i=0; i<12; i++)
	{
		QRect cr(off, ltop, btw, bth);
		buttons[13 + i]->setGeometry(cr);
		off += btw + 1;
	}

	ltop += bth + 1;
	lwidth = 12 * btw + 12 + btw * 2;
	off = (r.width() - lwidth) / 2;
	for(int i=0; i<12; i++)
	{
		QRect cr(off, ltop, btw, bth);
		buttons[13 + 12 + i]->setGeometry(cr);
		off += btw + 1;
	}
	ctrlButtons[2]->setGeometry(off, ltop, btw * 2, bth);

	ltop += bth + 1;
	lwidth = 12 * btw + 11;
	off = (r.width() - lwidth) / 2;
	ctrlButtons[3]->setGeometry(off, ltop, btw, bth);
	off+=btw+1;
	for(int i=0; i<10; i++)
	{
		QRect cr(off, ltop, btw, bth);
		buttons[13 + 12 + 12 + i]->setGeometry(cr);
		off += btw + 1;
	}
	ctrlButtons[4]->setGeometry(r.x() + off, ltop, btw, bth);

	ltop += bth + 1;
	lwidth = 11 * btw + 2;
	off = (r.width() - lwidth) / 2;
	ctrlButtons[5]->setGeometry(off, ltop, btw, bth);
	ctrlButtons[6]->setGeometry(off + btw + 1, ltop, btw*9, bth);
	ctrlButtons[7]->setGeometry(off + btw*10 + 2, ltop, btw, bth);
}


/*

			UBKeyboardButton
*/

UBKeyboardButton::UBKeyboardButton(UBKeyboardPalette* parent)
	:QWidget(parent), 
	bFocused(false),
	bPressed(false),
	keyboard(parent)
{
	setCursor(Qt::PointingHandCursor);
}

UBKeyboardButton::~UBKeyboardButton()
{}

void UBKeyboardButton::paintEvent(QPaintEvent*)
{
	QPainter painter(this);

	if (bPressed)
		painter.setBrush(QBrush(QColor(0, 0xcc, 0)));
	else if (bFocused)
		painter.setBrush(QBrush(QColor(0xcc, 0, 0)));
	
    painter.drawRoundedRect(0, 0, width()-1, height()-1, 3, 3);

	this->paintContent(painter);
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
	painter.drawText(rect(), Qt::AlignCenter, label);
}

UBCapsLockButton::UBCapsLockButton(UBKeyboardPalette* parent)
:UBKeyboardButton(parent)
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
	painter.drawText(rect(), Qt::AlignCenter, "^");
}


UBLocaleButton::UBLocaleButton(UBKeyboardPalette* parent)
:UBKeyboardButton(parent)
{
	localeMenu = new QMenu(this);

	for(int i=0; i<parent->nLocalesCount; i++)
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
