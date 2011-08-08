#ifndef UBCFFSUBSETADAPTOR_H
#define UBCFFSUBSETADAPTOR_H

#include <QtXml>
#include <QString>
#include <QStack>

class UBDocumentProxy;
class UBGraphicsScene;
class QSvgGenerator;

class UBCFFSubsetAdaptor
{
public:
    UBCFFSubsetAdaptor();

    static bool ConvertCFFFileToUbz(QString &cffSourceFile, UBDocumentProxy* pDocument);

private:
    class UBCFFSubsetReader
    {
        //xml parse states definition
        enum
        {
            NONE,
            IWB,
            SVG,
            PAGESET,
            PAGE,
            TEXTAREA,
            TSPAN
        };

    public:
        UBCFFSubsetReader(UBDocumentProxy *proxy, QByteArray &content);

        QXmlStreamReader mReader;
        UBDocumentProxy *mProxy;

        bool parse();

    private:
        QString mTempFilePath;
        UBGraphicsScene *mCurrentScene;
        QString mIndent;
        QRectF mViewBox;
        QSize mSize;

        //methods to store current xml parse state
        int PopState();
        void PushState(int state);

        //elements parsing methods
        bool parseDoc();

        bool parseCurrentElementStart();
        bool parseCurrentElementCharacters();
        bool parseCurrentElementEnd();

        bool parseIwb();
        bool parseIwbMeta();
        bool parseSvg();
        bool parseRect();
        bool parseEllipse();
        bool parseTextArea();
        bool parsePolygon();
        bool parsePage();
        bool parsePageSet();
        bool parseIwbElementRef();

        bool createNewScene();
        bool persistCurrentScene();

        QStack<int> stateStack;
        int currentState;

        //helper methods
        QColor colorFromString(const QString& clrString);
        QTransform transformFromString(const QString trString);
        bool getViewBoxDimenstions(const QString& viewBox);
        QSvgGenerator* createSvgGenerator();
        bool getTempFileName();
    };
};

#endif // UBCFFSUBSETADAPTOR_H
