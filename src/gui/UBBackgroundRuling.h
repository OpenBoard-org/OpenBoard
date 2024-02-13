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



#pragma once

#include <QColor>
#include <QMap>
#include <QUuid>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include <memory>

// forward
class QPainter;


class UBBackgroundRuling
{
public:
    UBBackgroundRuling();

    bool operator==(const UBBackgroundRuling& other) const;

    bool parseXml(QXmlStreamReader& reader, bool userProvided = true);
    bool toXml(QXmlStreamWriter& writer) const;

    bool isValid() const;
    QUuid uuid() const;
    QString description(const QString& languagecode = "en") const;
    bool isCrossed() const;
    bool isRuled() const;
    bool hasIntermediateLines() const;
    bool isUserProvided() const;

    void draw(QPainter* painter, const QRectF& rect, double gridSize, const QRectF& nominalScene, bool onDark) const;

private:
    class Line;  // forward
    void setLinePainter(QPainter* painter, const Line& line, bool onDark, QColor defaultOnDark,
                        QColor defaultOnLight) const;
    void drawBorderLines(QPainter* painter, const QList<Line>& borderLines, QLineF line, Qt::Edge edge, double gridSize,
                         bool onDark, QColor defaultOnDark, QColor defaultOnLight) const;

private:
    // data classes
    class Rules;

    class Data
    {
    public:
        explicit Data(Rules& rules);

        bool hasError() const;
        virtual void toXml(QXmlStreamWriter& writer) const = 0;

    protected:
        void setError(bool error = true);
        const Rules& rules() const;

    private:
        bool mError{false};
        Rules* mRules{nullptr};
    };

    class LineColor : public Data
    {
    public:
        LineColor(const Rules& rules, QXmlStreamReader& reader);
        LineColor(const Rules& rules, const QColor& colorOnDark, const QColor& colorOnLight);

        virtual void toXml(QXmlStreamWriter& writer) const override;

        const QColor& onDark() const;
        const QColor& onLight() const;

    private:
        QColor mOnDark{};
        QColor mOnLight{};
    };

    class Line : public Data
    {
    public:
        Line(const Rules& rules, QXmlStreamReader& reader);

        virtual void toXml(QXmlStreamWriter& writer) const override;

        double offset() const;
        double width() const;
        const LineColor& color() const;

    private:
        double mOffset{0.};
        double mWidth{1.};
        LineColor mColor;
    };

    class Border : public Data
    {
    public:
        Border(const Rules& rules, QXmlStreamReader& reader);

        virtual void toXml(QXmlStreamWriter& writer) const override;

        std::optional<double> left() const;
        std::optional<double> right() const;
        const QList<Line>& lines() const;

    private:
        std::optional<double> mLeft{};
        std::optional<double> mRight{};
        QList<Line> mLines{};
    };

    class Linegroup : public Data
    {
    public:
        enum class Origin
        {
            Center,
            TopLeft,
            TopRight,
            BottomLeft,
            BottomRight
        };

        Linegroup(const Rules& rules, QXmlStreamReader& reader);

        virtual void toXml(QXmlStreamWriter& writer) const override;

        double angle() const;
        double spacing() const;
        Origin origin() const;
        const QList<Line>& lines() const;
        std::optional<Border> border() const;

    private:
        double mAngle{0.};
        double mSpacing{0.};
        Origin mOrigin{Origin::Center};
        QList<Line> mLines{};
        std::optional<Border> mBorder{};
    };

    class Rules : public Data
    {
    public:
        explicit Rules(QXmlStreamReader& reader);

        static Rules* fromXml(QXmlStreamReader& reader);

        virtual void toXml(QXmlStreamWriter& writer) const override;

        QUuid uuid() const;
        QString description(const QString& languagecode = QString{}) const;
        bool isCrossed() const;
        bool isRuled() const;
        bool hasIntermediateLines() const;
        LineColor defaultColor() const;
        const QList<Linegroup>& linegroups() const;

    private:
        QUuid mUuid{};
        QMap<QString, QString> mDescriptions{};
        bool mCrossed{false};
        bool mRuled{false};
        bool mIntermediateLines{false};
        LineColor mDefaultColor;
        QList<Linegroup> mLinegroups{};
    };

private:
    std::shared_ptr<Rules> mRules{nullptr};
    bool mUserProvided{false};
};
