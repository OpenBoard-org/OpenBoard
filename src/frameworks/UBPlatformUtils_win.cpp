/*
 * Copyright (C) 2015-2022 Département de l'Instruction Publique (DIP-SEM)
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




#include "UBPlatformUtils.h"

#include <QtGui>
#include <QProcess>
#include <windows.h>
#include <shellapi.h>

#include "frameworks/UBFileSystemUtils.h"
#include "core/memcheck.h"
#include "core/UBSettings.h"

void UBPlatformUtils::init()
{
    initializeKeyboardLayouts();
}


QString UBPlatformUtils::applicationResourcesDirectory()
{
    return QApplication::applicationDirPath();
}


void UBPlatformUtils::hideFile(const QString &filePath)
{
    Q_UNUSED(filePath);

    // TODO UB 4.x : hide file from the Windows explorer
}

void UBPlatformUtils::setFileType(const QString &filePath, unsigned long fileType)
{
    Q_UNUSED(filePath);
    Q_UNUSED(fileType);

    // Probably no fileType equivalent on Windows
}

void UBPlatformUtils::fadeDisplayOut()
{
    // NOOP
}

void UBPlatformUtils::fadeDisplayIn()
{
    // NOOP
}

QStringList UBPlatformUtils::availableTranslations()
{
    QString translationsPath = applicationResourcesDirectory() + "/" + "i18n" + "/";
    QStringList translationsList = UBFileSystemUtils::allFiles(translationsPath);
    QRegularExpression sankoreTranslationFiles(".*OpenBoard_.*.qm");
    translationsList=translationsList.filter(sankoreTranslationFiles);
    return translationsList.replaceInStrings(QRegularExpression("(.*)OpenBoard_(.*).qm"),"\\2");
}

QString UBPlatformUtils::translationPath(QString pFilePrefix,QString pLanguage)
{
    QString qmPath = applicationResourcesDirectory() + "/" + "i18n" + "/" + pFilePrefix + pLanguage + ".qm";
    return qmPath;
}

QString UBPlatformUtils::systemLanguage()
{
    return QLocale::system().name();
}

void UBPlatformUtils::bringPreviousProcessToFront()
{
    // Mac only
}


QString UBPlatformUtils::osUserLoginName()
{
    WCHAR winUserName[256 + 1];
    DWORD winUserNameSize = sizeof(winUserName);
    GetUserName( winUserName, &winUserNameSize );
    QString userName = QString::fromWCharArray(winUserName, winUserNameSize - 1);

    return userName;
}


QString UBPlatformUtils::computerName()
{
    WCHAR winComputerName[256 + 1];
    DWORD winComputerNameSize = sizeof(winComputerName);
    GetComputerName(winComputerName, &winComputerNameSize );
    QString computerName = QString::fromWCharArray(winComputerName, winComputerNameSize - 1);

    return computerName;
}


void UBPlatformUtils::setDesktopMode(bool desktop)
{
    Q_UNUSED(desktop);
}

void UBPlatformUtils::setWindowNonActivableFlag(QWidget* widget, bool nonAcivable)
{/*
    long exStyle = (nonAcivable) ? GetWindowLong(widget->winId(), GWL_EXSTYLE) | WS_EX_NOACTIVATE
        : GetWindowLong(widget->winId(), GWL_EXSTYLE) & ~WS_EX_NOACTIVATE;

    SetWindowLong(widget->winId(), GWL_EXSTYLE, exStyle);
    */
}

#define KEYBTDECL(s1, s2, clSwitch) KEYBT(s1, s2, clSwitch, 0, 0, KEYCODE(s1), KEYCODE(s2))

