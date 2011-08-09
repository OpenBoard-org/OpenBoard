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
friend class UBLocaleButton;

public:
    ~UBKeyboardPalette();

    BTNImages *currBtnImages;

    bool isEnabled(){return locales!= NULL;}
    virtual QSize  sizeHint () const;
    virtual void adjustSizeAndPosition(bool pUp = true);
    QString getKeyButtonSize() const {QString res; res.sprintf("%dx%d", btnWidth, btnHeight); return res;}
    void setKeyButtonSize(const QString& strSize);

    static UBKeyboardPalette* create(QWidget *parent);
    static QList<UBKeyboardPalette*> instances;
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
    void showKeyboard(bool show);
    void hideKeyboard();

protected:
    bool capsLock;
    int nCurrentLocale;
    int nLocalesCount;
    UBKeyboardLocale** locales;

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

    void sendKeyEvent(const KEYBT& keybt);

    void setLocale(int nLocale);

    const QString* getLocaleName();
	
private:

    UBKeyboardPalette(QWidget *parent);

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
