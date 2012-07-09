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
