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

#include "UBMessagesDialog.h"

UBMessagesDialog::UBMessagesDialog(QString windowTitle, QWidget *parent)
: QWidget(parent)
{
    resize(400, 0);

    setWindowTitle(windowTitle);
    setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);

}

void UBMessagesDialog::setMessages(const QList<QString> messages)
{
    mMessages = messages;

    if (mMessages.count())
    {        
        QVBoxLayout *messagesLayout = new QVBoxLayout(this);
        foreach (QString message, mMessages)
        {
            QTextEdit *messageBox = new QTextEdit(this);
            messageBox->setMinimumHeight(55);
            messageBox->setReadOnly(true);
            messageBox->setFocusPolicy(Qt::NoFocus);
            messageBox->setText(message);
            messagesLayout->addWidget(messageBox);
        }
        QPushButton *closeButton = new QPushButton(tr("Close"), this);
        connect(closeButton, SIGNAL(clicked(bool)), this, SLOT(dispose()));

        messagesLayout->addWidget(closeButton);

        setLayout(messagesLayout);
    }
}

void UBMessagesDialog::dispose()
{
    delete this;
}