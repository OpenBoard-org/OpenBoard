/*
 * UBSetting.h
 *
 *  Created on: Nov 24, 2008
 *      Author: luc
 */

#ifndef UBSETTING_H_
#define UBSETTING_H_

#include <QtGui>

class UBSettings;

class UBSetting : public QObject
{
    Q_OBJECT;

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
    Q_OBJECT;

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
