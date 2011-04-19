/*
 * UBCryptoUtils.h
 *
 *  Created on: 23 sept. 2009
 *      Author: Luc
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