KEYBT RUSSIAN_LOCALE [] =
{
    /* Ј і */ KEYBTDECL(0x451, 0x401, true),
    /* 1 ! */ KEYBTDECL(0x31, 0x21, false),
    /* 2 " */ KEYBTDECL(0x32, 0x5c, false),
    /* 3 ? */ KEYBTDECL(0x33, 0x2116, false),
    /* 4 ; */ KEYBTDECL(0x34, 0x3b, false),
    /* 5 % */ KEYBTDECL(0x35, 0x25, false),
    /* 6 : */ KEYBTDECL(0x36, 0x3a, false),
    /* 7 ? */ KEYBTDECL(0x37, 0x3f, false),
    /* 8 * */ KEYBTDECL(0x38, 0x2a, false),
    /* 9 ( */ KEYBTDECL(0x39, 0x28, false),
    /* 0 ) */ KEYBTDECL(0x30, 0x29, false),
    /* - _ */ KEYBTDECL(0x2d, 0x4f, false),
    /* = + */ KEYBTDECL(0x3d, 0x2b, false),

    /* К к */ KEYBTDECL(0x439, 0x419, true),
    /* Г г */ KEYBTDECL(0x446, 0x426, true),
    /* Х х */ KEYBTDECL(0x443, 0x423, true),
    /* Л л */ KEYBTDECL(0x43a, 0x41a, true),
    /* Е е */ KEYBTDECL(0x435, 0x415, true),
    /* О о */ KEYBTDECL(0x43d, 0x41d, true),
    /* З з */ KEYBTDECL(0x433, 0x413, true),
    /* Ы ы */ KEYBTDECL(0x448, 0x428, true),
    /* Э э */ KEYBTDECL(0x449, 0x429, true),
    /* Ъ ъ */ KEYBTDECL(0x437, 0x417, true),
    /* И и */ KEYBTDECL(0x445, 0x425, true),
    /* Я я */ KEYBTDECL(0x44a, 0x42a, true),

    /* Ж ж */ KEYBTDECL(0x444, 0x424, true),
    /* Щ щ */ KEYBTDECL(0x44b, 0x42b, true),
    /* Ч ч */ KEYBTDECL(0x432, 0x412, true),
    /* Б б */ KEYBTDECL(0x430, 0x410, true),
    /* Р р */ KEYBTDECL(0x43f, 0x41f, true),
    /* Т т */ KEYBTDECL(0x440, 0x420, true),
    /* П п */ KEYBTDECL(0x43e, 0x41e, true),
    /* М м */ KEYBTDECL(0x43b, 0x41b, true),
    /* Д д */ KEYBTDECL(0x434, 0x414, true),
    /* Ц ц */ KEYBTDECL(0x436, 0x416, true),
    /* Ь ь */ KEYBTDECL(0x44d, 0x42d, true),
    /* \ / */ KEYBTDECL(0x5c, 0x2f, false),

    /* С с */ KEYBTDECL(0x44f, 0x42f, true),
    /* Ю ю */ KEYBTDECL(0x447, 0x427, true),
    /* У у */ KEYBTDECL(0x441, 0x421, true),
    /* Н н */ KEYBTDECL(0x43c, 0x41c, true),
    /* Й й */ KEYBTDECL(0x438, 0x418, true),
    /* Ф ф */ KEYBTDECL(0x442, 0x422, true),
    /* Ш ш */ KEYBTDECL(0x44c, 0x42c, true),
    /* В в */ KEYBTDECL(0x431, 0x411, true),
    /* А а */ KEYBTDECL(0x44e, 0x42e, true),
    /* . , */ KEYBTDECL(0x2e, 0x2c, false)};

