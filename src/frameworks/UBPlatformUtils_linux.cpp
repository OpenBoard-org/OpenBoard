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


#define QT_IMPLICIT_QCHAR_CONSTRUCTION

#include "UBPlatformUtils.h"

#include <QtGui>
#include <QApplication>

#include <unistd.h>
#include <X11/keysym.h>

#include "frameworks/UBFileSystemUtils.h"
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
    Q_UNUSED(filePath)
    // TODO UB 4.x Not possible on Linux as such, the filename should have a . as first char in name
}

void UBPlatformUtils::setFileType(const QString &filePath, unsigned long fileType)
{
    Q_UNUSED(filePath)
    Q_UNUSED(fileType)

    // No fileType equivalent on Linux
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
    static const QRegularExpression sankoreTranslationFiles("(.*)OpenBoard_(.*).qm");
    translationsList = translationsList.filter(sankoreTranslationFiles);
    return translationsList.replaceInStrings(sankoreTranslationFiles, "\\2");
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
    char *user = getenv("USER");
    return QString::fromLatin1(user);
}

QString UBPlatformUtils::computerName()
{
    char hostname[256];
    memset(hostname,0,256);
    // if the name is longer than 255 the name is truncated but os doesn't ensure
    // that the last character returned is a null character
    if(!gethostname(hostname,255))
        return QString::fromLatin1(hostname);
    else
        return "NOT FOUND";
}

void UBPlatformUtils::setWindowNonActivableFlag(QWidget* widget, bool nonAcivable)
{
    Q_UNUSED(widget);
    Q_UNUSED(nonAcivable);
}



void UBPlatformUtils::setDesktopMode(bool desktop)
{
    // NOOP
    Q_UNUSED(desktop);
}

#define KEYBTDECL(s1, s2, clSwitch, code) KEYBT(s1, s2, clSwitch, 0, 0, KEYCODE(s1, code, 0), KEYCODE(s2, code, 1))

#define KEYBTDECLEX1(s1, s2, clSwitch, code, cs1, cs2) KEYBT(s1, s2, clSwitch, 0, 0, KEYCODE(cs1, code, 0), KEYCODE(cs2, code, 1))
#define KEYBTDECLEX2(s1, s2, clSwitch, code, cs1, cs2) KEYBT(s1, s2, clSwitch, 0, 0, KEYCODE(cs1, code, 2), KEYCODE(cs2, code, 3))

#define KEYBTDECLEX6(s1, s2, clSwitch, code, cs1, cs2, cs3, cs4, cs5, cs6) KEYBT(s1, s2, clSwitch, 0, 0, KEYCODE(cs1, code, 0), KEYCODE(cs2, code, 1), KEYCODE(cs3, code, 2), KEYCODE(cs4, code, 3), KEYCODE(cs5, code, 4), KEYCODE(cs6, code, 5))

#define KEYBTDECLEX8(s1, s2, clSwitch, code, cs1, cs2, cs3, cs4, cs5, cs6, cs7, cs8) KEYBT(s1, s2, clSwitch, 0, 0, KEYCODE(cs1, code, 0), KEYCODE(cs2, code, 1), KEYCODE(cs3, code, 2), KEYCODE(cs4, code, 3), KEYCODE(cs5, code, 4), KEYCODE(cs6, code, 5), KEYCODE(cs7, code, 6), KEYCODE(cs8, code, 7))


