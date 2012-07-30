/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef UBSETTING_H_
#define UBSETTING_H_

#include <QtGui>

class UBSettings;

class UBSetting : public QObject
{
    Q_OBJECT

    public:
        UBSetting(UBSettings* parent = 0);
        UBSetting(UBSettings* owner, const QString& pDomain, const QString& pKey,
                        const QVariant& pDefaultValue);

        virtual ~UBSetting();

        virtual void set(const QVariant& pValue);
        virtual QVariant get();
        virtual QVariant reset();

        virtual QString domain() const
        {
            return mDomain;
        }

        virtual QString key() const
        {
            return mKey;
        }

        virtual QString path() const
        {
            return mPath;
        }

    public slots:

        void setBool(bool pValue);
		void setString(const QString& pValue);
        void setInt(int pValue);

    signals:

        void changed(QVariant newValue);

    protected:

        UBSettings* mOwner;
        QString mDomain;
        QString mKey;
        QString mPath;
        QVariant mDefaultValue;
};


class UBColorListSetting : public UBSetting
{
    Q_OBJECT

    public:
        UBColorListSetting(UBSettings* parent = 0);

        UBColorListSetting(UBSettings* owner, const QString& pDomain,
                const QString& pKey, const QVariant& pDefaultValue, qreal pAlpha = 1.0);

        virtual ~UBColorListSetting();

        virtual QVariant reset();

        QList<QColor> colors() const;

        void setColor(int pIndex, const QColor& color);

        void setAlpha(qreal pAlpha);

    protected:
        QList<QColor> mColors;
        qreal mAlpha;
};



#endif /* UBSETTING_H_ */
