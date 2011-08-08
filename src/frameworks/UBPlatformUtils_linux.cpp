/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "UBPlatformUtils.h"

#include <QtGui>

#include <X11/Xlib.h>
#include <X11/keysym.h>


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

QString UBPlatformUtils::preferredTranslation()
{
    QString qmPath = applicationResourcesDirectory() + "/" + "i18n" + "/" + QString("sankore_") + preferredLanguage() + ".qm";
    return qmPath;
}

QString UBPlatformUtils::preferredLanguage()
{
    return QLocale::system().name();
}

void UBPlatformUtils::runInstaller(const QString &installerFilePath)
{
    // TODO UB 4.x - auto update & installer are deactivated on Linux
    Q_UNUSED(installerFilePath);

}

void UBPlatformUtils::bringPreviousProcessToFront()
{
    // Mac only
}

QString UBPlatformUtils::osUserLoginName()
{
    char *user = getenv("USER");
    return QString::fromAscii(user);
}

QString UBPlatformUtils::computerName()
{
    // TODO Linux implement me
    return "Not Implemented";
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

const KEYBT ENGLISH_LOCALE[] = {
        /* ` ~ */ KEYBT(0x60, 0x7e),
        /* 1 ! */ KEYBT(0x31, 0x21),
        /* 2 @ */ KEYBT(0x32, 0x40),
        /* 3 # */ KEYBT(0x33, 0x23),
        /* 4 $ */ KEYBT(0x34, 0x24),
        /* 5 % */ KEYBT(0x35, 0x25),
        /* 6 ^ */ KEYBT(0x36, 0x88),
        /* 7 & */ KEYBT(0x37, 0x26),
        /* 8 * */ KEYBT(0x38, 0x2a),
        /* 9 ( */ KEYBT(0x39, 0x28),
        /* 0 ) */ KEYBT(0x30, 0x29),
        /* - _ */ KEYBT(0x2d, 0x5f),
        /* = + */ KEYBT(0x3d, 0x2b),

        /* q Q */ KEYBT(0x71, 0x51),
        /* w W */ KEYBT(0x77, 0x57),
        /* e E */ KEYBT(0x65, 0x45),
        /* r R */ KEYBT(0x72, 0x52),
        /* t T */ KEYBT(0x74, 0x54),
        /* y Y */ KEYBT(0x79, 0x59),
        /* u U */ KEYBT(0x75, 0x55),
        /* i I */ KEYBT(0x69, 0x49),
        /* o O */ KEYBT(0x6f, 0x4f),
        /* p P */ KEYBT(0x70, 0x50),
        /* [ { */ KEYBT(0x5b, 0x7b),
        /* ] } */ KEYBT(0x5d, 0x7d),

        /* a A */ KEYBT(0x61, 0x41),
        /* s S */ KEYBT(0x73, 0x53),
        /* d D */ KEYBT(0x64, 0x44),
        /* f F */ KEYBT(0x66, 0x46),
        /* g G */ KEYBT(0x67, 0x47),
        /* h H */ KEYBT(0x68, 0x48),
        /* j J */ KEYBT(0x6a, 0x4a),
        /* k K */ KEYBT(0x6b, 0x4b),
        /* l L */ KEYBT(0x6c, 0x4c),
        /* ; : */ KEYBT(0x3b, 0x3a),
        /* ' " */ KEYBT(0x27, 0x22),
        /* \ | */ KEYBT(0x5c, 0x7c),

        /* z Z */ KEYBT(0x7a, 0x5a),
        /* x X */ KEYBT(0x78, 0x58),
        /* c C */ KEYBT(0x63, 0x43),
        /* v V */ KEYBT(0x76, 0x56),
        /* b B */ KEYBT(0x62, 0x42),
        /* n N */ KEYBT(0x6e, 0x4e),
        /* m M */ KEYBT(0x6d, 0x4d),
        /* , < */ KEYBT(0x2c, 0x3c),
        /* . > */ KEYBT(0x2e, 0x3e),
        /* / ? */ KEYBT(0x2f, 0x5f)};

const KEYBT RUSSIAN_LOCALE [] =
{
        /* ё Ё */ KEYBT(0x451, XK_Cyrillic_io, 0x401, XK_Cyrillic_IO),
        /* 1 ! */ KEYBT(0x31, 0x21),
        /* 2 " */ KEYBT(0x32, 0x5c),
        /* 3 № */ KEYBT(0x33, 0x33, 0x2116, XK_numerosign),
        /* 4 ; */ KEYBT(0x34, 0x3b),
        /* 5 % */ KEYBT(0x35, 0x25),
        /* 6 : */ KEYBT(0x36, 0x3a),
        /* 7 ? */ KEYBT(0x37, 0x3f),
        /* 8 * */ KEYBT(0x38, 0x2a),
        /* 9 ( */ KEYBT(0x39, 0x28),
        /* 0 ) */ KEYBT(0x30, 0x29),
        /* - _ */ KEYBT(0x2d, 0x4f),
        /* = + */ KEYBT(0x3d, 0x2b),

        /* й Й */ KEYBT(0x439, XK_Cyrillic_shorti, 0x419, XK_Cyrillic_SHORTI),
        /* ц Ц */ KEYBT(0x446, XK_Cyrillic_tse, 0x426, XK_Cyrillic_TSE),
        /* у У */ KEYBT(0x443, XK_Cyrillic_u, 0x423, XK_Cyrillic_U),
        /* к К */ KEYBT(0x43a, XK_Cyrillic_ka, 0x41a, XK_Cyrillic_KA),
        /* е Е */ KEYBT(0x435, XK_Cyrillic_ie, 0x415, XK_Cyrillic_IE),
        /* н Н */ KEYBT(0x43d, XK_Cyrillic_en, 0x41d, XK_Cyrillic_EN),
        /* г Г */ KEYBT(0x433, XK_Cyrillic_ghe, 0x413, XK_Cyrillic_GHE),
        /* ш Ш */ KEYBT(0x448, XK_Cyrillic_sha, 0x428, XK_Cyrillic_SHA),
        /* щ Щ */ KEYBT(0x449, XK_Cyrillic_shcha, 0x429, XK_Cyrillic_SHCHA),
        /* з З */ KEYBT(0x437, XK_Cyrillic_ze, 0x417, XK_Cyrillic_ZE),
        /* х Х */ KEYBT(0x445, XK_Cyrillic_ha, 0x425, XK_Cyrillic_HA),
        /* ъ Ъ */ KEYBT(0x44a, XK_Cyrillic_hardsign, 0x42a, XK_Cyrillic_HARDSIGN),

        /* ф Ф */ KEYBT(0x444, XK_Cyrillic_ef, 0x424, XK_Cyrillic_EF),
        /* ы Ы */ KEYBT(0x44b, XK_Cyrillic_yeru, 0x42b, XK_Cyrillic_YERU),
        /* в В */ KEYBT(0x432, XK_Cyrillic_ve, 0x412, XK_Cyrillic_VE),
        /* а А */ KEYBT(0x430, XK_Cyrillic_a, 0x410, XK_Cyrillic_A),
        /* п П */ KEYBT(0x43f, XK_Cyrillic_pe, 0x41f, XK_Cyrillic_PE),
        /* р Р */ KEYBT(0x440, XK_Cyrillic_er, 0x420, XK_Cyrillic_ER),
        /* о О */ KEYBT(0x43e, XK_Cyrillic_o, 0x41e, XK_Cyrillic_O),
        /* л Л */ KEYBT(0x43b, XK_Cyrillic_el, 0x41b, XK_Cyrillic_EL),
        /* д Д */ KEYBT(0x434, XK_Cyrillic_de, 0x414, XK_Cyrillic_DE),
        /* ж Ж */ KEYBT(0x436, XK_Cyrillic_zhe, 0x416, XK_Cyrillic_ZHE),
        /* э Э */ KEYBT(0x44d, XK_Cyrillic_e, 0x42d, XK_Cyrillic_E),
        /* \ / */ KEYBT(0x5c, 0x2f),

        /* я Я */ KEYBT(0x44f, XK_Cyrillic_ya, 0x42f, XK_Cyrillic_YA),
        /* ч Ч */ KEYBT(0x447, XK_Cyrillic_che, 0x427, XK_Cyrillic_CHE),
        /* с С */ KEYBT(0x441, XK_Cyrillic_es, 0x421, XK_Cyrillic_ES),
        /* м М */ KEYBT(0x43c, XK_Cyrillic_em, 0x41c, XK_Cyrillic_EM),
        /* и И */ KEYBT(0x438, XK_Cyrillic_i, 0x418, XK_Cyrillic_I),
        /* т Т */ KEYBT(0x442, XK_Cyrillic_te, 0x422, XK_Cyrillic_TE),
        /* ь Ь */ KEYBT(0x44c, XK_Cyrillic_softsign, 0x42c, XK_Cyrillic_SOFTSIGN),
        /* б Б */ KEYBT(0x431, XK_Cyrillic_be, 0x411, XK_Cyrillic_BE),
        /* ю Ю */ KEYBT(0x44e, XK_Cyrillic_yu, 0x42e, XK_Cyrillic_YU),
        /* . , */ KEYBT(0x2e, 0x2c)};


const KEYBT FRENCH_LOCALE[] = {
        /* ? ? */ KEYBT(0xb2, XK_twosuperior, 0xb2, XK_twosuperior),
        /* & 1 */ KEYBT(0x26, 0x31),
        /* ? 2 */ KEYBT(0xe9, 0x32),
        /* " 3 */ KEYBT(0x22, 0x33),
        /* ' 4 */ KEYBT(0x27, 0x34),
        /* ( 5 */ KEYBT(0x28, 0x35),
        /* - 6 */ KEYBT(0x2d, 0x36),
        /* ? 7 */ KEYBT(0xe8, XK_egrave, 0x37, 0x37),
        /* _ 8 */ KEYBT(0x5f, 0x38),
        /* ? 9 */ KEYBT(0xe7, XK_ccedilla, 0x39, 0x39),
        /* ? 0 */ KEYBT(0xe0, 0x29),
        /* ) ? */ KEYBT(0x29, 0xb0),
        /* = + */ KEYBT(0x3d, 0x2b),

        /* a A */ KEYBT(0x61, 0x41),
        /* z Z */ KEYBT(0x7a, 0x5a),
        /* e E */ KEYBT(0x65, 0x45),
        /* r R */ KEYBT(0x72, 0x52),
        /* t T */ KEYBT(0x74, 0x54),
        /* y Y */ KEYBT(0x79, 0x59),
        /* u U */ KEYBT(0x75, 0x55),
        /* i I */ KEYBT(0x69, 0x49),
        /* o O */ KEYBT(0x6f, 0x4f),
        /* p P */ KEYBT(0x70, 0x50),
        /* ^ ? */ KEYBT(0x5e, 0xa8),
        /* $ ? } */ KEYBT(0x24, 0xa3),

        /* q Q */ KEYBT(0x71, 0x51),
        /* s S */ KEYBT(0x73, 0x53),
        /* d D */ KEYBT(0x64, 0x44),
        /* f F */ KEYBT(0x66, 0x46),
        /* g G */ KEYBT(0x67, 0x47),
        /* h H */ KEYBT(0x68, 0x48),
        /* j J */ KEYBT(0x6a, 0x4a),
        /* k K */ KEYBT(0x6b, 0x4b),
        /* l L */ KEYBT(0x6c, 0x4c),
        /* m M */ KEYBT(0x6b, 0x4d),
        /* ? % */ KEYBT(0xf9, 0x25),
        /* * ? */ KEYBT(0x2a, 0xb5),

        /* w W */ KEYBT(0x77, 0x57),
        /* x X */ KEYBT(0x78, 0x58),
        /* c C */ KEYBT(0x63, 0x43),
        /* v V */ KEYBT(0x76, 0x56),
        /* b B */ KEYBT(0x62, 0x42),
        /* n N */ KEYBT(0x6e, 0x4e),
        /* , ? */ KEYBT(0x2c, 0x3f),
        /* ; . */ KEYBT(0x3b, 0x2e),
        /* : / */ KEYBT(0x3a, 0x2f),
        /* ! ? */ KEYBT(0x21, 0xa7)};


const KEYBT GERMAN_LOCALE[] = {
        /* ^ ? */ KEYBT(0x5f, 0xb0),
        /* 1 ! */ KEYBT(0x31, 0x21),
        /* 2 " */ KEYBT(0x32, 0x22),
        /* 3  */ KEYBT(0x33, 0xa7),
        /* 4 $ */ KEYBT(0x34, 0x24),
        /* 5 % */ KEYBT(0x35, 0x25),
        /* 6 & */ KEYBT(0x36, 0x26),
        /* 7 / */ KEYBT(0x37, 0x2f),
        /* 8 ( */ KEYBT(0x38, 0x28),
        /* 9 ) */ KEYBT(0x39, 0x29),
        /* 0 = */ KEYBT(0x30, 0x3d),
        /* ? ? */ KEYBT(0xdf, 0x3f),
        /* ` ? */ KEYBT(0x60, 0xb4),

        /* q Q */ KEYBT(0x71, 0x51),
        /* w W */ KEYBT(0x77, 0x57),
        /* e E */ KEYBT(0x65, 0x45),
        /* r R */ KEYBT(0x72, 0x52),
        /* t T */ KEYBT(0x74, 0x54),
        /* z Z */ KEYBT(0x7a, 0x5a),
        /* u U */ KEYBT(0x75, 0x55),
        /* i I */ KEYBT(0x69, 0x49),
        /* o O */ KEYBT(0x6f, 0x4f),
        /* p P */ KEYBT(0x70, 0x50),
        /* ? ? */ KEYBT(0xfc, 0xdc),
        /* + * */ KEYBT(0x2b, 0x2a),

        /* a A */ KEYBT(0x61, 0x41),
        /* s S */ KEYBT(0x73, 0x53),
        /* d D */ KEYBT(0x64, 0x44),
        /* f F */ KEYBT(0x66, 0x46),
        /* g G */ KEYBT(0x67, 0x47),
        /* h H */ KEYBT(0x68, 0x48),
        /* j J */ KEYBT(0x6a, 0x4a),
        /* k K */ KEYBT(0x6b, 0x4b),
        /* l L */ KEYBT(0x6c, 0x4c),
        /* ? ? */ KEYBT(0xf6, 0xd6),
        /* ? ? */ KEYBT(0xe4, 0xc4),
        /* ' # */ KEYBT(0x27, 0x23),

        /* y Y */ KEYBT(0x79, 0x59),
        /* x X */ KEYBT(0x78, 0x58),
        /* c C */ KEYBT(0x63, 0x43),
        /* v V */ KEYBT(0x76, 0x56),
        /* b B */ KEYBT(0x62, 0x42),
        /* n N */ KEYBT(0x6e, 0x4e),
        /* m M */ KEYBT(0x6d, 0x4d),
        /* , ; */ KEYBT(0x2c, 0x3b),
        /* . : */ KEYBT(0x2e, 0x3a),
        /* _ - */ KEYBT(0x5f, 0x2d)};


void UBPlatformUtils::initializeKeyboardLayouts()
{
    nKeyboardLayouts = 4;
    keyboardLayouts = new UBKeyboardLocale*[nKeyboardLayouts];
    keyboardLayouts[0] = new UBKeyboardLocale("English", "en", "", new QIcon(":/images/flags/en.png"), ENGLISH_LOCALE);
    keyboardLayouts[1] = new UBKeyboardLocale("Russian", "ru", "", new QIcon(":/images/flags/ru.png"),RUSSIAN_LOCALE);
    keyboardLayouts[2] = new UBKeyboardLocale("French", "fr", "", new QIcon(":/images/flags/fr.png"), FRENCH_LOCALE);
    keyboardLayouts[3] = new UBKeyboardLocale("German", "de", "", new QIcon(":/images/flags/de.png"), GERMAN_LOCALE);
}

void UBPlatformUtils::destroyKeyboardLayouts()
{
	for(int i=0; i<4; i++)
		delete keyboardLayouts[i];
	delete [] keyboardLayouts;
	keyboardLayouts = NULL;
}