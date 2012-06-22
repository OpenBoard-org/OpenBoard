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

#ifndef UBPLATFORMUTILS_H_
#define UBPLATFORMUTILS_H_

#include <QtCore>
#include <QIcon>

class QMainWindow;

#define SYMBOL_KEYS_COUNT 47

struct KEYBT
{
	const QChar symbol1;
	const int code1;
	const QChar symbol2;
    const int code2;

	KEYBT(unsigned int _symbol1,
		unsigned int _symbol2):
        symbol1(_symbol1),
        code1(_symbol1),
        symbol2(_symbol2),
        code2(_symbol2){}


    KEYBT(QChar _symbol1,
		int _code1,
		QChar _symbol2,
		int _code2):
			symbol1(_symbol1),
            code1(_code1),
            symbol2(_symbol2),
            code2(_code2){}
    };

	class UBKeyboardLocale
	{
	public:
		UBKeyboardLocale(const QString& _fullName,
			const QString& _name,
			const QString& _id,
			QIcon* _icon,
			KEYBT** _symbols)
			:fullName(_fullName),name(_name), id(_id), icon(_icon),
                        constSymbols(NULL), varSymbols(_symbols)
		{}
		UBKeyboardLocale(const QString& _fullName,
			const QString& _name,
			const QString& _id,
			QIcon* _icon,
			const KEYBT _symbols[])
			:fullName(_fullName),name(_name),  id(_id), icon(_icon),
                        constSymbols(_symbols), varSymbols(NULL)
		{}

		~UBKeyboardLocale();

		const QString fullName;
		const QString name;
		const QString id;
		QIcon* icon;
		const KEYBT* operator[] (int index) const
		{
			return (varSymbols==NULL)? constSymbols + index : varSymbols[index];
		}
	private:
		const KEYBT* constSymbols;
		KEYBT** varSymbols;
	};


class UBPlatformUtils
{
    private:
        UBPlatformUtils();
        virtual ~UBPlatformUtils();

        static void initializeKeyboardLayouts();
        static void destroyKeyboardLayouts();
        static int nKeyboardLayouts;
        static UBKeyboardLocale** keyboardLayouts;

    public:
        static void init();
        static void destroy();
        static QString applicationResourcesDirectory();
        static void hideFile(const QString &filePath);
        static void setFileType(const QString &filePath, unsigned long fileType);
        static void fadeDisplayOut();
        static void fadeDisplayIn();
        static QString translationPath(QString pFilePrefix, QString pLanguage);
        static QString systemLanguage();
        static bool hasVirtualKeyboard();
        //static void showVirtualKeyboard();
        static void runInstaller(const QString &installerFilePath);
        static void bringPreviousProcessToFront();
        static QString osUserLoginName();
        static void setDesktopMode(bool desktop);
        static void setWindowNonActivableFlag(QWidget* widget, bool nonAcivable);
        static QString computerName();
        static UBKeyboardLocale** getKeyboardLayouts(int& nCount);
        static QString urlFromClipboard();
        static QStringList availableTranslations();
};



#endif /* UBPLATFORMUTILS_H_ */