KEYBT ENGLISH_LOCALE[] = {
    /* ` ~ */ KEYBTDECL(0x60, 0x7e, false),
    /* 1 ! */ KEYBTDECL(0x31, 0x21, false),
    /* 2 @ */ KEYBTDECL(0x32, 0x40, false),
    /* 3 # */ KEYBTDECL(0x33, 0x23, false),
    /* 4 $ */ KEYBTDECL(0x34, 0x24, false),
    /* 5 % */ KEYBTDECL(0x35, 0x25, false),
    /* 6 ^ */ KEYBTDECL(0x36, 0x5e, false),
    /* 7 & */ KEYBTDECL(0x37, 0x26, false),
    /* 8 * */ KEYBTDECL(0x38, 0x2a, false),
    /* 9 ( */ KEYBTDECL(0x39, 0x28, false),
    /* 0 ) */ KEYBTDECL(0x30, 0x29, false),
    /* - _ */ KEYBTDECL(0x2d, 0x5f, false),
    /* = + */ KEYBTDECL(0x3d, 0x2b, false),

    /* q Q */ KEYBTDECL(0x71, 0x51, true),
    /* w W */ KEYBTDECL(0x77, 0x57, true),
    /* e E */ KEYBTDECL(0x65, 0x45, true),
    /* r R */ KEYBTDECL(0x72, 0x52, true),
    /* t T */ KEYBTDECL(0x74, 0x54, true),
    /* y Y */ KEYBTDECL(0x79, 0x59, true),
    /* u U */ KEYBTDECL(0x75, 0x55, true),
    /* i I */ KEYBTDECL(0x69, 0x49, true),
    /* o O */ KEYBTDECL(0x6f, 0x4f, true),
    /* p P */ KEYBTDECL(0x70, 0x50, true),
    /* [ { */ KEYBTDECL(0x5b, 0x7b, false),
    /* ] } */ KEYBTDECL(0x5d, 0x7d, false),

    /* a A */ KEYBTDECL(0x61, 0x41, true),
    /* s S */ KEYBTDECL(0x73, 0x53, true),
    /* d D */ KEYBTDECL(0x64, 0x44, true),
    /* f F */ KEYBTDECL(0x66, 0x46, true),
    /* g G */ KEYBTDECL(0x67, 0x47, true),
    /* h H */ KEYBTDECL(0x68, 0x48, true),
    /* j J */ KEYBTDECL(0x6a, 0x4a, true),
    /* k K */ KEYBTDECL(0x6b, 0x4b, true),
    /* l L */ KEYBTDECL(0x6c, 0x4c, true),
    /* ; : */ KEYBTDECL(0x3b, 0x3a, false),
    /* ' " */ KEYBTDECL(0x27, 0x22, false),
    /* \ | */ KEYBTDECL(0x5c, 0x7c, false),

    /* z Z */ KEYBTDECL(0x7a, 0x5a, true),
    /* x X */ KEYBTDECL(0x78, 0x58, true),
    /* c C */ KEYBTDECL(0x63, 0x43, true),
    /* v V */ KEYBTDECL(0x76, 0x56, true),
    /* b B */ KEYBTDECL(0x62, 0x42, true),
    /* n N */ KEYBTDECL(0x6e, 0x4e, true),
    /* m M */ KEYBTDECL(0x6d, 0x4d, true),
    /* , < */ KEYBTDECL(0x2c, 0x3c, false),
    /* . > */ KEYBTDECL(0x2e, 0x3e, false),
    /* / ? */ KEYBTDECL(0x2f, 0x5f, false)};


