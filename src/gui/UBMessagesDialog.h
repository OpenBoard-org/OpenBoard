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

#ifndef UB_MESSAGES_DIALOG_H_
#define UB_MESSAGES_DIALOG_H_

#include <QtGui>

class UBMessagesDialog : public QWidget
{
    Q_OBJECT

public:
    UBMessagesDialog(QString windowTitle, QWidget *parent = NULL);
    void setMessages(const QList<QString> messages);

    private slots:
        void dispose();

private:
    QList<QString> mMessages;
    int mMessagesFontSize;
};

#endif UB_MESSAGES_DIALOG_H_