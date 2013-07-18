#include <QDebug>
#include "UBTextTools.h"

QString UBTextTools::cleanHtmlCData(const QString &_html){

    QString clean = "";


    for(int i = 0; i < _html.length(); i+=1){
        if(_html.at(i) != '\0')
            clean.append(_html.at(i));
    }
    return clean;
}

QString UBTextTools::cleanHtml(const QString& _html){
    const QString START_TAG = "<!doctype";
    const QString END_TAG = "</html";

    QString cleanSource = "";
    QString simplifiedHtml = _html;

    int start = simplifiedHtml.toLower().indexOf(START_TAG);
    int end = simplifiedHtml.toLower().indexOf(END_TAG) + END_TAG.size();

    cleanSource = simplifiedHtml.mid(start, end);

    return cleanSource;
}
