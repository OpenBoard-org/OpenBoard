/*
 * Copyright (C) 2015-2024 Département de l'Instruction Publique (DIP-SEM)
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



#include "UBBackgroundRuling.h"

#include "core/UBSettings.h"

#include <QColor>
#include <QDebug>
#include <QLineF>
#include <QPainter>
#include <QPointF>
#include <QRectF>

// namespace and element names
static constexpr char const* namespaceURI{"http://openboard.org/template/background/"};
static constexpr char const* namespacePrefix{"bg"};

static constexpr char const* attributeCrossed{"crossed"};
static constexpr char const* attributeIntermediateLines{"intermediateLines"};
static constexpr char const* attributeRuled{"ruled"};
static constexpr char const* attributeXmlLang{"xml:lang"};

static constexpr char const* elementAngle{"angle"};
static constexpr char const* elementAttributes{"attributes"};
static constexpr char const* elementBackground{"background"};
static constexpr char const* elementBorder{"border"};
static constexpr char const* elementColor{"color"};
static constexpr char const* elementDefaultColor{"defaultColor"};
static constexpr char const* elementDescription{"description"};
static constexpr char const* elementLeft{"left"};
static constexpr char const* elementLinegroup{"linegroup"};
static constexpr char const* elementLine{"line"};
static constexpr char const* elementOffset{"offset"};
static constexpr char const* elementOnDark{"onDark"};
static constexpr char const* elementOnLight{"onLight"};
static constexpr char const* elementOrigin{"origin"};
static constexpr char const* elementRight{"right"};
static constexpr char const* elementSpacing{"spacing"};
static constexpr char const* elementUuid{"uuid"};
static constexpr char const* elementWidth{"width"};

static constexpr char const* originBottomleft{"bottomleft"};
static constexpr char const* originBottomright{"bottomright"};
static constexpr char const* originCenter{"center"};
static constexpr char const* originTopleft{"topleft"};
static constexpr char const* originTopright{"topright"};

static constexpr char const* valueTrue{"true"};


// helper functions
static bool getStringValue(QXmlStreamReader& reader, const QString& name, QString& value)
{
    if (reader.isStartElement() && reader.namespaceUri().toString() == namespaceURI && reader.name() == name)
    {
        const auto text = reader.readElementText();

        if (!reader.hasError())
        {
            value = text;
            return true;
        }
    }

    return false;
}

static bool getDecimalValue(QXmlStreamReader& reader, const QString& name, double& value)
{
    QString text;

    if (getStringValue(reader, name, text))
    {
        bool ok{true};
        value = text.toDouble(&ok);
        return ok;
    }

    return false;
}

static bool getColorValue(QXmlStreamReader& reader, const QString& name, QColor& color)
{
    QString colorValue;

    if (getStringValue(reader, name, colorValue))
    {
        if (colorValue.startsWith("#"))
        {
            color = colorValue;
            return true;
        }
        else
        {
            bool ok;
            const auto alpha = colorValue.toDouble(&ok);

            if (ok)
            {
                color.setAlphaF(color.alphaF() * alpha);
                return true;
            }
            else
            {
                // try to interpret as color name
                color = colorValue;
                return color.isValid();
            }
        }
    }

    return false;
}

static double distance(const QLineF& line, const QPointF& point)
{
    if (line.isNull())
    {
        return QLineF(line.p1(), point).length();
    }

    const auto a = line.y1() - line.y2();
    const auto b = line.x2() - line.x1();
    const auto c = line.x1() * line.y2() - line.x2() * line.y1();

    return (a * point.x() + b * point.y() + c) / std::sqrt(a * a + b * b);
}

// Liang—Barsky line-clipping algorithm
// inspired by https://en.wikipedia.org/wiki/Liang%E2%80%93Barsky_algorithm
bool clip(const QRectF& rect, QLineF& line)
{
    // defining variables
    const auto p1 = -line.dx();
    const auto p3 = -line.dy();

    const auto q1 = line.x1() - rect.left();
    const auto q2 = rect.right() - line.x1();
    const auto q3 = line.y1() - rect.top();
    const auto q4 = rect.bottom() - line.y1();

    qreal posarr[5]{1};
    qreal negarr[5]{0};
    int posind = 1;
    int negind = 1;

    if ((p1 == 0 && q1 < 0) || (p1 == 0 && q2 < 0) || (p3 == 0 && q3 < 0) || (p3 == 0 && q4 < 0))
    {
        return false;
    }

    if (p1 != 0)
    {
        const auto r1 = q1 / p1;
        const auto r2 = q2 / -p1;

        if (p1 < 0)
        {
            negarr[negind++] = r1; // for negative p1, add it to negative array
            posarr[posind++] = r2; // and add r2 to positive array
        }
        else
        {
            negarr[negind++] = r2;
            posarr[posind++] = r1;
        }
    }

    if (p3 != 0)
    {
        const auto r3 = q3 / p3;
        const auto r4 = q4 / -p3;

        if (p3 < 0)
        {
            negarr[negind++] = r3;
            posarr[posind++] = r4;
        }
        else
        {
            negarr[negind++] = r4;
            posarr[posind++] = r3;
        }
    }

    const auto rn1 = *std::max_element(negarr, negarr + negind); // maximum of negative array
    const auto rn2 = *std::min_element(posarr, posarr + posind); // minimum of positive array

    if (rn1 > rn2)
    {
        // reject
        return false;
    }

    // computing new points
    const auto xn1 = line.x1() - p1 * rn1;
    const auto yn1 = line.y1() - p3 * rn1;

    const auto xn2 = line.x1() - p1 * rn2;
    const auto yn2 = line.y1() - p3 * rn2;

    line.setLine(xn1, yn1, xn2, yn2);

    return true;
}


UBBackgroundRuling::UBBackgroundRuling()
{
}

bool UBBackgroundRuling::operator==(const UBBackgroundRuling& other) const
{
    return uuid() == other.uuid();
}

bool UBBackgroundRuling::parseXml(QXmlStreamReader& reader, bool userProvided)
{
    mRules = std::shared_ptr<Rules>(Rules::fromXml(reader));
    mUserProvided = userProvided;
    return mRules && !mRules->hasError();
}

bool UBBackgroundRuling::toXml(QXmlStreamWriter& writer) const
{
    if (mRules)
    {
        mRules->toXml(writer);
        return true;
    }

    return false;
}

bool UBBackgroundRuling::isValid() const
{
    return mRules && !mRules->hasError();
}

QUuid UBBackgroundRuling::uuid() const
{
    if (!mRules)
    {
        return {};
    }

    return mRules->uuid();
}

QString UBBackgroundRuling::description(const QString& languagecode) const
{
    if (!mRules)
    {
        return {};
    }

    return mRules->description(languagecode);
}

bool UBBackgroundRuling::isCrossed() const
{
    if (!mRules)
    {
        return false;
    }

    return mRules->isCrossed();
}

bool UBBackgroundRuling::isRuled() const
{
    if (!mRules)
    {
        return false;
    }

    return mRules->isRuled();
}

bool UBBackgroundRuling::hasIntermediateLines() const
{
    if (!mRules)
    {
        return false;
    }

    return mRules->hasIntermediateLines();
}

bool UBBackgroundRuling::isUserProvided() const
{
    return mUserProvided;
}

void UBBackgroundRuling::draw(QPainter* painter, const QRectF& rect, double gridSize, const QRectF& nominalScene,
                              bool onDark) const
{
    if (!mRules)
    {
        return;
    }

    if (gridSize == 0)
    {
        // guess reasonable grid size for buttons
        // at least two spacings on rect, else 12.
        double spacing{0};

        if (!mRules->linegroups().empty())
        {
            spacing = mRules->linegroups().at(0).spacing();
        }

        if (spacing > 0)
        {
            // factor 10: spacing is in mm
            // factor 2.5: at least 2.5 repetitions in button
            // limit 12: reasonable limit for small spacings, about 1/3 of grid size on screen
            gridSize = std::min(rect.height() / (2.5 * spacing / 10.), 12.);
        }
    }

    painter->setRenderHint(QPainter::Antialiasing, true);
    const QColor defaultOnDark{UBSettings::settings()->boardCrossColorDarkBackground->get().toString()};
    const QColor defaultOnLight{UBSettings::settings()->boardCrossColorLightBackground->get().toString()};

    // loop through line groups
    for (const auto& linegroup : mRules->linegroups())
    {
        const auto angle = linegroup.angle();
        const auto spacing = linegroup.spacing() * gridSize / 10;

        auto origin{nominalScene.center()};

        switch (linegroup.origin())
        {
        case Linegroup::Origin::TopLeft:
            origin = nominalScene.topLeft();
            break;

        case Linegroup::Origin::TopRight:
            origin = nominalScene.topRight();
            break;

        case Linegroup::Origin::BottomLeft:
            origin = nominalScene.bottomLeft();
            break;

        case Linegroup::Origin::BottomRight:
            origin = nominalScene.bottomRight();
            break;

        default:
            break;
        }

        // border needs different calculation of start and end of line
        // relative to left and right border
        double leftBorder{0};
        double rightBorder{0};

        bool hasLeftBorder{false};
        bool hasRightBorder{false};

        QList<Line> borderLines;

        if (linegroup.border())
        {
            const auto border = linegroup.border().value();

            hasLeftBorder = border.left().has_value();
            hasRightBorder = border.right().has_value();

            leftBorder = border.left().value_or(0);
            rightBorder = border.right().value_or(0);

            leftBorder *= gridSize / 10;
            rightBorder *= gridSize / 10;

            borderLines = border.lines();
        }

        // loop through lines
        for (const auto& line : linegroup.lines())
        {
            const auto offset = line.offset() * gridSize / 10;

            setLinePainter(painter, line, onDark, defaultOnDark, defaultOnLight);

            // get relevant corners of rectangle according to line angle
            const bool ascending = int(angle / 90.) % 2 == 0;

            QPointF p1;
            QPointF p2;

            if (ascending)
            {
                p1 = rect.topLeft();
                p2 = rect.bottomRight();
            }
            else
            {
                p1 = rect.topRight();
                p2 = rect.bottomLeft();
            }

            // create a line through origin
            auto originLine = QLineF::fromPolar(5000, angle).translated(origin);
            originLine.setP1(origin + origin - originLine.p2()); // extend in other direction

            if (spacing > 0)
            {
                // calculate offsets for first and last lines
                auto minIndex = int(distance(originLine, p1) / spacing);
                auto maxIndex = int(distance(originLine, p2) / spacing);

                if (minIndex > maxIndex)
                {
                    std::swap(minIndex, maxIndex);
                }

                // normal vector for translations
                auto normal = originLine.normalVector();
                normal.setLength(spacing);
                const auto spaceShift = normal.p1() - normal.p2();
                normal.setLength(offset);
                const auto offsetShift = normal.p1() - normal.p2();

                const auto lastLine = &line == &linegroup.lines().last();

                for (int index = minIndex - 1; index <= maxIndex; ++index)
                {
                    auto gridLine = originLine.translated(index * spaceShift + offsetShift);

                    if (hasLeftBorder)
                    {
                        auto p1 = gridLine.p1();
                        p1.setX(nominalScene.left() + leftBorder);
                        gridLine.setP1(p1);

                        if (lastLine)
                        {
                            // draw border lines on left side
                            QLineF borderLine{p1 - offsetShift, p1};

                            if (clip(rect, borderLine))
                            {
                                drawBorderLines(painter, borderLines, borderLine, Qt::LeftEdge, gridSize, onDark,
                                                defaultOnDark, defaultOnLight);
                            }
                        }
                    }

                    if (hasRightBorder)
                    {
                        auto p2 = gridLine.p2();
                        p2.setX(nominalScene.right() - rightBorder);
                        gridLine.setP2(p2);

                        if (lastLine)
                        {
                            // draw border lines on right side
                            QLineF borderLine{p2 - offsetShift, p2};

                            if (clip(rect, borderLine))
                            {
                                drawBorderLines(painter, borderLines, borderLine, Qt::RightEdge, gridSize, onDark,
                                                defaultOnDark, defaultOnLight);
                            }
                        }
                    }

                    if (clip(rect, gridLine))
                    {
                        painter->drawLine(gridLine);
                    }
                }
            }
            else if (spacing == 0)
            {
                // single line at offset
                auto normal = originLine.normalVector();
                normal.setLength(offset);
                const auto offsetShift = normal.p1() - normal.p2();
                auto gridLine = originLine.translated(offsetShift);

                if (clip(rect, gridLine))
                {
                    painter->drawLine(gridLine);
                }
            }
        }
    }
}

void UBBackgroundRuling::setLinePainter(QPainter* painter, const Line& line, bool onDark, QColor defaultOnDark,
                                        QColor defaultOnLight) const
{
    const auto colorOnDark = line.color().onDark();
    const auto colorOnLight = line.color().onLight();

    // apply alpha value to defaults
    defaultOnDark.setAlphaF(colorOnDark.alphaF());
    defaultOnLight.setAlphaF(colorOnLight.alphaF());

    auto pen = painter->pen();
    pen.setWidth(line.width());
    pen.setColor(onDark ? (colorOnDark.isValid() ? colorOnDark : defaultOnDark)
                        : (colorOnLight.isValid() ? colorOnLight : defaultOnLight));
    pen.setCapStyle(Qt::FlatCap);
    painter->setPen(pen);
}

void UBBackgroundRuling::drawBorderLines(QPainter* painter, const QList<Line>& borderLines, QLineF line, Qt::Edge edge,
                                         double gridSize, bool onDark, QColor defaultOnDark,
                                         QColor defaultOnLight) const
{
    painter->save();

    for (const auto& borderLine : borderLines)
    {
        setLinePainter(painter, borderLine, onDark, defaultOnDark, defaultOnLight);

        auto offset = borderLine.offset();
        offset *= gridSize / 10;

        if (edge == Qt::RightEdge)
        {
            offset = -offset;
        }

        painter->drawLine(line.translated({offset, 0}));
    }

    painter->restore();
}

// ====================
// private data classes
// ====================

UBBackgroundRuling::Data::Data(Rules& rules)
    : mRules{&rules}
{
}

bool UBBackgroundRuling::Data::hasError() const
{
    return mError;
}

void UBBackgroundRuling::Data::setError(bool error)
{
    mError = error;
}

const UBBackgroundRuling::Rules& UBBackgroundRuling::Data::rules() const
{
    return *mRules;
}

UBBackgroundRuling::LineColor::LineColor(const Rules& rules, QXmlStreamReader& reader)
    : Data{rules}
    , mOnDark{rules.defaultColor().onDark()}
    , mOnLight{rules.defaultColor().onLight()}
{
    while (reader.readNextStartElement() && reader.namespaceUri().toString() == namespaceURI)
    {
        if (getColorValue(reader, elementOnDark, mOnDark))
        {
        }
        else if (getColorValue(reader, elementOnLight, mOnLight))
        {
        }
        else
        {
            setError();
        }
    }
}

UBBackgroundRuling::LineColor::LineColor(const UBBackgroundRuling::Rules& rules, const QColor& colorOnDark,
                                         const QColor& colorOnLight)
    : Data{rules}
    , mOnDark{colorOnDark}
    , mOnLight{colorOnLight}
{
}

void UBBackgroundRuling::LineColor::toXml(QXmlStreamWriter& writer) const
{
    if (mOnDark.isValid())
    {
        writer.writeTextElement(namespaceURI, elementOnDark, mOnDark.name(QColor::HexArgb));
    }

    if (mOnLight.isValid())
    {
        writer.writeTextElement(namespaceURI, elementOnLight, mOnLight.name(QColor::HexArgb));
    }
}

const QColor& UBBackgroundRuling::LineColor::onDark() const
{
    return mOnDark;
}

const QColor& UBBackgroundRuling::LineColor::onLight() const
{
    return mOnLight;
}

UBBackgroundRuling::Line::Line(const Rules& rules, QXmlStreamReader& reader)
    : Data{rules}
    , mColor{rules.defaultColor()}
{
    while (reader.readNextStartElement() && reader.namespaceUri().toString() == namespaceURI)
    {
        if (getDecimalValue(reader, elementOffset, mOffset))
        {
        }
        else if (getDecimalValue(reader, elementWidth, mWidth))
        {
        }
        else if (reader.name().toString() == elementColor)
        {
            mColor = LineColor{rules, reader};
        }
        else
        {
            setError();
        }
    }
}

void UBBackgroundRuling::Line::toXml(QXmlStreamWriter& writer) const
{
    writer.writeTextElement(namespaceURI, elementOffset, QString::number(mOffset));
    writer.writeTextElement(namespaceURI, elementWidth, QString::number(mWidth));

    if (mColor.onDark().isValid() || mColor.onLight().isValid())
    {
        writer.writeStartElement(namespaceURI, elementColor);
        mColor.toXml(writer);
        writer.writeEndElement();
    }
}

double UBBackgroundRuling::Line::offset() const
{
    return mOffset;
}

double UBBackgroundRuling::Line::width() const
{
    return mWidth;
}

const UBBackgroundRuling::LineColor& UBBackgroundRuling::Line::color() const
{
    return mColor;
}

UBBackgroundRuling::Border::Border(const Rules& rules, QXmlStreamReader& reader)
    : Data{rules}
{
    while (reader.readNextStartElement() && reader.namespaceUri().toString() == namespaceURI)
    {
        double value;

        if (getDecimalValue(reader, elementLeft, value))
        {
            mLeft.emplace(value);
        }
        else if (getDecimalValue(reader, elementRight, value))
        {
            mRight.emplace(value);
        }
        else if (reader.name().toString() == elementLine)
        {
            const Line bgLine{rules, reader};

            if (bgLine.hasError())
            {
                setError();
            }
            else
            {
                mLines.append(bgLine);
            }
        }
    }
}

void UBBackgroundRuling::Border::toXml(QXmlStreamWriter& writer) const
{
    if (mLeft)
    {
        writer.writeTextElement(namespaceURI, elementLeft, QString::number(mLeft.value()));
    }

    if (mRight)
    {
        writer.writeTextElement(namespaceURI, elementRight, QString::number(mRight.value()));
    }

    for (const auto& line : mLines)
    {
        writer.writeStartElement(namespaceURI, elementLine);
        line.toXml(writer);
        writer.writeEndElement();
    }
}

std::optional<double> UBBackgroundRuling::Border::left() const
{
    return mLeft;
}

std::optional<double> UBBackgroundRuling::Border::right() const
{
    return mRight;
}

const QList<UBBackgroundRuling::Line>& UBBackgroundRuling::Border::lines() const
{
    return mLines;
}

UBBackgroundRuling::Linegroup::Linegroup(const Rules& rules, QXmlStreamReader& reader)
    : Data{rules}
{
    QString lineGroupOrigin;

    while (reader.readNextStartElement() && reader.namespaceUri().toString() == namespaceURI)
    {
        if (getDecimalValue(reader, elementAngle, mAngle))
        {
        }
        else if (getDecimalValue(reader, elementSpacing, mSpacing))
        {
        }
        else if (getStringValue(reader, elementOrigin, lineGroupOrigin))
        {
            if (lineGroupOrigin == originTopleft)
            {
                mOrigin = Origin::TopLeft;
            }
            else if (lineGroupOrigin == originTopright)
            {
                mOrigin = Origin::TopRight;
            }
            else if (lineGroupOrigin == originBottomleft)
            {
                mOrigin = Origin::BottomLeft;
            }
            else if (lineGroupOrigin == originBottomright)
            {
                mOrigin = Origin::BottomRight;
            }
            else if (lineGroupOrigin == originCenter)
            {
                mOrigin = Origin::Center;
            }
            else
            {
                setError();
            }
        }
        else if (reader.name().toString() == elementLine)
        {
            const Line bgLine{rules, reader};

            if (bgLine.hasError())
            {
                setError();
            }
            else
            {
                mLines.append(bgLine);
            }
        }
        else if (reader.name().toString() == elementBorder)
        {
            mBorder.emplace(rules, reader);
        }
        else
        {
            setError();
        }
    }
}

void UBBackgroundRuling::Linegroup::toXml(QXmlStreamWriter& writer) const
{
    writer.writeTextElement(namespaceURI, elementAngle, QString::number(mAngle));

    if (mSpacing != 0)
    {
        writer.writeTextElement(namespaceURI, elementSpacing, QString::number(mSpacing));
    }

    QString origin;

    switch (mOrigin)
    {
    case Origin::Center:
        origin = originCenter;
        break;
    case Origin::TopLeft:
        origin = originTopleft;
        break;
    case Origin::TopRight:
        origin = originTopright;
        break;
    case Origin::BottomLeft:
        origin = originBottomleft;
        break;
    case Origin::BottomRight:
        origin = originBottomright;
        break;
    }

    writer.writeTextElement(namespaceURI, elementOrigin, origin);

    for (const auto& line : mLines)
    {
        writer.writeStartElement(namespaceURI, elementLine);
        line.toXml(writer);
        writer.writeEndElement();
    }

    if (mBorder)
    {
        writer.writeStartElement(namespaceURI, elementBorder);
        mBorder.value().toXml(writer);
        writer.writeEndElement();
    }
}

double UBBackgroundRuling::Linegroup::angle() const
{
    return mAngle;
}

double UBBackgroundRuling::Linegroup::spacing() const
{
    return mSpacing;
}

UBBackgroundRuling::Linegroup::Origin UBBackgroundRuling::Linegroup::origin() const
{
    return mOrigin;
}

const QList<UBBackgroundRuling::Line>& UBBackgroundRuling::Linegroup::lines() const
{
    return mLines;
}

std::optional<UBBackgroundRuling::Border> UBBackgroundRuling::Linegroup::border() const
{
    return mBorder;
}

UBBackgroundRuling::Rules::Rules(QXmlStreamReader& reader)
    : Data{*this}
    , mDefaultColor{*this, {}, {}}
{
    // current position of reader should be at the <background> element
    while (!reader.isEndElement())
    {
        while (reader.readNextStartElement() && reader.namespaceUri().toString() == namespaceURI)
        {
            const auto name = reader.name();

            QString text;

            if (getStringValue(reader, elementUuid, text))
            {
                mUuid = QUuid(text);
            }
            else if (name.toString() == elementDescription)
            {
                const auto attributes = reader.attributes();
                const auto lang = attributes.value(attributeXmlLang).toString();

                mDescriptions[lang.isEmpty() ? "en" : lang] = reader.readElementText();
            }
            else if (name.toString() == elementAttributes)
            {
                const auto attributes = reader.attributes();
                mCrossed = attributes.value(attributeCrossed).toString() == valueTrue;
                mRuled = attributes.value(attributeRuled).toString() == valueTrue;
                mIntermediateLines = attributes.value(attributeIntermediateLines).toString() == valueTrue;
                reader.skipCurrentElement();
            }
            else if (name.toString() == elementDefaultColor)
            {
                mDefaultColor = LineColor{*this, reader};
            }
            else if (name.toString() == elementLinegroup)
            {
                const Linegroup bgLinegroup{*this, reader};

                if (bgLinegroup.hasError())
                {
                    setError();
                }
                else
                {
                    mLinegroups.append(bgLinegroup);
                }
            }
            else
            {
                setError();
            }
        }
    }
}

UBBackgroundRuling::Rules* UBBackgroundRuling::Rules::fromXml(QXmlStreamReader& reader)
{
    if (reader.namespaceUri().toString() == namespaceURI && reader.name().toString() == elementBackground)
    {
        return new Rules(reader);
    }

    return nullptr;
}

void UBBackgroundRuling::Rules::toXml(QXmlStreamWriter& writer) const
{
    writer.writeNamespace(namespaceURI, namespacePrefix);
    writer.writeStartElement(namespaceURI, elementBackground);
    writer.writeTextElement(namespaceURI, elementUuid, mUuid.toString(QUuid::WithoutBraces));

    for (auto description = mDescriptions.constKeyValueBegin(); description != mDescriptions.constKeyValueEnd();
         ++description)
    {
        writer.writeStartElement(namespaceURI, elementDescription);
        writer.writeAttribute(attributeXmlLang, description->first);
        writer.writeCharacters(description->second);
        writer.writeEndElement();
    }

    if (mCrossed || mRuled)
    {
        writer.writeEmptyElement(namespaceURI, elementAttributes);

        if (mCrossed)
        {
            writer.writeAttribute(attributeCrossed, valueTrue);
        }

        if (mRuled)
        {
            writer.writeAttribute(attributeRuled, valueTrue);
        }

        if (mIntermediateLines)
        {
            writer.writeAttribute(attributeIntermediateLines, valueTrue);
        }
    }

    if (mDefaultColor.onDark().isValid() || mDefaultColor.onLight().isValid())
    {
        writer.writeStartElement(namespaceURI, elementDefaultColor);
        mDefaultColor.toXml(writer);
        writer.writeEndElement();
    }

    for (const auto& linegroup : mLinegroups)
    {
        writer.writeStartElement(namespaceURI, elementLinegroup);
        linegroup.toXml(writer);
        writer.writeEndElement();
    }

    writer.writeEndElement();
}

QUuid UBBackgroundRuling::Rules::uuid() const
{
    return mUuid;
}

QString UBBackgroundRuling::Rules::description(const QString& languagecode) const
{
    return mDescriptions.value(languagecode, mDescriptions.value("en"));
}

bool UBBackgroundRuling::Rules::isCrossed() const
{
    return mCrossed;
}

bool UBBackgroundRuling::Rules::isRuled() const
{
    return mRuled;
}

bool UBBackgroundRuling::Rules::hasIntermediateLines() const
{
    return mIntermediateLines;
}

UBBackgroundRuling::LineColor UBBackgroundRuling::Rules::defaultColor() const
{
    return mDefaultColor;
}

const QList<UBBackgroundRuling::Linegroup>& UBBackgroundRuling::Rules::linegroups() const
{
    return mLinegroups;
}