KEYBT FRENCH_LOCALE[] = {
    /* ? ? */ KEYBTDECL(0xb2, 0xb2, true),
    /* & 1 */ KEYBTDECL(0x26, 0x31, true),
    /* ? 2 */ KEYBTDECL(0xe9, 0x32, true),
    /* " 3 */ KEYBTDECL(0x22, 0x33, true),
    /* ' 4 */ KEYBTDECL(0x27, 0x34, true),
    /* ( 5 */ KEYBTDECL(0x28, 0x35, true),
    /* - 6 */ KEYBTDECL(0x2d, 0x36, true),
    /* ? 7 */ KEYBTDECL(0xe8, 0x37, true),
    /* _ 8 */ KEYBTDECL(0x5f, 0x38, true),
    /* ? 9 */ KEYBTDECL(0xe7, 0x39, true),
    /* ? 0 */ KEYBTDECL(0xe0, 0x30, true),
    /* ) ? */ KEYBTDECL(0x29, 0xb0, true),
    /* = + */ KEYBTDECL(0x3d, 0x2b, true),

    /* a A */ KEYBT(0x61, 0x41, true, 0, 0, 0x61, 0x41, 0xe2, 0xc2, 0xe4, 0xc4),
    /* z Z */ KEYBTDECL(0x7a, 0x5a, true),
    /* e E */ KEYBT(0x65, 0x45, true, 0, 0, 0x65, 0x45, 0xea, 0xca, 0xeb, 0xcb),
    /* r R */ KEYBTDECL(0x72, 0x52, true),
    /* t T */ KEYBTDECL(0x74, 0x54, true),
    /* y Y */ KEYBT(0x79, 0x59, true, 0, 0, 0x79, 0x59, 0, 0, 0xff),
    /* u U */ KEYBT(0x75, 0x55, true, 0, 0, 0x75, 0x55, 0xfb, 0xdb, 0xfc, 0xdc),
    /* i I */ KEYBT(0x69, 0x49, true, 0, 0, 0x69, 0x49, 0xee, 0xce, 0xef, 0xcf),
    /* o O */ KEYBT(0x6f, 0x4f, true, 0, 0, 0x6f, 0x4f, 0xf4, 0xd4, 0xf6, 0xd6),
    /* p P */ KEYBTDECL(0x70, 0x50, true),
    /* ^ ? */ KEYBT(0x5e, 0xa8, true, 1, 2, 0x5e, 0xa8),
    /* $ ? } */ KEYBTDECL(0x24, 0xa3, true),

    /* q Q */ KEYBTDECL(0x71, 0x51, true),
    /* s S */ KEYBTDECL(0x73, 0x53, true),
    /* d D */ KEYBTDECL(0x64, 0x44, true),
    /* f F */ KEYBTDECL(0x66, 0x46, true),
    /* g G */ KEYBTDECL(0x67, 0x47, true),
    /* h H */ KEYBTDECL(0x68, 0x48, true),
    /* j J */ KEYBTDECL(0x6a, 0x4a, true),
    /* k K */ KEYBTDECL(0x6b, 0x4b, true),
    /* l L */ KEYBTDECL(0x6c, 0x4c, true),
    /* m M */ KEYBTDECL(0x6d, 0x4d, true),
    /* ? % */ KEYBTDECL(0xf9, 0x25, true),
    /* * ? */ KEYBTDECL(0x2a, 0xb5, true),

    /* w W */ KEYBTDECL(0x77, 0x57, true),
    /* x X */ KEYBTDECL(0x78, 0x58, true),
    /* c C */ KEYBTDECL(0x63, 0x43, true),
    /* v V */ KEYBTDECL(0x76, 0x56, true),
    /* b B */ KEYBTDECL(0x62, 0x42, true),
    /* n N */ KEYBTDECL(0x6e, 0x4e, true),
    /* , ? */ KEYBTDECL(0x2c, 0x3f, true),
    /* ; . */ KEYBTDECL(0x3b, 0x2e, true),
    /* : / */ KEYBTDECL(0x3a, 0x2f, true),
    /* ! ? */ KEYBTDECL(0x21, 0xa7, true)};

