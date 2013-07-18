#ifndef UBTEXTTOOLS_H
#define UBTEXTTOOLS_H

#include <QString>

class UBTextTools{
public:
    UBTextTools(){}
    virtual ~UBTextTools(){}

    static QString cleanHtmlCData(const QString& _html);
    static QString cleanHtml(const QString& _html);
};

#endif // UBTEXTTOOLS_H
