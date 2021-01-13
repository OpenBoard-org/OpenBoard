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




#ifndef UBPREFERENCESCONTROLLER_H_
#define UBPREFERENCESCONTROLLER_H_

#include <QtGui>
#include <QDialog>

class UBColorPicker;
class UBApplication;
class UBSettings;
class UBPreferencesController;

namespace Ui
{
    class preferencesDialog;
}

#include "ui_brushProperties.h"

class UBBrushPropertiesFrame;

class UBPreferencesDialog : public QDialog
{
    Q_OBJECT;

public:
    UBPreferencesDialog(UBPreferencesController* prefController, QWidget* parent = 0,Qt::WindowFlags f = 0 );
    ~UBPreferencesDialog();

protected:
    void closeEvent(QCloseEvent* e);
    UBPreferencesController *mPreferencesController;
};


class UBPreferencesController : public QObject
{
    Q_OBJECT

    public:
        UBPreferencesController(QWidget *parent);
        virtual ~UBPreferencesController();


    public slots:

        void show();

    protected:

        void wire();
        void init();

        UBPreferencesDialog* mPreferencesWindow;
        Ui::preferencesDialog* mPreferencesUI;
        UBBrushPropertiesFrame* mPenProperties;
        UBBrushPropertiesFrame* mMarkerProperties;
        UBColorPicker* mDarkBackgroundGridColorPicker;
        UBColorPicker* mLightBackgroundGridColorPicker;

    protected slots:

        void close();
        void defaultSettings();
        void penPreviewFromSizeChanged(int value);
        void darkBackgroundCrossOpacityValueChanged(int value);
        void lightBackgroundCrossOpacityValueChanged(int value);
        void widthSliderChanged(int value);
        void opacitySliderChanged(int value);
        void colorSelected(const QColor&);
        void setCrossColorOnDarkBackground(const QColor& color);
        void setCrossColorOnLightBackground(const QColor& color);
        void toolbarPositionChanged(bool checked);
        void toolbarOrientationVertical(bool checked);
        void toolbarOrientationHorizontal(bool checked);
        void systemOSKCheckBoxToggled(bool checked);
        void setPdfZoomBehavior(bool checked);

    private slots:
        void adjustScreens(int screen);

    private:
        static qreal sSliderRatio;
        static qreal sMinPenWidth;
        static qreal sMaxPenWidth;
        QDesktopWidget* mDesktop;

};

class UBBrushPropertiesFrame : public Ui::brushProperties
{

    public:
        UBBrushPropertiesFrame(QFrame* owner, const QList<QColor>& lightBackgroundColors,const QList<QColor>& darkBackgroundColors, const QList<QColor>& lightBackgroundSelectedColors,const QList<QColor>& darkBackgroundSelectedColors, UBPreferencesController* controller);

        virtual ~UBBrushPropertiesFrame(){}

        QList<UBColorPicker*> lightBackgroundColorPickers;
        QList<UBColorPicker*> darkBackgroundColorPickers;

};

#endif /* UBPREFERENCESCONTROLLER_H_ */
