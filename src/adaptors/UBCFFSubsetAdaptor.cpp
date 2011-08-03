#include "UBCFFSubsetAdaptor.h"
#include "document/UBDocumentProxy.h"

UBCFFSubsetAdaptor::UBCFFSubsetAdaptor()
{
}

void UBCFFSubsetAdaptor::ConvertCFFFileToUbz(QString &cffSourceFolder, QString &destinationFolder)
{
    //TODO create document proxy for destination file
    // create document proxy with destination folder
    // fill document proxy metadata
    // create persistance manager to save data using proxy
    // create UBCFFSubsetReader and make it parse cffSourceFolder
}

UBCFFSubsetAdaptor::UBCFFSubsetReader::UBCFFSubsetReader(UBDocumentProxy *proxy):
    mReader(), mProxy(proxy)
{
    //TODO parse
}