KEYBT SWISS_FRENCH_LOCALE[] = {
    /* § °*/    KEYBTDECL(0xa7, 0xb0, false),
    /* 1 +*/    KEYBTDECL(0x31, 0x2b, false),
    /* 2 "*/    KEYBTDECL(0x32, 0x22, false),
    /* 3 **/    KEYBTDECL(0x33, 0x2a, false),
    /* 4 ç*/    KEYBTDECL(0x34, 0xe7, false),
    /* 5 %*/    KEYBTDECL(0x35, 0x25, false),
    /* 6 &*/    KEYBTDECL(0x36, 0x26, false),
    /* 7 /*/    KEYBTDECL(0x37, 0x2f, false),
    /* 8 (*/    KEYBTDECL(0x38, 0x28, false),
    /* 9 )*/    KEYBTDECL(0x39, 0x29, false),
    /* 0 =*/    KEYBTDECL(0x30, 0x3d, false),
    /* ' ?*/    KEYBTDECL(0x27, 0x5f, false),
    /* ^ `*/    KEYBT(0x5e, 0x60, false, 1, 2, 0x5e, 0x60),

    /* q Q*/    KEYBTDECL(0x71, 0x51, true),
    /* w W*/    KEYBTDECL(0x77, 0x57, true),
    /* e E*/    KEYBT(0x65, 0x45, true, 0, 0, 0x65, 0x45, 0xea, 0xca, 0xe8, 0xc8, 0xeb, 0xcb),
    /* r R*/    KEYBTDECL(0x72, 0x52, true),
    /* t T*/    KEYBTDECL(0x74, 0x54, true),
    /* z Z*/    KEYBTDECL(0x7a, 0x5a, true),
    /* u U*/    KEYBT(0x75, 0x55, true, 0, 0, 0x75, 0x55, 0xfb, 0xdb, 0xf9, 0xd9, 0xfc, 0xdc),
    /* i I*/    KEYBT(0x69, 0x49, true, 0, 0, 0x69, 0x49, 0xee, 0xce, 0xec, 0xcc, 0xef, 0xcf),
    /* o O*/    KEYBT(0x6f, 0x4f, true, 0, 0, 0x6f, 0x4f, 0xf4, 0xd4, 0xf2, 0xd2, 0xf6, 0xd6),
    /* p P*/    KEYBTDECL(0x70, 0x50, true),
    /* è ü*/    KEYBTDECL(0xe8, 0xfa, false),
    /* ¨ !*/    KEYBT(0xa8, 0x21, false, 3, 0, 0xa8, 0x21),

    /* a A*/    KEYBT(0x61, 0x41, true, 0, 0, 0x61, 0x41, 0xe2, 0xc2, 0xe0, 0xc0, 0xe4, 0xc4),
    /* s S*/    KEYBTDECL(0x73, 0x53, true),
    /* d D*/    KEYBTDECL(0x64, 0x44, true),
    /* f F*/    KEYBTDECL(0x66, 0x46, true),
    /* g G*/    KEYBTDECL(0x67, 0x47, true),
    /* h H*/    KEYBTDECL(0x68, 0x48, true),
    /* j J*/    KEYBTDECL(0x6a, 0x4a, true),
    /* k K*/    KEYBTDECL(0x6b, 0x4b, true),
    /* l L*/    KEYBTDECL(0x6c, 0x4c, true),
    /* é ö*/    KEYBTDECL(0xe9, 0xf6, false),
    /* à ä*/    KEYBTDECL(0xe0, 0xe4, false),
    /* $ £*/    KEYBTDECL(0x24, 0xa3, false),

    /* < >*/    KEYBTDECL(0x3c, 0x3e, false),
    /* y Y*/    KEYBT(0x79, 0x59, true, 0, 0, 0x79, 0x59, 0, 0, 0, 0, 0xff),
    /* x X*/    KEYBTDECL(0x78, 0x58, true),
    /* c C*/    KEYBTDECL(0x63, 0x43, true),
    /* v V*/    KEYBTDECL(0x76, 0x56, true),
    /* b B*/    KEYBTDECL(0x62, 0x42, true),
    /* n N*/    KEYBTDECL(0x6e, 0x4e, true),
    /* m M*/    KEYBTDECL(0x6d, 0x4d, true),
    /* , ;*/    KEYBTDECL(0x2c, 0x3b, false),
    /* . :*/    KEYBTDECL(0x2e, 0x3a, false),
    /* - _*/    KEYBTDECL(0x2d, 0x5f, false)};