KEYBT ENGLISH_LOCALE[] = {
        /* ` ~ */ KEYBTDECL(0x60, 0x7e, false, 41),
        /* 1 ! */ KEYBTDECL(0x31, 0x21, false, 2),
        /* 2 @ */ KEYBTDECL(0x32, 0x40, false, 3),
        /* 3 # */ KEYBTDECL(0x33, 0x23, false, 4),
        /* 4 $ */ KEYBTDECL(0x34, 0x24, false, 5),
        /* 5 % */ KEYBTDECL(0x35, 0x25, false, 6),
        /* 6 ^ */ KEYBTDECL(0x36, 0x88, false, 7),
        /* 7 & */ KEYBTDECL(0x37, 0x26, false, 8),
        /* 8 * */ KEYBTDECL(0x38, 0x2a, false, 9),
        /* 9 ( */ KEYBTDECL(0x39, 0x28, false, 10),
        /* 0 ) */ KEYBTDECL(0x30, 0x29, false, 11),
        /* - _ */ KEYBTDECL(0x2d, 0x5f, false, 12),
        /* = + */ KEYBTDECL(0x3d, 0x2b, false, 13),

        /* q Q */ KEYBTDECL(0x71, 0x51, true, 16),
        /* w W */ KEYBTDECL(0x77, 0x57, true, 17),
        /* e E */ KEYBTDECL(0x65, 0x45, true, 18),
        /* r R */ KEYBTDECL(0x72, 0x52, true, 19),
        /* t T */ KEYBTDECL(0x74, 0x54, true, 20),
        /* y Y */ KEYBTDECL(0x79, 0x59, true, 21),
        /* u U */ KEYBTDECL(0x75, 0x55, true, 22),
        /* i I */ KEYBTDECL(0x69, 0x49, true, 23),
        /* o O */ KEYBTDECL(0x6f, 0x4f, true, 24),
        /* p P */ KEYBTDECL(0x70, 0x50, true, 25),
        /* [ { */ KEYBTDECL(0x5b, 0x7b, false, 26),
        /* ] } */ KEYBTDECL(0x5d, 0x7d, false, 27),

        /* a A */ KEYBTDECL(0x61, 0x41, true, 30),
        /* s S */ KEYBTDECL(0x73, 0x53, true, 31),
        /* d D */ KEYBTDECL(0x64, 0x44, true, 32),
        /* f F */ KEYBTDECL(0x66, 0x46, true, 33),
        /* g G */ KEYBTDECL(0x67, 0x47, true, 34),
        /* h H */ KEYBTDECL(0x68, 0x48, true, 35),
        /* j J */ KEYBTDECL(0x6a, 0x4a, true, 36),
        /* k K */ KEYBTDECL(0x6b, 0x4b, true, 37),
        /* l L */ KEYBTDECL(0x6c, 0x4c, true, 38),
        /* ; : */ KEYBTDECL(0x3b, 0x3a, false, 39),
        /* ' " */ KEYBTDECL(0x27, 0x22, false, 40),
        /* \ | */ KEYBTDECL(0x5c, 0x7c, false, 43),

        /* z Z */ KEYBTDECL(0x7a, 0x5a, true, 44),
        /* x X */ KEYBTDECL(0x78, 0x58, true, 45),
        /* c C */ KEYBTDECL(0x63, 0x43, true, 46),
        /* v V */ KEYBTDECL(0x76, 0x56, true, 47),
        /* b B */ KEYBTDECL(0x62, 0x42, true, 48),
        /* n N */ KEYBTDECL(0x6e, 0x4e, true, 49),
        /* m M */ KEYBTDECL(0x6d, 0x4d, true, 50),
        /* , < */ KEYBTDECL(0x2c, 0x3c, false, 51),
        /* . > */ KEYBTDECL(0x2e, 0x3e, false, 52),
        /* / ? */ KEYBTDECL(0x2f, 0x5f, false, 53)};

