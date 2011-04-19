/*
 * UBExportAdaptor.h
 *
 *  Created on: Feb 10, 2009
 *      Author: julienbachmann
 */

#ifndef UBEXPORTADAPTOR_H_
#define UBEXPORTADAPTOR_H_

#include <QtGui>

class UBDocumentProxy;

class UBExportAdaptor : public QObject
{
    Q_OBJECT;

    public:
        UBExportAdaptor(QObject *parent = 0);
        virtual ~UBExportAdaptor();

        virtual QString exportName() = 0;
        virtual QString exportExtention() { return "";}
        virtual void persist(UBDocumentProxy* pDocument) = 0;

        virtual void setVerbode(bool verbose)
        {
            mIsVerbose = verbose;
        }

        virtual bool isVerbose() const
        {
            return mIsVerbose;
        }

    protected:
        QString askForFileName(UBDocumentProxy* pDocument, const QString& pDialogTitle);
        QString askForDirName(UBDocumentProxy* pDocument, const QString& pDialogTitle);

        bool mIsVerbose;

};

#endif /* UBEXPORTADAPTOR_H_ */
