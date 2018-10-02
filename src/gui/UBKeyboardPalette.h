/*
 * Copyright (C) 2015-2018 Département de l'Instruction Publique (DIP-SEM)
 *
 * Copyright (C) 2013 Open Education Foundation
 *
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour
 * l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of OpenBoard.
 *
 * OpenBoard is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * OpenBoard is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenBoard. If not, see <http://www.gnu.org/licenses/>.
 */




#ifndef UBKEYBOARDPALETTE_H
#define UBKEYBOARDPALETTE_H

#include "UBActionPalette.h"

#include <QLayout>
#include <QPainter>
#include <QMenu>
#include <QIcon>

#include "frameworks/UBPlatformUtils.h"

class UBKeyButton;
class UBKeyboardButton;

class UBApplication;
class UBMainWindow;

class BTNImages
{
public:

    BTNImages(QString _strHeight, int _width, int _height);

    QString m_strHeight;
    int m_width;
    int m_height;

    QImage m_btnLeftPassive;
    QImage m_btnCenterPassive;
    QImage m_btnRightPassive;
    QImage m_btnLeftActive;
    QImage m_btnCenterActive;
    QImage m_btnRightActive;

private:
    QString m_strLeftPassive;
    QString m_strCenterPassive;
    QString m_strRightPassive;
    QString m_strLeftActive;
    QString m_strCenterActive;
    QString m_strRightActive;

};

class ContentImage
{
public:

    ContentImage(QString strHeight, int m_height, QString strContentPath);

    QString m_strHeight;
    int m_height;

    QImage m_btnContent;

private:
    QString m_strContent;
};

class UBKeyboardPalette : public UBActionPalette
{
    Q_OBJECT

friend class UBKeyboardButton;
friend class UBCapsLockButton;
friend class UBShiftButton;
friend class UBLocaleButton;
friend class UBKeyButton;

public:
    UBKeyboardPalette(QWidget *parent);
    ~UBKeyboardPalette();

    BTNImages *currBtnImages;

    bool isEnabled(){return locales!= NULL;}
    virtual QSize  sizeHint () const;
    virtual void adjustSizeAndPosition(bool pUp = true);
    QString getKeyButtonSize() const {QString res; res.sprintf("%dx%d", btnWidth, btnHeight); return res;}
    void setKeyButtonSize(const QString& strSize);

    bool m_isVisible;
    QPoint m_pos;

signals:
    void moved(const QPoint&);
    void localeChanged(int);
    void keyboardActivated(bool);

private slots:
    void syncPosition(const QPoint & pos);
    void syncLocale(int nLocale);
    void keyboardPaletteButtonSizeChanged(QVariant size);
    void onActivated(bool b);
    void onDeactivated();
    void showKeyboard(bool show);
    void hideKeyboard();

protected:
    bool capsLock;
    bool shift;
    int nCurrentLocale;
    int nLocalesCount;
    UBKeyboardLocale** locales;

    int nSpecialModifierIndex;
    KEYCODE specialModifier;

    QString strSize;
    int btnWidth;
    int btnHeight;
// 
    bool languagePopupActive;
    bool keyboardActive;
// 
    virtual void  enterEvent ( QEvent * event );
    virtual void  leaveEvent ( QEvent * event );
    virtual void  paintEvent(QPaintEvent *event);
    virtual void  moveEvent ( QMoveEvent * event );

    void sendKeyEvent(KEYCODE keyCode);

    void setLocale(int nLocale);

    const QString* getLocaleName();

    void init();


private:

    QRect originalRect;

    UBKeyButton** buttons;
    UBKeyboardButton** ctrlButtons;

    /*
      For MacOS: synchronization with system locale.
    */
    void checkLayout();

    void createCtrlButtons();

    void setInput(const UBKeyboardLocale* locale);

    // Can be redefined under each platform
    void onLocaleChanged(UBKeyboardLocale* locale);

    // Storage for platform-dependent objects (linux)
    void* storage;
    // Linux-related parameters
    int min_keycodes, max_keycodes, byte_per_code;
};

class UBKeyboardButton : public QWidget
{
    Q_OBJECT

public:
    UBKeyboardButton(UBKeyboardPalette* parent, QString contentImagePath);
    ~UBKeyboardButton();

protected:

    UBKeyboardPalette* m_parent;
    ContentImage *imgContent;
    QString m_contentImagePath;

    void paintEvent(QPaintEvent *event);

    virtual void  enterEvent ( QEvent * event );
    virtual void  leaveEvent ( QEvent * event );
    virtual void  mousePressEvent ( QMouseEvent * event );
    virtual void  mouseReleaseEvent ( QMouseEvent * event );

    virtual void onPress() = 0;
    virtual void onRelease() = 0;
    virtual void paintContent(QPainter& painter) = 0;

    virtual bool isPressed();

    UBKeyboardPalette* keyboard;

    void sendUnicodeSymbol(KEYCODE keycode);
    void sendControlSymbol(int nSymbol);

private:
    bool bFocused;
    bool bPressed;
};

class UBKeyButton : public UBKeyboardButton
{
    Q_OBJECT

public:
    UBKeyButton(UBKeyboardPalette* parent);
    ~UBKeyButton();

    void setKeyBt(const KEYBT* keybt){this->keybt = keybt;}

    virtual void onPress();
    virtual void onRelease();
    virtual void paintContent(QPainter& painter);

private:
    bool shifted();
    const KEYBT* keybt;
};

class UBCntrlButton : public UBKeyboardButton
{
    Q_OBJECT

public:
    UBCntrlButton(UBKeyboardPalette* parent, int _code, const QString& _contentImagePath );
    UBCntrlButton(UBKeyboardPalette* parent, const QString& _label, int _code );
    ~UBCntrlButton();

    virtual void onPress();
    virtual void onRelease();
    virtual void paintContent(QPainter& painter);

private:
    QString label;
    int code;
};

class UBCapsLockButton : public UBKeyboardButton
{
    Q_OBJECT

public:
    UBCapsLockButton(UBKeyboardPalette* parent, const QString _contentImagePath);
    ~UBCapsLockButton();

    virtual void onPress();
    virtual void onRelease();
    virtual void paintContent(QPainter& painter);

protected:
    virtual bool isPressed();
};

class UBShiftButton : public UBKeyboardButton
{
    Q_OBJECT

public:
    UBShiftButton(UBKeyboardPalette* parent, const QString _contentImagePath);
    ~UBShiftButton();

    virtual void onPress();
    virtual void onRelease();
    virtual void paintContent(QPainter& painter);

protected:
    virtual bool isPressed();
};


class UBLocaleButton : public UBKeyboardButton
{
    Q_OBJECT

public:
    UBLocaleButton(UBKeyboardPalette* parent);
    ~UBLocaleButton();

    virtual void onPress();
    virtual void onRelease();
    virtual void paintContent(QPainter& painter);

protected:
    QMenu* localeMenu;
};

#endif // UBKEYBOARDPALETTE_H
