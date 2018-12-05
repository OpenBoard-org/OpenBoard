/*
 * Copyright (C) 2015-2018 Département de l'Instruction Publique (DIP-SEM)
 *
 * Copyright (C) 2013 Open Education Foundation
 *
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour
 * l'Education Numérique en Afrique (GIP ENA)
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




#ifndef UBEXPORTADAPTOR_H_
#define UBEXPORTADAPTOR_H_

#include <QtGui>

class UBDocumentProxy;

class UBExportAdaptor : public QObject
{
    Q_OBJECT

    public:
        UBExportAdaptor(QObject *parent = 0);
        virtual ~UBExportAdaptor();

        virtual QString exportName() = 0;
        virtual QString exportExtention() { return "";}
        virtual void persist(UBDocumentProxy* pDocument) = 0;
        virtual bool persistsDocument(UBDocumentProxy* pDocument, const QString& filename);
        virtual bool associatedActionactionAvailableFor(const QModelIndex &selectedIndex) {Q_UNUSED(selectedIndex); return false;}
        QAction *associatedAction() {return mAssociatedAction;}
        void setAssociatedAction(QAction *pAssociatedAction) {mAssociatedAction = pAssociatedAction;}

        virtual void setVerbose(bool verbose)
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

        virtual void persistLocally(UBDocumentProxy* pDocumentProxy, const QString &pDialogTitle);

        void showErrorsList(QList<QString> errorsList);

        bool mIsVerbose;
        QAction* mAssociatedAction;

};

#endif /* UBEXPORTADAPTOR_H_ */
