#ifndef UBCFFSUBSETADAPTOR_H
#define UBCFFSUBSETADAPTOR_H

#include <QtXml>
#include <QString>

class UBDocumentProxy;

class UBCFFSubsetAdaptor
{
public:
    UBCFFSubsetAdaptor();

    static void ConvertCFFFileToUbz(QString &cffSourceFolder, QString &destinationFolder);

private:
    class UBCFFSubsetReader
    {
    public:
        UBCFFSubsetReader(UBDocumentProxy *proxy);

        QXmlStreamReader mReader;
        UBDocumentProxy *mProxy;
    };
};

#endif // UBCFFSUBSETADAPTOR_H
