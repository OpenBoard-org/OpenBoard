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




#include "UBCryptoUtils.h"

#include <openssl/aes.h>

#include "core/memcheck.h"

UBCryptoUtils* UBCryptoUtils::sInstance(0);
QString UBCryptoUtils::sAESKey("9ecHaspud9uD9ste5erAchehefrup3echej-caje6&thestawacuk=h#F3jet3aF");
QString UBCryptoUtils::sAESSalt("6f0083e0-a90c-11de-ac21-0002a5d5c51b");

UBCryptoUtils* UBCryptoUtils::instance()
{
    if(!sInstance)
        sInstance = new UBCryptoUtils(UBApplication::staticMemoryCleaner);

    return sInstance;
}

void UBCryptoUtils::destroy()
{
    if (sInstance)
        delete sInstance;
    sInstance = NULL;
}


UBCryptoUtils::UBCryptoUtils(QObject * pParent)
    : QObject(pParent)
{
    aesInit();
}


UBCryptoUtils::~UBCryptoUtils()
{
    // TODO UB 4.x aes destroy
#if OPENSSL_VERSION_NUMBER >= 10100000L
    EVP_CIPHER_CTX_free(mAesEncryptContext);
    EVP_CIPHER_CTX_free(mAesDecryptContext);
#endif
}

void UBCryptoUtils::aesInit()
{
    int i, nrounds = 5;
    unsigned char key[32], iv[32];
    unsigned char *key_data = (unsigned char *)sAESKey.toLatin1().data();
    int key_data_len = sAESKey.length();

    i = EVP_BytesToKey(EVP_aes_256_cbc(), EVP_sha1(), (unsigned char *)sAESSalt.toLatin1().data(), key_data,
            key_data_len, nrounds, key, iv);

    if (i != 32)
    {
        qWarning() << QString("Key size is %1 bits - should be 256 bits").arg(i);
        return;
    }

#if OPENSSL_VERSION_NUMBER >= 10100000L
    mAesEncryptContext = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(mAesEncryptContext, EVP_aes_256_cbc(), NULL, key, iv);
    mAesDecryptContext = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(mAesDecryptContext, EVP_aes_256_cbc(), NULL, key, iv);
#else
    EVP_CIPHER_CTX_init(&mAesEncryptContext);
    EVP_EncryptInit_ex(&mAesEncryptContext, EVP_aes_256_cbc(), NULL, key, iv);
    EVP_CIPHER_CTX_init(&mAesDecryptContext);
    EVP_DecryptInit_ex(&mAesDecryptContext, EVP_aes_256_cbc(), NULL, key, iv);
#endif
}
