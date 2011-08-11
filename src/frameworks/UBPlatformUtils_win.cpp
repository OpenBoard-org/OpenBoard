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
#include <QProcess>
#include <windows.h>
#include <shellapi.h>

#include "core/memcheck.h"

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

QString UBPlatformUtils::preferredTranslation()
{
    QString localPreferredLanguage = preferredLanguage();

    QString qmPath = applicationResourcesDirectory() + "/" + "i18n" + "/" + QString("sankore_") + localPreferredLanguage + ".qm";

    qDebug() << "Looking for translation:" << qmPath;

    return qmPath;
}

QString UBPlatformUtils::preferredLanguage()
{
    return QLocale::system().name();
}

//void UBPlatformUtils::showVirtualKeyboard()
//{
//    ShellExecute(NULL,NULL,TEXT("tabtip.exe"), NULL, NULL, SW_SHOWNORMAL);
//}

void UBPlatformUtils::runInstaller(const QString &installerFilePath)
{
    QProcess process;

    QString escaped = QString("\"") + installerFilePath + QString("\" /SILENT");

    bool success = process.startDetached(escaped);
    if (!success)
    {
        qWarning() << "Running '" << installerFilePath << "' failed (error=" << process.error() << ")";
        QString verb = "runas";
        ::ShellExecute(NULL, verb.utf16(), installerFilePath.utf16(), NULL, NULL, SW_HIDE);
    }
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
{
	long exStyle = (nonAcivable) ? GetWindowLong(widget->winId(), GWL_EXSTYLE) | WS_EX_NOACTIVATE
		: GetWindowLong(widget->winId(), GWL_EXSTYLE) & ~WS_EX_NOACTIVATE;

	SetWindowLong(widget->winId(), GWL_EXSTYLE, exStyle);
}



const KEYBT RUSSIAN_LOCALE [] = 
{
	/* £ ³ */ KEYBT(0x451, 0x401),
	/* 1 ! */ KEYBT(0x31, 0x21),
	/* 2 " */ KEYBT(0x32, 0x5c),
	/* 3 ? */ KEYBT(0x33, 0x2116),
	/* 4 ; */ KEYBT(0x34, 0x3b),
	/* 5 % */ KEYBT(0x35, 0x25),
	/* 6 : */ KEYBT(0x36, 0x3a),
	/* 7 ? */ KEYBT(0x37, 0x3f),
	/* 8 * */ KEYBT(0x38, 0x2a),
	/* 9 ( */ KEYBT(0x39, 0x28),
	/* 0 ) */ KEYBT(0x30, 0x29),
	/* - _ */ KEYBT(0x2d, 0x4f),
	/* = + */ KEYBT(0x3d, 0x2b),

	/* Ê ê */ KEYBT(0x439, 0x419),
	/* Ã ã */ KEYBT(0x446, 0x426),
	/* Õ õ */ KEYBT(0x443, 0x423),
	/* Ë ë */ KEYBT(0x43a, 0x41a),
	/* Å å */ KEYBT(0x435, 0x415),
	/* Î î */ KEYBT(0x43d, 0x41d),
	/* Ç ç */ KEYBT(0x433, 0x413),
	/* Û û */ KEYBT(0x448, 0x428),
	/* Ý ý */ KEYBT(0x449, 0x429),
	/* Ú ú */ KEYBT(0x437, 0x417),
	/* È è */ KEYBT(0x445, 0x425),
	/* ß ÿ */ KEYBT(0x44a, 0x42a),

	/* Æ æ */ KEYBT(0x444, 0x424),
	/* Ù ù */ KEYBT(0x44b, 0x42b),
	/* × ÷ */ KEYBT(0x432, 0x412),
	/* Á á */ KEYBT(0x430, 0x410),
	/* Ð ð */ KEYBT(0x43f, 0x41f),
	/* Ò ò */ KEYBT(0x440, 0x420),
	/* Ï ï */ KEYBT(0x43e, 0x41e),
	/* Ì ì */ KEYBT(0x43b, 0x41b),
	/* Ä ä */ KEYBT(0x434, 0x414),
	/* Ö ö */ KEYBT(0x436, 0x416),
	/* Ü ü */ KEYBT(0x44d, 0x42d),
	/* \ / */ KEYBT(0x5c, 0x2f),

	/* Ñ ñ */ KEYBT(0x44f, 0x42f),
	/* Þ þ */ KEYBT(0x447, 0x427),
	/* Ó ó */ KEYBT(0x441, 0x421),
	/* Í í */ KEYBT(0x43c, 0x41c),
	/* É é */ KEYBT(0x438, 0x418),
	/* Ô ô */ KEYBT(0x442, 0x422),
	/* Ø ø */ KEYBT(0x44c, 0x42c),
	/* Â â */ KEYBT(0x431, 0x411),
	/* À à */ KEYBT(0x44e, 0x42e),
	/* . , */ KEYBT(0x2e, 0x2c)};

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


const KEYBT FRENCH_LOCALE[] = {
	/* ? ? */ KEYBT(0xb2, 0xb2),
	/* & 1 */ KEYBT(0x26, 0x31),
	/* ? 2 */ KEYBT(0xe9, 0x32),
	/* " 3 */ KEYBT(0x22, 0x33),
	/* ' 4 */ KEYBT(0x27, 0x34),
	/* ( 5 */ KEYBT(0x28, 0x35),
	/* - 6 */ KEYBT(0x2d, 0x36),
	/* ? 7 */ KEYBT(0xe8, 0x37),
	/* _ 8 */ KEYBT(0x5f, 0x38),
	/* ? 9 */ KEYBT(0xe7, 0x39),
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
	/* m M */ KEYBT(0x6d, 0x4d),
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
	/* 3 § */ KEYBT(0x33, 0xa7),
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
	for(int i=0; i<nKeyboardLayouts; i++)
		delete keyboardLayouts[i];
	delete [] keyboardLayouts;
	keyboardLayouts = NULL;
}