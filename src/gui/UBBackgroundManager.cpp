#include "UBBackgroundManager.h"

#include "core/UBSettings.h"
#include "frameworks/UBPlatformUtils.h"
#include "gui/UBBackgroundRuling.h"

#include <QDomDocument>
#include <QSvgGenerator>


UBBackgroundManager::UBBackgroundManager(QObject *parent)
    : QObject{parent}
{
    scan();
}

void UBBackgroundManager::scan()
{
    // scan application and user directories for background rules
    QString appDir = UBPlatformUtils::applicationTemplateDirectory() + "/background";
    QString usrDir = UBSettings::userDataDirectory() + "/template/background";

    scan(appDir);
    scan(usrDir);
}

void UBBackgroundManager::addBackground(UBBackgroundRuling &background)
{
    if (background.isValid() && !this->background(background.uuid()))
    {
        mBackgrounds.append(background);
    }
    else
    {
        qWarning() << "Invalid or duplicate background ruling" << background.description("en");
    }
}

const QList<UBBackgroundRuling> &UBBackgroundManager::backgrounds() const
{
    return mBackgrounds;
}

QAction *UBBackgroundManager::backgroundAction(const UBBackgroundRuling &background, bool dark)
{
    QAction* action = new QAction{this};
    action->setIcon(createButtonIcon(background, dark));
    QString lang = QLocale().name().left(2);
    QString descr = background.description(lang);
    action->setToolTip(descr);
    action->setProperty("uuid", background.uuid());
    action->setCheckable(true);
    return action;
}

void UBBackgroundManager::updateAction(QAction *action, bool dark) const
{
    const auto uuid = action->property("uuid").toUuid();
    const auto bg = background(uuid);

    if (bg)
    {
        action->setIcon(createButtonIcon(*bg, dark));
    }
}

const UBBackgroundRuling* UBBackgroundManager::background(const QUuid &uuid) const
{
    for (auto& background : qAsConst(mBackgrounds))
    {
        if (background.uuid() == uuid)
        {
            return &background;
        }
    }

    return nullptr;
}

const UBBackgroundRuling* UBBackgroundManager::guessBackground(bool crossed, bool ruled, bool intermediateLines) const
{
    for (auto& background : qAsConst(mBackgrounds))
    {
        if (background.isCrossed() == crossed
                && background.isRuled() == ruled
                && background.hasIntermediateLines() == intermediateLines)
        {
            return &background;
        }
    }

    return nullptr;
}

void UBBackgroundManager::scan(const QString &dirname)
{
    const QDir dir{dirname};

    if (dir.exists())
    {
        const auto list = dir.entryInfoList({"*.xml"}, QDir::Files, QDir::Name);

        for (const auto& entry : list)
        {
            UBBackgroundRuling bg;
            QFile file{entry.absoluteFilePath()};

            if (!file.open(QFile::ReadOnly))
            {
                continue;
            }

            QXmlStreamReader reader(&file);

            if (reader.readNextStartElement())
            {
                bg.parseXml(reader);
            }

            if (bg.isValid())
            {
                mBackgrounds.append(bg);
            }
            else
            {
                qWarning() << "Error reading background definitions from" << file;
            }
        }
    }
}

QByteArray UBBackgroundManager::renderToSvg(const UBBackgroundRuling &background, bool dark) const
{
    QSvgGenerator generator;
    QBuffer buffer;
    generator.setOutputDevice(&buffer);
    generator.setSize(QSize(93, 75));
    generator.setViewBox(QRect(0, 0, 93, 75));

    QPainter painter;

    painter.begin(&generator);
    background.draw(&painter, {0, 0, 93, 75}, 0., {0, 0, 193, 75}, dark);
    painter.end();

    return buffer.buffer();
}

QPixmap UBBackgroundManager::createButtonPixmap(const UBBackgroundRuling &background, bool dark, bool on) const
{
    QString filename{"bgButtonTemplate"};

    filename += dark ? "Dark" : "Light";
    filename += on ? "On" : "Off";
    filename += ".svg";

    QFile templateFile(UBPlatformUtils::applicationTemplateDirectory() + "/background/" + filename);

    if (!templateFile.open(QFile::ReadOnly))
    {
        qWarning() << "Cannot find template file" << templateFile;
        return {};
    }

    QDomDocument doc;

    if (!doc.setContent(&templateFile, true))
    {
        qWarning() << "Cannot load template file" << templateFile;
        return {};
    }

    // find <g> element with id="placeholder"
    auto plist = doc.documentElement().elementsByTagName("g");

    QDomElement placeholder;

    for (int i = 0; i < plist.size(); ++i)
    {
        QDomElement item = plist.at(i).toElement();

        if (item.attribute("id") == "placeholder")
        {
            placeholder = item;
            break;
        }
    }

    // now render background to SVG
    QByteArray bgSvg = renderToSvg(background, dark);
    QDomDocument bgDoc;

    if (!bgDoc.setContent(bgSvg))
    {
        qWarning() << "Cannot load rendered ruling" << templateFile;
        return {};
    }

    // find first <g> element
    auto bgElement = bgDoc.documentElement().firstChildElement("g");

    // replace element in original document
    auto bg = doc.importNode(bgElement, true);
    placeholder.parentNode().replaceChild(bg, placeholder);

    QByteArray replaced = doc.toByteArray(2);

    // render SVG to pixmap by using an QImageReader
    QBuffer buffer{&replaced};
    QImageReader reader(&buffer, "svg");
    QPixmap pixmap = QPixmap::fromImageReader(&reader);

    return pixmap;
}

QIcon UBBackgroundManager::createButtonIcon(const UBBackgroundRuling &background, bool dark) const
{
    QIcon icon;

    QPixmap pix = createButtonPixmap(background, dark, false);
    icon.addPixmap(pix, QIcon::Normal, QIcon::Off);

    pix = createButtonPixmap(background, dark, true);
    icon.addPixmap(pix, QIcon::Normal, QIcon::On);

    return icon;
}
