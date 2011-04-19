#ifndef UBKEYBOARDPALETTE_H
#define UBKEYBOARDPALETTE_H

#include "UBFloatingPalette.h"


#include <QLayout>
#include <QPainter>
#include <QMenu>
#include <QIcon>

#include "frameworks/UBPlatformUtils.h"

class UBKeyButton;
class UBKeyboardButton;

class UBKeyboardPalette : public UBFloatingPalette
{
    Q_OBJECT

friend class UBKeyboardButton;
friend class UBCapsLockButton;
friend class UBLocaleButton;

public:
    ~UBKeyboardPalette();

    bool isEnabled(){return locales!= NULL;}
    virtual QSize  sizeHint () const;
    virtual void adjustSizeAndPosition(bool pUp = true);
    QString getKeyButtonSize() const {QString res; res.sprintf("%dx%d", btnWidth, btnHeight); return res;}
    void setKeyButtonSize(const QString& strSize);
    void setAutoMinimize(bool autoMinimize);
    static UBKeyboardPalette* create(QWidget *parent);

signals:
    void moved(const QPoint&);
    void localeChanged(int);
    void keyboardActivated(bool);

private slots:
    void syncPosition(const QPoint & pos);
    void syncLocale(int nLocale);
    void keyboardPaletteAutoMinimizeChanged(QVariant b);
    void keyboardPaletteButtonSizeChanged(QVariant size);
    void onActivated(bool b);

protected:
    bool capsLock;
    int nCurrentLocale;
    int nLocalesCount;
    UBKeyboardLocale** locales;

    int btnWidth;
    int btnHeight;

    bool languagePopupActive;
    bool keyboardActive;
    bool autoMinimize;

    virtual void  enterEvent ( QEvent * event );
    virtual void  leaveEvent ( QEvent * event );
    virtual void paintEvent(QPaintEvent *event);
    virtual void  moveEvent ( QMoveEvent * event );

    void sendKeyEvent(const KEYBT& keybt);

    void setLocale(int nLocale);

    const QString* getLocaleName();
	
private:

    UBKeyboardPalette(QWidget *parent);
    static QList<UBKeyboardPalette*> instances;

    QRect originalRect;

    UBKeyButton** buttons;
    UBKeyboardButton** ctrlButtons;

    void createCtrlButtons();

    void setInput(const UBKeyboardLocale* locale);

    // Can be redefined under each platform
    void onLocaleChanged(UBKeyboardLocale* locale);

    // Storage for platform-dependent objects (linux)
    void* storage;
    // Linux-related parameters
    int min_keycodes, max_keycodes, byte_per_code;

    // Save locale before activation to restore it after (MAC)
    QString activeLocale;

};


class UBKeyboardButton : public QWidget
{
    Q_OBJECT

public:
    UBKeyboardButton(UBKeyboardPalette* parent);
    ~UBKeyboardButton();

protected:
    void paintEvent(QPaintEvent *event);
    virtual void  enterEvent ( QEvent * event );
    virtual void  leaveEvent ( QEvent * event );
    virtual void  mousePressEvent ( QMouseEvent * event );
    virtual void  mouseReleaseEvent ( QMouseEvent * event );

    virtual void onPress() = 0;
    virtual void onRelease() = 0;
    virtual void paintContent(QPainter& painter) = 0;

    bool capsLock(){return keyboard->capsLock;}

    UBKeyboardPalette* keyboard;

    void sendUnicodeSymbol(unsigned int nSymbol1, unsigned int nSymbol2, bool shift);
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
    const KEYBT* keybt;
};

class UBCntrlButton : public UBKeyboardButton
{
    Q_OBJECT

public:
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
    UBCapsLockButton(UBKeyboardPalette* parent);
    ~UBCapsLockButton();

    virtual void onPress();
    virtual void onRelease();
    virtual void paintContent(QPainter& painter);
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
