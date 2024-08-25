/*
 * Copyright (C) 2015-2022 Département de l'Instruction Publique (DIP-SEM)
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
#include <QLineEdit>

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
    UBPreferencesDialog(UBPreferencesController* prefController, QWidget* parent = 0, Qt::WindowFlags f = {} );
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

        bool handleKeyEvent(QKeyEvent *event);
        bool handleMouseEvent(QMouseEvent *event);
        bool handleTabletEvent(QTabletEvent *event);

    public slots:

        void show();

    protected:

        void wire();
        void init();

        virtual bool eventFilter(QObject* obj, QEvent* event) Q_DECL_OVERRIDE;

        UBPreferencesDialog* mPreferencesWindow;
        Ui::preferencesDialog* mPreferencesUI;
        UBBrushPropertiesFrame* mPenProperties;
        UBBrushPropertiesFrame* mMarkerProperties;
        UBColorPicker* mDarkBackgroundGridColorPicker;
        UBColorPicker* mLightBackgroundGridColorPicker;
        QString mScreenConfigurationPath;

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
        void actionSelected(const QModelIndex& index);
        void recordingClicked(bool checked);
        void abortClicked();
        void resetClicked();

    private slots:
        void adjustScreensPreferences();
        void applyShortcutFilter(const QString& filter, int filterCol = -1);


    private:
        static qreal sSliderRatio;
        static qreal sMinPenWidth;
        static qreal sMaxPenWidth;
        QModelIndex currentIndex;
};

class UBBrushPropertiesFrame : public Ui::brushProperties
{

    public:
        UBBrushPropertiesFrame(QFrame* owner, const QList<QColor>& lightBackgroundColors,const QList<QColor>& darkBackgroundColors, const QList<QColor>& lightBackgroundSelectedColors,const QList<QColor>& darkBackgroundSelectedColors, UBPreferencesController* controller);

        virtual ~UBBrushPropertiesFrame(){}

        QList<UBColorPicker*> lightBackgroundColorPickers;
        QList<UBColorPicker*> darkBackgroundColorPickers;

};

// forward
class UBStringListValidator;

class UBScreenListLineEdit : public QLineEdit
{
    Q_OBJECT;

public:
    UBScreenListLineEdit(QWidget* parent);
    virtual ~UBScreenListLineEdit() = default;

    void setDefault();
    void loadScreenList(const QStringList& screenList);

protected:
    virtual void focusInEvent(QFocusEvent* focusEvent) override;
    virtual void focusOutEvent(QFocusEvent* focusEvent) override;

signals:
    void screenListChanged(QStringList screenList);

private slots:
    void addScreen();
    void onTextChanged(const QString& input);

private:
    QList<QPushButton*> mScreenLabels;
    UBStringListValidator* mValidator;
};

class UBStringListValidator : public QValidator
{
    Q_OBJECT;

public:
    UBStringListValidator(QObject* parent = nullptr);
    virtual ~UBStringListValidator() = default;

    virtual void fixup(QString& input) const;
    virtual QValidator::State validate(QString& input, int& pos) const;

    void setValidationStringList(const QStringList& list);

private:
    QStringList mList;
};

#endif /* UBPREFERENCESCONTROLLER_H_ */