KEYBT GERMAN_LOCALE[] = {
    /* ^ ? */ KEYBTDECL(0x5f, 0xb0, false),
    /* 1 ! */ KEYBTDECL(0x31, 0x21, true),
    /* 2 " */ KEYBTDECL(0x32, 0x22, true),
    /* 3 § */ KEYBTDECL(0x33, 0xa7, true),
    /* 4 $ */ KEYBTDECL(0x34, 0x24, true),
    /* 5 % */ KEYBTDECL(0x35, 0x25, true),
    /* 6 & */ KEYBTDECL(0x36, 0x26, true),
    /* 7 / */ KEYBTDECL(0x37, 0x2f, true),
    /* 8 ( */ KEYBTDECL(0x38, 0x28, true),
    /* 9 ) */ KEYBTDECL(0x39, 0x29, true),
    /* 0 = */ KEYBTDECL(0x30, 0x3d, true),
    /* ? ? */ KEYBTDECL(0xdf, 0x3f, true),
    /* ` ? */ KEYBTDECL(0x60, 0xb4, false),

    /* q Q */ KEYBTDECL(0x71, 0x51, true),
    /* w W */ KEYBTDECL(0x77, 0x57, true),
    /* e E */ KEYBTDECL(0x65, 0x45, true),
    /* r R */ KEYBTDECL(0x72, 0x52, true),
    /* t T */ KEYBTDECL(0x74, 0x54, true),
    /* z Z */ KEYBTDECL(0x7a, 0x5a, true),
    /* u U */ KEYBTDECL(0x75, 0x55, true),
    /* i I */ KEYBTDECL(0x69, 0x49, true),
    /* o O */ KEYBTDECL(0x6f, 0x4f, true),
    /* p P */ KEYBTDECL(0x70, 0x50, true),
    /* ? ? */ KEYBTDECL(0xfc, 0xdc, true),
    /* + * */ KEYBTDECL(0x2b, 0x2a, true),

    /* a A */ KEYBTDECL(0x61, 0x41, true),
    /* s S */ KEYBTDECL(0x73, 0x53, true),
    /* d D */ KEYBTDECL(0x64, 0x44, true),
    /* f F */ KEYBTDECL(0x66, 0x46, true),
    /* g G */ KEYBTDECL(0x67, 0x47, true),
    /* h H */ KEYBTDECL(0x68, 0x48, true),
    /* j J */ KEYBTDECL(0x6a, 0x4a, true),
    /* k K */ KEYBTDECL(0x6b, 0x4b, true),
    /* l L */ KEYBTDECL(0x6c, 0x4c, true),
    /* ? ? */ KEYBTDECL(0xf6, 0xd6, true),
    /* ? ? */ KEYBTDECL(0xe4, 0xc4, true),
    /* ' # */ KEYBTDECL(0x27, 0x23, true),

    /* y Y */ KEYBTDECL(0x79, 0x59, true),
    /* x X */ KEYBTDECL(0x78, 0x58, true),
    /* c C */ KEYBTDECL(0x63, 0x43, true),
    /* v V */ KEYBTDECL(0x76, 0x56, true),
    /* b B */ KEYBTDECL(0x62, 0x42, true),
    /* n N */ KEYBTDECL(0x6e, 0x4e, true),
    /* m M */ KEYBTDECL(0x6d, 0x4d, true),
    /* , ; */ KEYBTDECL(0x2c, 0x3b, true),
    /* . : */ KEYBTDECL(0x2e, 0x3a, true),
    /* _ - */ KEYBTDECL(0x5f, 0x2d, false)};

void UBPlatformUtils::initializeKeyboardLayouts()
{
    nKeyboardLayouts = 5;
    keyboardLayouts = new UBKeyboardLocale*[nKeyboardLayouts];
    keyboardLayouts[0] = new UBKeyboardLocale(tr("English"), "en", "", new QIcon(":/images/flags/en.png"), ENGLISH_LOCALE);
    keyboardLayouts[1] = new UBKeyboardLocale(tr("Russian"), "ru", "", new QIcon(":/images/flags/ru.png"),RUSSIAN_LOCALE);
    keyboardLayouts[2] = new UBKeyboardLocale(tr("French"), "fr", "", new QIcon(":/images/flags/fr.png"), FRENCH_LOCALE);
    keyboardLayouts[3] = new UBKeyboardLocale(tr("Swiss French"), "fr-CH", "", new QIcon(":/images/flags/fr.png"), SWISS_FRENCH_LOCALE);
    keyboardLayouts[4] = new UBKeyboardLocale(tr("German"), "de", "", new QIcon(":/images/flags/de.png"), GERMAN_LOCALE);
}

void UBPlatformUtils::destroyKeyboardLayouts()
{
    for(int i=0; i<nKeyboardLayouts; i++)
        delete keyboardLayouts[i];
    delete [] keyboardLayouts;
    keyboardLayouts = NULL;
}

QString UBPlatformUtils::urlFromClipboard()
{
    QString qsRet;
    //  Not implemented yet
    return qsRet;
}

void UBPlatformUtils::setFrontProcess()
{
    // not used in Windows
}


void UBPlatformUtils::showFullScreen(QWidget *pWidget)
{
    if (UBSettings::settings()->appRunInWindow->get().toBool()) {
        pWidget->show();
    } else {
        pWidget->showFullScreen();
    }
}

void UBPlatformUtils::showOSK(bool show)
{
    if (show) {
        QString windir = qgetenv("WINDIR");
        QString osk_path = windir+"\\System32\\osk.exe";

        QProcess oskProcess;
        // We have to pass by explorer.exe because osk.exe can only be launched
        // directly with administrator rights
        oskProcess.startDetached("explorer.exe", QStringList() << osk_path);
    }

    else {
        HWND oskWindow = ::FindWindow(TEXT("OSKMainClass"), NULL);
        if (oskWindow)
            PostMessage(oskWindow, WM_SYSCOMMAND, SC_CLOSE, 0);
    }
}