KEYBT RUSSIAN_LOCALE [] =
{
        /* ё Ё */ KEYBTDECLEX2(0x451, 0x401, true, 41, XK_Cyrillic_io, XK_Cyrillic_IO),
        /* 1 ! */ KEYBTDECL(0x31, 0x21, false, 2),
        /* 2 " */ KEYBTDECL(0x32, 0x5c, false, 3),
        /* 3 № */ KEYBTDECLEX1(0x33, 0x2116, false, 4, 0x33, XK_numerosign),
        /* 4 ; */ KEYBTDECL(0x34, 0x3b, false, 5),
        /* 5 % */ KEYBTDECL(0x35, 0x25, false, 6),
        /* 6 : */ KEYBTDECL(0x36, 0x3a, false, 7),
        /* 7 ? */ KEYBTDECL(0x37, 0x3f, false, 8),
        /* 8 * */ KEYBTDECL(0x38, 0x2a, false, 9),
        /* 9 ( */ KEYBTDECL(0x39, 0x28, false, 10),
        /* 0 ) */ KEYBTDECL(0x30, 0x29, false, 11),
        /* - _ */ KEYBTDECL(0x2d, 0x4f, false, 12),
        /* = + */ KEYBTDECL(0x3d, 0x2b, false, 13),

        /* й Й */ KEYBTDECLEX2(0x439, 0x419, true, 16, XK_Cyrillic_shorti, XK_Cyrillic_SHORTI),
        /* ц Ц */ KEYBTDECLEX2(0x446, 0x426, true, 17, XK_Cyrillic_tse, XK_Cyrillic_TSE),
        /* у У */ KEYBTDECLEX2(0x443, 0x423, true, 18, XK_Cyrillic_u, XK_Cyrillic_U),
        /* к К */ KEYBTDECLEX2(0x43a, 0x41a, true, 19, XK_Cyrillic_ka, XK_Cyrillic_KA),
        /* е Е */ KEYBTDECLEX2(0x435, 0x415, true, 20, XK_Cyrillic_ie, XK_Cyrillic_IE),
        /* н Н */ KEYBTDECLEX2(0x43d, 0x41d, true, 21, XK_Cyrillic_en, XK_Cyrillic_EN),
        /* г Г */ KEYBTDECLEX2(0x433, 0x413, true, 22, XK_Cyrillic_ghe, XK_Cyrillic_GHE),
        /* ш Ш */ KEYBTDECLEX2(0x448, 0x428, true, 23, XK_Cyrillic_sha, XK_Cyrillic_SHA),
        /* щ Щ */ KEYBTDECLEX2(0x449, 0x429, true, 24, XK_Cyrillic_shcha, XK_Cyrillic_SHCHA),
        /* з З */ KEYBTDECLEX2(0x437, 0x417, true, 25, XK_Cyrillic_ze, XK_Cyrillic_ZE),
        /* х Х */ KEYBTDECLEX2(0x445, 0x425, true, 26, XK_Cyrillic_ha, XK_Cyrillic_HA),
        /* ъ Ъ */ KEYBTDECLEX2(0x44a, 0x42a, true, 27, XK_Cyrillic_hardsign, XK_Cyrillic_HARDSIGN),

        /* ф Ф */ KEYBTDECLEX2(0x444, 0x424, true, 30, XK_Cyrillic_ef, XK_Cyrillic_EF),
        /* ы Ы */ KEYBTDECLEX2(0x44b, 0x42b, true, 31, XK_Cyrillic_yeru, XK_Cyrillic_YERU),
        /* в В */ KEYBTDECLEX2(0x432, 0x412, true, 32, XK_Cyrillic_ve, XK_Cyrillic_VE),
        /* а А */ KEYBTDECLEX2(0x430, 0x410, true, 33, XK_Cyrillic_a, XK_Cyrillic_A),
        /* п П */ KEYBTDECLEX2(0x43f, 0x41f, true, 34, XK_Cyrillic_pe, XK_Cyrillic_PE),
        /* р Р */ KEYBTDECLEX2(0x440, 0x420, true, 35, XK_Cyrillic_er, XK_Cyrillic_ER),
        /* о О */ KEYBTDECLEX2(0x43e, 0x41e, true, 36, XK_Cyrillic_o, XK_Cyrillic_O),
        /* л Л */ KEYBTDECLEX2(0x43b, 0x41b, true, 37, XK_Cyrillic_el, XK_Cyrillic_EL),
        /* д Д */ KEYBTDECLEX2(0x434, 0x414, true, 38, XK_Cyrillic_de, XK_Cyrillic_DE),
        /* ж Ж */ KEYBTDECLEX2(0x436, 0x416, true, 39, XK_Cyrillic_zhe, XK_Cyrillic_ZHE),
        /* э Э */ KEYBTDECLEX2(0x44d, 0x42d, true, 40, XK_Cyrillic_e, XK_Cyrillic_E),
        /* \ / */ KEYBTDECL(0x5c, 0x2f, false, 43),

        /* я Я */ KEYBTDECLEX2(0x44f, 0x42f, true, 44, XK_Cyrillic_ya, XK_Cyrillic_YA),
        /* ч Ч */ KEYBTDECLEX2(0x447, 0x427, true, 45, XK_Cyrillic_che, XK_Cyrillic_CHE),
        /* с С */ KEYBTDECLEX2(0x441, 0x421, true, 46, XK_Cyrillic_es, XK_Cyrillic_ES),
        /* м М */ KEYBTDECLEX2(0x43c, 0x41c, true, 47, XK_Cyrillic_em, XK_Cyrillic_EM),
        /* и И */ KEYBTDECLEX2(0x438, 0x418, true, 48, XK_Cyrillic_i, XK_Cyrillic_I),
        /* т Т */ KEYBTDECLEX2(0x442, 0x422, true, 49, XK_Cyrillic_te, XK_Cyrillic_TE),
        /* ь Ь */ KEYBTDECLEX2(0x44c, 0x42c, true, 50, XK_Cyrillic_softsign, XK_Cyrillic_SOFTSIGN),
        /* б Б */ KEYBTDECLEX2(0x431, 0x411, true, 51, XK_Cyrillic_be, XK_Cyrillic_BE),
        /* ю Ю */ KEYBTDECLEX2(0x44e, 0x42e, true, 52, XK_Cyrillic_yu, XK_Cyrillic_YU),
        /* . , */ KEYBTDECL(0x2e, 0x2c, false, 53)};

