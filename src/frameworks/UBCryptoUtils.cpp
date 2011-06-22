/*
 * UBCryptoUtils.cpp
 *
 *  Created on: 23 sept. 2009
 *      Author: Luc
 */

#include "UBCryptoUtils.h"

#include <openssl/aes.h>

#include "core/memcheck.h"

UBCryptoUtils* UBCryptoUtils::sInstance(0);
QString UBCryptoUtils::sAESKey("9ecHaspud9uD9ste5erAchehefrup3echej-caje6&thestawacuk=h#F3jet3aF");
QString UBCryptoUtils::sAESSalt("6f0083e0-a90c-11de-ac21-0002a5d5c51b");


UBCryptoUtils::UBCryptoUtils(QObject * pParent)
    : QObject(pParent)
{
    aesInit();
}


UBCryptoUtils::~UBCryptoUtils()
{
    // TODO UB 4.x aes destroy
}


QString UBCryptoUtils::symetricEncrypt(const QString& clear)
{
    QByteArray clearData = clear.toUtf8();

    int cipheredLength = clearData.length() + AES_BLOCK_SIZE;
    int paddingLength = 0;
    unsigned char *ciphertext = (unsigned char *)malloc(cipheredLength);

    if(!EVP_EncryptInit_ex(&mAesEncryptContext, NULL, NULL, NULL, NULL))
        return QString();

    if(!EVP_EncryptUpdate(&mAesEncryptContext, ciphertext, &cipheredLength, (unsigned char *)clearData.data(), clearData.length()))
        return QString();

    /* update ciphertext with the final remaining bytes */
    if(!EVP_EncryptFinal_ex(&mAesEncryptContext, ciphertext + cipheredLength, &paddingLength))
        return QString();

    QByteArray cipheredData((const char *)ciphertext, cipheredLength + paddingLength);

    free(ciphertext);

    return QString::fromAscii(cipheredData.toBase64());
}


QString UBCryptoUtils::symetricDecrypt(const QString& encrypted)
{
    QByteArray encryptedData = QByteArray::fromBase64(encrypted.toAscii());

    int encryptedLength = encryptedData.length();
    int paddingLength = 0;
    unsigned char *plaintext = (unsigned char *)malloc(encryptedLength);

    if(!EVP_DecryptInit_ex(&mAesDecryptContext, NULL, NULL, NULL, NULL))
        return QString();

    if(!EVP_DecryptUpdate(&mAesDecryptContext, plaintext, &encryptedLength, (const unsigned char *)encryptedData.data(), encryptedData.length()))
        return QString();

    if(!EVP_DecryptFinal_ex(&mAesDecryptContext, plaintext + encryptedLength, &paddingLength))
        return QString();

    int len = encryptedLength + paddingLength;
    QByteArray clearData((const char *)plaintext, len);

    free(plaintext);

    return QString::fromUtf8(clearData);;
}


void UBCryptoUtils::aesInit()
{
    int i, nrounds = 5;
    unsigned char key[32], iv[32];
    unsigned char *key_data = (unsigned char *)sAESKey.toAscii().data();
    int key_data_len = sAESKey.length();

    i = EVP_BytesToKey(EVP_aes_256_cbc(), EVP_sha1(), (unsigned char *)sAESSalt.toAscii().data(), key_data,
            key_data_len, nrounds, key, iv);

    if (i != 32)
    {
        qWarning() << QString("Key size is %1 bits - should be 256 bits").arg(i);
        return;
    }

    EVP_CIPHER_CTX_init(&mAesEncryptContext);
    EVP_EncryptInit_ex(&mAesEncryptContext, EVP_aes_256_cbc(), NULL, key, iv);
    EVP_CIPHER_CTX_init(&mAesDecryptContext);
    EVP_DecryptInit_ex(&mAesDecryptContext, EVP_aes_256_cbc(), NULL, key, iv);
}
