/*
 * UBPreferencesController.h
 *
 *  Created on: Nov 18, 2008
 *      Author: luc
 */

#ifndef UBPREFERENCESCONTROLLER_H_
#define UBPREFERENCESCONTROLLER_H_

#include <QtGui>

class UBColorPicker;
class UBApplication;
class UBSettings;

namespace Ui
{
    class preferencesDialog;
}

#include "ui_brushProperties.h"

class UBBrushPropertiesFrame;

class UBPreferencesController : public QObject
{
    Q_OBJECT;

    public:
        UBPreferencesController(QWidget *parent);
        virtual ~UBPreferencesController();

    public slots:

        void show();

    protected:

        void wire();
        void init();

        QDialog* mPreferencesWindow;
        Ui::preferencesDialog* mPreferencesUI;
        UBBrushPropertiesFrame* mPenProperties;
        UBBrushPropertiesFrame* mMarkerProperties;

    protected slots:

        void close();
        void defaultSettings();
        void widthSliderChanged(int value);
        void opacitySliderChanged(int value);
        void colorSelected(const QColor&);
        void toolbarPositionChanged(bool checked);
        void toolbarOrientationVertical(bool checked);
        void toolbarOrientationHorizontal(bool checked);
        void onCommunityUsernameChanged();
        void onCommunityPasswordChanged();

    private:

        static qreal sSliderRatio;
        static qreal sMinPenWidth;
        static qreal sMaxPenWidth;

};

class UBBrushPropertiesFrame : public Ui::brushProperties
{

    public:
        UBBrushPropertiesFrame(QFrame* owner, const QList<QColor>& lightBackgroundColors,
                const QList<QColor>& darkBackgroundColors, const QList<QColor>& lightBackgroundSelectedColors,
                const QList<QColor>& darkBackgroundSelectedColors, UBPreferencesController* controller);

        virtual ~UBBrushPropertiesFrame(){}

        QList<UBColorPicker*> lightBackgroundColorPickers;
        QList<UBColorPicker*> darkBackgroundColorPickers;

};


#endif /* UBPREFERENCESCONTROLLER_H_ */