KEYBT GERMAN_LOCALE[] = {
        /* ^ ? */ KEYBTDECL(0x5f, 0xb0, false, 41),
        /* 1 ! */ KEYBTDECL(0x31, 0x21, true, 2),
        /* 2 " */ KEYBTDECL(0x32, 0x22, true, 3),
        /* 3  */ KEYBTDECL(0x33, 0xa7, true, 4),
        /* 4 $ */ KEYBTDECL(0x34, 0x24, true, 5),
        /* 5 % */ KEYBTDECL(0x35, 0x25, true, 6),
        /* 6 & */ KEYBTDECL(0x36, 0x26, true, 7),
        /* 7 / */ KEYBTDECL(0x37, 0x2f, true, 8),
        /* 8 ( */ KEYBTDECL(0x38, 0x28, true, 9),
        /* 9 ) */ KEYBTDECL(0x39, 0x29, true, 10),
        /* 0 = */ KEYBTDECL(0x30, 0x3d, true, 11),
        /* ? ? */ KEYBTDECL(0xdf, 0x3f, true, 12),
        /* ` ? */ KEYBTDECL(0x60, 0xb4, true, 13),

        /* q Q */ KEYBTDECL(0x71, 0x51, true, 16),
        /* w W */ KEYBTDECL(0x77, 0x57, true, 17),
        /* e E */ KEYBTDECL(0x65, 0x45, true, 18),
        /* r R */ KEYBTDECL(0x72, 0x52, true, 19),
        /* t T */ KEYBTDECL(0x74, 0x54, true, 20),
        /* z Z */ KEYBTDECL(0x7a, 0x5a, true, 21),
        /* u U */ KEYBTDECL(0x75, 0x55, true, 22),
        /* i I */ KEYBTDECL(0x69, 0x49, true, 23),
        /* o O */ KEYBTDECL(0x6f, 0x4f, true, 24),
        /* p P */ KEYBTDECL(0x70, 0x50, true, 25),
        /* ? ? */ KEYBTDECL(0xfc, 0xdc, true, 26),
        /* + * */ KEYBTDECL(0x2b, 0x2a, true, 27),

        /* a A */ KEYBTDECL(0x61, 0x41, true, 30),
        /* s S */ KEYBTDECL(0x73, 0x53, true, 31),
        /* d D */ KEYBTDECL(0x64, 0x44, true, 32),
        /* f F */ KEYBTDECL(0x66, 0x46, true, 33),
        /* g G */ KEYBTDECL(0x67, 0x47, true, 34),
        /* h H */ KEYBTDECL(0x68, 0x48, true, 35),
        /* j J */ KEYBTDECL(0x6a, 0x4a, true, 36),
        /* k K */ KEYBTDECL(0x6b, 0x4b, true, 37),
        /* l L */ KEYBTDECL(0x6c, 0x4c, true, 38),
        /* ? ? */ KEYBTDECL(0xf6, 0xd6, true, 39),
        /* ? ? */ KEYBTDECL(0xe4, 0xc4, true, 40),
        /* ' # */ KEYBTDECL(0x27, 0x23, true, 43),

        /* y Y */ KEYBTDECL(0x79, 0x59, true, 44),
        /* x X */ KEYBTDECL(0x78, 0x58, true, 45),
        /* c C */ KEYBTDECL(0x63, 0x43, true, 46),
        /* v V */ KEYBTDECL(0x76, 0x56, true, 47),
        /* b B */ KEYBTDECL(0x62, 0x42, true, 48),
        /* n N */ KEYBTDECL(0x6e, 0x4e, true, 49),
        /* m M */ KEYBTDECL(0x6d, 0x4d, true, 50),
        /* , ; */ KEYBTDECL(0x2c, 0x3b, true, 51),
        /* . : */ KEYBTDECL(0x2e, 0x3a, true, 52),
        /* _ - */ KEYBTDECL(0x5f, 0x2d, true, 53)};

