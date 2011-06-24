#include "UBAbstractPublisher.h"

#include <QtXml>

#include <openssl/aes.h>
#include <openssl/evp.h>

#include "core/UBSettings.h"
#include "core/UBApplication.h"

#include "network/UBNetworkAccessManager.h"
#include "network/UBServerXMLHttpRequest.h"

#include "core/memcheck.h"

UBAbstractPublisher::UBAbstractPublisher(QObject *parent)
    : QObject(parent)
    , mTokenRequest(0)
{
    // NOOP
}


void UBAbstractPublisher::authenticate()
{
    sendAuthenticationTokenRequest();
}


void UBAbstractPublisher::sendAuthenticationTokenRequest()
{
    QUrl publishingEndpoint = QUrl(UBSettings::settings()->publishingAuthenticationUrl);

    mTokenRequest = new UBServerXMLHttpRequest(UBNetworkAccessManager::defaultAccessManager());

    mTokenRequest->setVerbose(true);

    UBApplication::showMessage(tr("Contacting %1").arg(UBSettings::settings()->uniboardWebUrl->get().toString()), true);

    connect(mTokenRequest, SIGNAL(finished(bool, const QByteArray&)), this, SLOT(processAuthenticationTokenResponse(bool, const QByteArray&)));

    mTokenRequest->get(publishingEndpoint);
}


void UBAbstractPublisher::processAuthenticationTokenResponse(bool success, const QByteArray& payload)
{
    if (mTokenRequest)
        mTokenRequest->deleteLater();

    if (success)
    {
        /*
          <publishing-token>
            ...
            <token>iFzZIXhPxHTLhJeHWFF9BDVblMDWU546rpzoEEfxMSLrftMq444w4BuoisnNtRAjm6ht3hKUIHmMKA3xGN2Hlaof8tbYNLHmzf2R1dO439vnXFiHMPLBi7nFpSEPtBNJ</token>
            <uuid>26fea4c0-1319-012d-d0fc-001f5b3d920c</uuid>
            ...
          </publishing-token>
        */

        QDomDocument doc("publishing-token");
        doc.setContent(payload, false);

        QDomElement root = doc.documentElement();

        const QString token = root.firstChildElement("token").text();
        const QString uuid = root.firstChildElement("uuid").text();

        if (token.length() > 0 && uuid.length() > 0)
        {
            const QByteArray encrypted = encrypt(token);
            QString encryptedBase64 = QString::fromAscii(encrypted.toBase64());

             UBApplication::showMessage(tr("Found %1").arg(UBSettings::settings()->uniboardWebUrl->get().toString()), false);

            emit authenticated(QUuid(uuid), encryptedBase64);
        }
        else
        {
            qWarning() << "Authentication failed" << QString::fromUtf8(payload);

            emit authenticationFailure();

            UBApplication::showMessage(tr("Cannot Authenticate with %1").arg(UBSettings::settings()->uniboardWebUrl->get().toString()));
        }
    }
    else
    {
        qWarning() << "Authentication failed" << QString::fromUtf8(payload);

        emit authenticationFailure();

        UBApplication::showMessage(tr("Cannot Authenticate with %1").arg(UBSettings::settings()->uniboardWebUrl->get().toString()));
    }
}

QByteArray UBAbstractPublisher::encrypt(const QString& clear)
{
    static const char *key = "9ecHaspud9uD9ste5erAchehefrup3ec";

    EVP_CIPHER_CTX aesEncryptContext;

    EVP_CIPHER_CTX_init(&aesEncryptContext);

    EVP_EncryptInit_ex(&aesEncryptContext, EVP_aes_256_ecb(), NULL, (const unsigned char*)key, NULL);

    QByteArray clearData = clear.toUtf8();

    int cipheredLength = clearData.length() + AES_BLOCK_SIZE;
    int paddingLength = 0;

    unsigned char *cipherText = (unsigned char *)malloc(cipheredLength);

    if(!EVP_EncryptInit_ex(&aesEncryptContext, NULL, NULL, NULL, NULL))
        return QByteArray();

    if(!EVP_EncryptUpdate(&aesEncryptContext, cipherText, &cipheredLength, (unsigned char *)clearData.data(), clearData.length()))
        return QByteArray();

    /* update ciphertext with the final remaining bytes */
    if(!EVP_EncryptFinal_ex(&aesEncryptContext, cipherText + cipheredLength, &paddingLength))
        return QByteArray();

    QByteArray cipheredData((const char *)cipherText, cipheredLength + paddingLength);

    free(cipherText);
    EVP_CIPHER_CTX_cleanup(&aesEncryptContext);

    return cipheredData;

}


