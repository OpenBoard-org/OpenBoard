/*
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open-Sankoré.  If not, see <http://www.gnu.org/licenses/>.
 */



#ifndef UBCREATELINKPALETTE_H
#define UBCREATELINKPALETTE_H

class QVBoxLayout;
class QStackedWidget;
class QToolButton;
class QWidget;
class UBGraphicsItemAction;
class QLineEdit;
class QButtonGroup;
class QComboBox;
class QDropEvent;

#include <QMap>
#include <QLabel>

#include "UBFloatingPalette.h"

class UBCreateLinkLabel : public QLabel
{
    Q_OBJECT

public:
    explicit UBCreateLinkLabel(QString labelText, QWidget* parent = 0);

signals:
    void droppedFile(QString& path);

protected:
    void dropEvent(QDropEvent* event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *event);
private:
    QString mInitialText;

};


class UBCreateLinkPalette : public UBFloatingPalette
{
    Q_OBJECT
public:
    explicit UBCreateLinkPalette(QWidget *parent = 0);
    ~UBCreateLinkPalette();
signals:
    void definedAction(UBGraphicsItemAction* action);

public slots:

protected:
    virtual void paintEvent(QPaintEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent * event);
    virtual void showEvent(QShowEvent *event);

private:
    void close();
    void adjustGeometry();
    void init();
    QToolButton* addInitiaWidgetButton(QString textButton, QString iconPath);
    QWidget* addBasicFunctionaliltiesToWidget(QWidget* centralWidget);

    QVBoxLayout* mLayout;
    QStackedWidget* mStackedWidget;
    QWidget* mInitialWidget;
    QWidget* mAudioWidget;
    QWidget* mPageLinkWidget;
    QWidget* mUrlLinkWidget;

    QLineEdit* mUrlLineEdit;
    QButtonGroup* mButtonGroup;
    QComboBox* mPageComboBox;
    UBCreateLinkLabel* mpAudioLabel;
    QString mAudioFilePath;
    bool mIsFirstTime;

private slots:
    void onBackButtonClicked();
    void onPlayAudioClicked();
    void onAddLinkToPageClicked();
    void onAddLinkToWebClicked();

    void onOkAudioClicked();
    void onOkLinkToPageClicked();
    void onOkLinkToWebClicked();

    void onPageNumberCheckBoxClicked(bool checked);

    void onDroppedAudioFile(QString& path);
};

#endif // UBCREATELINKPALETTE_H