KEYBT FRENCH_LOCALE[] = {
    /* ? ? */ KEYBTDECL(0xb2, 0xb2, true, 41),
    /* & 1 */ KEYBTDECL(0x26, 0x31, true, 2),
    /* ? 2 */ KEYBTDECL(0xe9, 0x32, true, 3),
    /* " 3 */ KEYBTDECL(0x22, 0x33, true, 4),
    /* ' 4 */ KEYBTDECL(0x27, 0x34, true, 5),
    /* ( 5 */ KEYBTDECL(0x28, 0x35, true, 6),
    /* - 6 */ KEYBTDECL(0x2d, 0x36, true, 7),
    /* ? 7 */ KEYBTDECL(0xe8, 0x37, true, 8),
    /* _ 8 */ KEYBTDECL(0x5f, 0x38, true, 9),
    /* ? 9 */ KEYBTDECL(0xe7, 0x39, true, 10),
    /* ? 0 */ KEYBTDECL(0xe0, 0x30, true, 11),
    /* ) ? */ KEYBTDECL(0x29, 0xb0, true, 12),
    /* = + */ KEYBTDECL(0x3d, 0x2b, true, 13),

    /* a A */ KEYBTDECLEX6(0x61, 0x41, true, 16, 0x61, 0x41, 0xe2, 0xc2, 0xe4, 0xc4),
    /* z Z */ KEYBTDECL(0x7a, 0x5a, true, 17),
    /* e E */ KEYBTDECLEX6(0x65, 0x45, true, 18, 0x65, 0x45, 0xea, 0xca, 0xeb, 0xcb),
    /* r R */ KEYBTDECL(0x72, 0x52, true, 19),
    /* t T */ KEYBTDECL(0x74, 0x54, true, 20),
    /* y Y */ KEYBTDECLEX6(0x79, 0x59, true, 21, 0x79, 0x59, 0, 0, 0xff, 0),
    /* u U */ KEYBTDECLEX6(0x75, 0x55, true, 22, 0x75, 0x55, 0xfb, 0xdb, 0xfc, 0xdc),
    /* i I */ KEYBTDECLEX6(0x69, 0x49, true, 23, 0x69, 0x49, 0xee, 0xce, 0xef, 0xcf),
    /* o O */ KEYBTDECLEX6(0x6f, 0x4f, true, 24, 0x6f, 0x4f, 0xf4, 0xd4, 0xf6, 0xd6),
    /* p P */ KEYBTDECL(0x70, 0x50, true, 25),
    /* ^ ? */ KEYBT(0x5e, 0xa8, true, 1, 2, KEYCODE(0x5e, 26, 0), KEYCODE(0xa8, 26, 1)),
    /* $ ? } */ KEYBTDECL(0x24, 0xa3, true, 27),

    /* q Q */ KEYBTDECL(0x71, 0x51, true, 30),
    /* s S */ KEYBTDECL(0x73, 0x53, true, 31),
    /* d D */ KEYBTDECL(0x64, 0x44, true, 32),
    /* f F */ KEYBTDECL(0x66, 0x46, true, 33),
    /* g G */ KEYBTDECL(0x67, 0x47, true, 34),
    /* h H */ KEYBTDECL(0x68, 0x48, true, 35),
    /* j J */ KEYBTDECL(0x6a, 0x4a, true, 36),
    /* k K */ KEYBTDECL(0x6b, 0x4b, true, 37),
    /* l L */ KEYBTDECL(0x6c, 0x4c, true, 38),
    /* m M */ KEYBTDECL(0x6d, 0x4d, true, 39),
    /* ? % */ KEYBTDECL(0xf9, 0x25, true, 40),
    /* * ? */ KEYBTDECL(0x2a, 0xb5, true, 43),

    /* w W */ KEYBTDECL(0x77, 0x57, true, 44),
    /* x X */ KEYBTDECL(0x78, 0x58, true, 45),
    /* c C */ KEYBTDECL(0x63, 0x43, true, 46),
    /* v V */ KEYBTDECL(0x76, 0x56, true, 47),
    /* b B */ KEYBTDECL(0x62, 0x42, true, 48),
    /* n N */ KEYBTDECL(0x6e, 0x4e, true, 49),
    /* , ? */ KEYBTDECL(0x2c, 0x3f, true, 50),
    /* ; . */ KEYBTDECL(0x3b, 0x2e, true, 51),
    /* : / */ KEYBTDECL(0x3a, 0x2f, true, 52),
    /* ! ? */ KEYBTDECL(0x21, 0xa7, true, 53)};


