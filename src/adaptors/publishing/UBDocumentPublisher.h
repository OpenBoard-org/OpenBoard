/*
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




#ifndef UBDOCUMENTPUBLISHER_H
#define UBDOCUMENTPUBLISHER_H

#include <QtGui>
#include <QtNetwork>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>

typedef struct
{
    QString title;
    QString description;
} sDocumentInfos;

class UBDocumentProxy;
class UBServerXMLHttpRequest;
class UBGraphicsW3CWidgetItem;
class QWebView;

class UBProxyLoginDlg : public QDialog
{
    Q_OBJECT
public:
    UBProxyLoginDlg(QWidget* parent=0, const char* name="ProxyLoginDlg");
    ~UBProxyLoginDlg();

    QString username(){return mpUsername->text();}
    QString password(){return mpPassword->text();}

private:
    QVBoxLayout* mpLayout;
    QHBoxLayout* mpUserLayout;
    QHBoxLayout* mpPasswordLayout;
    QDialogButtonBox* mpButtons;
    QLabel* mpUserLabel;
    QLabel* mpPasswordLabel;
    QLineEdit* mpUsername;
    QLineEdit* mpPassword;
};

class UBPublicationDlg : public QDialog
{
    Q_OBJECT
public:
    UBPublicationDlg(QWidget* parent=0, const char* name="UBPublicationDlg");
    ~UBPublicationDlg();

    QString title(){return mpTitle->text();}
    QString description(){return mpDescription->document()->toPlainText();}

private slots:
    void onTextChanged();

private:
    QVBoxLayout* mpLayout;
    QHBoxLayout* mpTitleLayout;
    QLabel* mpTitleLabel;
    QLineEdit* mpTitle;
    QLabel* mpDescLabel;
    QTextEdit* mpDescription;
    QDialogButtonBox* mpButtons;
};
#endif // UBDOCUMENTPUBLISHER_H
