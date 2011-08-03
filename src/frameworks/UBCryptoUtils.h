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

#ifndef UBCRYPTOUTILS_H_
#define UBCRYPTOUTILS_H_

#include <QtCore>
#include <openssl/evp.h>

#include "core/UBApplication.h"


class UBCryptoUtils : public QObject
{
    Q_OBJECT;

    public:

        static UBCryptoUtils* instance()
        {
            if(!sInstance)
                sInstance = new UBCryptoUtils(UBApplication::staticMemoryCleaner);

            return sInstance;
        }

        QString symetricEncrypt(const QString& clear);
        QString symetricDecrypt(const QString& encrypted);

    private:

        UBCryptoUtils(QObject * pParent = 0);
        virtual ~UBCryptoUtils();

        static UBCryptoUtils* sInstance;
        static QString sAESKey;
        static QString sAESSalt;

        void aesInit();

        EVP_CIPHER_CTX mAesEncryptContext;
        EVP_CIPHER_CTX mAesDecryptContext;

};

#endif /* UBCRYPTOUTILS_H_ */