KEYBT SWISS_FRENCH_LOCALE[] = {
    /* § °*/    KEYBTDECL(0xa7, 0xb0, false, 41),
    /* 1 +*/    KEYBTDECL(0x31, 0x2b, false, 2),
    /* 2 "*/    KEYBTDECL(0x32, 0x22, false, 3),
    /* 3 **/    KEYBTDECL(0x33, 0x2a, false, 4),
    /* 4 ç*/    KEYBTDECL(0x34, 0xe7, false, 5),
    /* 5 %*/    KEYBTDECL(0x35, 0x25, false, 6),
    /* 6 &*/    KEYBTDECL(0x36, 0x26, false, 7),
    /* 7 /*/    KEYBTDECL(0x37, 0x2f, false, 8),
    /* 8 (*/    KEYBTDECL(0x38, 0x28, false, 9),
    /* 9 )*/    KEYBTDECL(0x39, 0x29, false, 10),
    /* 0 =*/    KEYBTDECL(0x30, 0x3d, false, 11),
    /* ' ?*/    KEYBTDECL(0x27, 0x5f, false, 12),
    /* ^ `*/    KEYBT(0x5e, 0x60, false, 1, 2, KEYCODE(0x5e, 13, 0), KEYCODE(0x60, 13, 1)),

    /* q Q*/    KEYBTDECL(0x71, 0x51, true, 16),
    /* w W*/    KEYBTDECL(0x77, 0x57, true, 17),
    /* e E*/    KEYBTDECLEX8(0x65, 0x45, true, 18, 0x65, 0x45, 0xea, 0xca, 0xe8, 0xc8, 0xeb, 0xcb),
    /* r R*/    KEYBTDECL(0x72, 0x52, true, 19),
    /* t T*/    KEYBTDECL(0x74, 0x54, true, 20),
    /* z Z*/    KEYBTDECL(0x7a, 0x5a, true, 21),
    /* u U*/    KEYBTDECLEX8(0x75, 0x55, true, 22, 0x75, 0x55, 0xfb, 0xdb, 0xf9, 0xd9, 0xfc, 0xdc),
    /* i I*/    KEYBTDECLEX8(0x69, 0x49, true, 23, 0x69, 0x49, 0xee, 0xce, 0xec, 0xcc, 0xef, 0xcf),
    /* o O*/    KEYBTDECLEX8(0x6f, 0x4f, true, 24, 0x6f, 0x4f, 0xf4, 0xd4, 0xf2, 0xd2, 0xf6, 0xd6),
    /* p P*/    KEYBTDECL(0x70, 0x50, true, 25),
    /* è ü*/    KEYBTDECL(0xe8, 0xfa, false, 26),
    /* ¨ !*/    KEYBT(0xa8, 0x21, false, 3, 0, KEYCODE(0xa8, 27, 0), KEYCODE(0x21, 27, 1)),

    /* a A*/    KEYBTDECLEX8(0x61, 0x41, true, 30, 0x61, 0x41, 0xe2, 0xc2, 0xe0, 0xc0, 0xe4, 0xc4),
    /* s S*/    KEYBTDECL(0x73, 0x53, true, 31),
    /* d D*/    KEYBTDECL(0x64, 0x44, true, 32),
    /* f F*/    KEYBTDECL(0x66, 0x46, true, 33),
    /* g G*/    KEYBTDECL(0x67, 0x47, true, 34),
    /* h H*/    KEYBTDECL(0x68, 0x48, true, 35),
    /* j J*/    KEYBTDECL(0x6a, 0x4a, true, 36),
    /* k K*/    KEYBTDECL(0x6b, 0x4b, true, 37),
    /* l L*/    KEYBTDECL(0x6c, 0x4c, true, 38),
    /* é ö*/    KEYBTDECL(0xe9, 0xf6, false, 39),
    /* à ä*/    KEYBTDECL(0xe0, 0xe4, false, 40),
    /* $ £*/    KEYBTDECL(0x24, 0xa3, false, 43),

    /* < >*/    KEYBTDECL(0x3c, 0x3e, false, 44),
    /* y Y*/    KEYBTDECLEX8(0x79, 0x59, true, 45, 0x79, 0x59, 0, 0, 0, 0, 0xff, 0),
    /* x X*/    KEYBTDECL(0x78, 0x58, true, 46),
    /* c C*/    KEYBTDECL(0x63, 0x43, true, 47),
    /* v V*/    KEYBTDECL(0x76, 0x56, true, 48),
    /* b B*/    KEYBTDECL(0x62, 0x42, true, 49),
    /* n N*/    KEYBTDECL(0x6e, 0x4e, true, 50),
    /* m M*/    KEYBTDECL(0x6d, 0x4d, true, 51),
    /* , ;*/    KEYBTDECL(0x2c, 0x3b, false, 52),
    /* . :*/    KEYBTDECL(0x2e, 0x3a, false, 53),
    /* - _*/    KEYBTDECL(0x2d, 0x5f, false, 54)};


void UBPlatformUtils::initializeKeyboardLayouts()
{
    nKeyboardLayouts = 5;
    keyboardLayouts = new UBKeyboardLocale*[nKeyboardLayouts];
    keyboardLayouts[0] = new UBKeyboardLocale(tr("English"), "en", "", new QIcon(":/images/flags/en.png"), ENGLISH_LOCALE);
    keyboardLayouts[1] = new UBKeyboardLocale(tr("Russian"), "ru", "", new QIcon(":/images/flags/ru.png"),RUSSIAN_LOCALE);
    keyboardLayouts[2] = new UBKeyboardLocale(tr("German"), "de", "", new QIcon(":/images/flags/de.png"), GERMAN_LOCALE);
    keyboardLayouts[3] = new UBKeyboardLocale(tr("French"), "fr", "", new QIcon(":/images/flags/fr.png"), FRENCH_LOCALE);
    keyboardLayouts[4] = new UBKeyboardLocale(tr("Swiss French"), "fr-CH", "", new QIcon(":/images/flags/fr.png"), SWISS_FRENCH_LOCALE);
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

    // Not used on Linux

    return qsRet;
}

void UBPlatformUtils::setFrontProcess()
{
    // not used in Linux
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
    QProcess oskProcess;

    if (show)
        oskProcess.startDetached("/usr/bin/env", {"onboard"});

    else
        /* Not exactly a great solution, but it isn't possible to just
         * close onboard through wmctrl or xdotool */
        oskProcess.startDetached("pkill", {"-3", "onboard"});
}
