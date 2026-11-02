/*
 * Copyright (C) 2015-2026 Département de l'Instruction Publique (DIP-SEM)
 * and contributors.
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


#include "UBPageSizePalette.h"

#include "adaptors/UBThumbnailAdaptor.h"

#include "core/UBApplication.h"

#include "document/UBDocument.h"
#include "document/UBDocumentContainer.h"
#include "document/UBDocumentController.h"

#include "document/UBDocumentToc.h"
#include "domain/UBGraphicsScene.h"
#include "domain/UBPageSizeUndoCommand.h"

#include "gui/UBMainWindow.h"
#include "gui/UBThumbnail.h"
#include "gui/UBThumbnailScene.h"


UBPageSizePalette::UBPageSizePalette(QToolBar* toolBar, UBDocumentContainer* container, QWidget* parent)
    : UBToolbarExtensionPalette(toolBar, parent)
    , mContainer{container}
{
    Ui::PageSize::setupUi(this);

    // add undo/redo buttons
    QToolButton* undoButton = new QToolButton{this};
    undoButton->setObjectName("toolButtonUndo");
    QIcon undoIcon;
    undoIcon.addFile(QString::fromUtf8(":/images/toolbar/undo.png"), QSize(), QIcon::Mode::Disabled);
    undoIcon.addFile(QString::fromUtf8(":/images/toolbar/undoOn.png"), QSize(), QIcon::Mode::Normal);
    undoButton->setIcon(undoIcon);
    undoButton->setIconSize(QSize(24, 24));
    undoButton->setEnabled(false);
    buttonBox->addButton(undoButton, QDialogButtonBox::ActionRole);

    QToolButton* redoButton = new QToolButton{this};
    undoButton->setObjectName("toolButtonRedo");
    QIcon redoIcon;
    redoIcon.addFile(QString::fromUtf8(":/images/toolbar/redo.png"), QSize(), QIcon::Mode::Disabled);
    redoIcon.addFile(QString::fromUtf8(":/images/toolbar/redoOn.png"), QSize(), QIcon::Mode::Normal);
    redoButton->setIcon(redoIcon);
    redoButton->setIconSize(QSize(24, 24));
    redoButton->setEnabled(false);
    buttonBox->addButton(redoButton, QDialogButtonBox::ActionRole);

    auto mainWindow = dynamic_cast<UBMainWindow*>(parent);

    if (mainWindow)
    {
        connect(mainWindow->actionPageSize, &QAction::triggered, this, &UBPageSizePalette::showOnToolbar);
    }

    connect(container, &UBDocumentContainer::documentSet, this, &UBPageSizePalette::setDocument);

    connect(buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &QWidget::hide);
    connect(buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, this, &UBPageSizePalette::apply);

    radioButton4_3->setProperty("format", DocumentSize::Ratio4_3);
    radioButton16_9->setProperty("format", DocumentSize::Ratio16_9);
    radioButton16_10->setProperty("format", DocumentSize::Ratio16_10);
    radioButtonCustomRatio->setProperty("format", DocumentSize::CustomRatio);
    radioButton_A4->setProperty("format", DocumentSize::SizeA4);
    radioButton_A5->setProperty("format", DocumentSize::SizeA5);
    radioButtonCustomSize->setProperty("format", DocumentSize::CustomSize);
    radioButtonImportedSize->setProperty("format", DocumentSize::ImportedSize);

    connect(radioButton4_3, &QRadioButton::clicked, this, &UBPageSizePalette::selectFormat);
    connect(radioButton16_9, &QRadioButton::clicked, this, &UBPageSizePalette::selectFormat);
    connect(radioButton16_10, &QRadioButton::clicked, this, &UBPageSizePalette::selectFormat);
    connect(radioButtonCustomRatio, &QRadioButton::clicked, this, &UBPageSizePalette::selectFormat);
    connect(radioButton_A4, &QRadioButton::clicked, this, &UBPageSizePalette::selectFormat);
    connect(radioButton_A5, &QRadioButton::clicked, this, &UBPageSizePalette::selectFormat);
    connect(radioButtonImportedSize, &QRadioButton::clicked, this, &UBPageSizePalette::selectFormat);
    connect(radioButtonCustomSize, &QRadioButton::clicked, this, &UBPageSizePalette::selectFormat);

    connect(spinBoxX, &QSpinBox::valueChanged, this, &UBPageSizePalette::switchToCustom);
    connect(spinBoxY, &QSpinBox::valueChanged, this, &UBPageSizePalette::switchToCustom);
    connect(doubleSpinBoxWidth, &QDoubleSpinBox::valueChanged, this, &UBPageSizePalette::switchToCustom);
    connect(doubleSpinBoxHeight, &QDoubleSpinBox::valueChanged, this, &UBPageSizePalette::switchToCustom);

    connect(toolButtonFormat, &QToolButton::clicked, this, &UBPageSizePalette::flip);

    connect(container->undoStack(), &QUndoStack::canUndoChanged, undoButton, &QToolButton::setEnabled);
    connect(container->undoStack(), &QUndoStack::canRedoChanged, redoButton, &QToolButton::setEnabled);

    connect(undoButton, &QToolButton::clicked, this, &UBPageSizePalette::undo);
    connect(redoButton, &QToolButton::clicked, this, &UBPageSizePalette::redo);
}

void UBPageSizePalette::showEvent(QShowEvent* event)
{
    selectionChanged();
}

void UBPageSizePalette::showOnToolbar()
{
    if (mToolBar->isVisible())
    {
        show();
        setFocus();
    }
}

void UBPageSizePalette::setDocument(std::shared_ptr<UBDocumentProxy> proxy)
{
    if (mDocument)
    {
        disconnect(mDocument->thumbnailScene(false), &UBThumbnailScene::selectionChanged, this, &UBPageSizePalette::selectionChanged);
    }

    mDocument = UBDocument::getDocument(proxy);

    if (mDocument)
    {
        connect(mDocument->thumbnailScene(false), &UBThumbnailScene::selectionChanged, this, &UBPageSizePalette::selectionChanged);
    }
}

void UBPageSizePalette::selectionChanged()
{
    if (!isVisible())
    {
        return;
    }

    // update buttons and values
    if (mDocument)
    {
        mValue = {false, {}, {}};
        const auto thumbnailScene = mDocument->thumbnailScene();
        const auto count = mDocument->pageCount();

        if (thumbnailScene->selectedItems().count() == 0)
        {
            const auto pageSize = mDocument->proxy()->defaultDocumentSize();
            mFormat = formatFromSize(pageSize, mPortrait);
            mValue = UBSettings::settings()->documentSizes.value(mFormat, {false, pageSize, pageSize / UB_DOTS_PER_MM / 10});
        }
        else
        {
            for (auto thumbnail : thumbnailScene->selectedItems())
            {
                const auto i = dynamic_cast<UBThumbnail*>(thumbnail)->sceneIndex();
                auto pageSize = mDocument->pageSize(i);

                if (mValue.size.isEmpty())
                {
                    mFormat = formatFromSize(pageSize, mPortrait);

                    if (mFormat == DocumentSize::CustomSize)
                    {
                        if (mPortrait)
                        {
                            pageSize.transpose();
                        }

                        mValue = {false, pageSize, pageSize / UB_DOTS_PER_MM / 10};
                    }
                    else
                    {
                        mValue = UBSettings::settings()->documentSizes.value(mFormat, {false, pageSize, pageSize / UB_DOTS_PER_MM / 10});
                    }
                }
                else if (mValue.size != (mPortrait ? pageSize.transposed() : pageSize))
                {
                    mValue = {false, {}, {}};
                    mFormat = DocumentSize::CustomSize;
                    break;
                }
            }
        }

        toolButtonFormat->setChecked(mPortrait);
        toolButtonScope->setChecked(count != 1 && thumbnailScene->selectedItems().count() == count);

        radioButtonImportedSize->setChecked(false);
        bool isRatio{false};

        switch (mFormat)
        {
        case DocumentSize::Ratio4_3:
            radioButton4_3->setChecked(true);
            isRatio = true;
            break;

        case DocumentSize::Ratio16_9:
            radioButton16_9->setChecked(true);
            isRatio = true;
            break;

        case DocumentSize::Ratio16_10:
            radioButton16_10->setChecked(true);
            isRatio = true;
            break;

        case DocumentSize::CustomRatio:
            radioButtonCustomRatio->setChecked(true);
            isRatio = true;
            break;

        case DocumentSize::SizeA4:
            radioButton_A4->setChecked(true);
            break;

        case DocumentSize::SizeA5:
            radioButton_A5->setChecked(true);
            break;

        case DocumentSize::CustomSize:
            radioButtonCustomSize->setChecked(true);
            break;

        case DocumentSize::ImportedSize:
            radioButtonImportedSize->setChecked(true);
            break;
        }

        buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!mValue.size.isEmpty() || mFormat == DocumentSize::ImportedSize);
        updateSpinBoxes(isRatio);
    }
}

void UBPageSizePalette::selectFormat()
{
    mFormat = sender()->property("format").value<DocumentSize::Enum>();
    auto value = UBSettings::settings()->documentSizes.value(mFormat);
    const auto isRatio = mFormat <= DocumentSize::CustomRatio;

    if (value.size.isValid())
    {
        mValue = value;
        mPortrait = !isRatio;
    }
    else if (mValue.isRatio && !value.isRatio)
    {
        // switch from ratio format to custom size
        const auto heightCm = mValue.size.height() / UB_DOTS_PER_MM / 10.;
        value.displayedSize = QSizeF{heightCm * mValue.displayedSize.width() / mValue.displayedSize.height(), heightCm};
        value.size = (value.displayedSize * 10. * UB_DOTS_PER_MM).toSize();
        mValue = value;
    }
    else if (!mValue.isRatio && value.isRatio)
    {
        // switch from size format to custom ratio, invalidate numbers
        mValue.displayedSize = QSizeF{};
        mValue.size = QSize{};
    }

    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!mValue.size.isEmpty() || mFormat == DocumentSize::ImportedSize);

    toolButtonFormat->setChecked(mPortrait);
    updateSpinBoxes(isRatio);
}

void UBPageSizePalette::switchToCustom()
{
    if (!mIsUpdatingSpinBoxes)
    {
        if (mValue.isRatio)
        {
            radioButtonCustomRatio->setChecked(true);
        }
        else
        {
            radioButtonCustomSize->setChecked(true);
        }
    }
}

void UBPageSizePalette::flip(bool portrait)
{
    if (portrait != mPortrait)
    {
        mPortrait = portrait;

        mIsUpdatingSpinBoxes = true;

        const auto tmpX = spinBoxX->value();
        spinBoxX->setValue(spinBoxY->value());
        spinBoxY->setValue(tmpX);

        const auto tmpWidth = doubleSpinBoxWidth->value();
        doubleSpinBoxWidth->setValue(doubleSpinBoxHeight->value());
        doubleSpinBoxHeight->setValue(tmpWidth);

        mIsUpdatingSpinBoxes = false;
    }
}

void UBPageSizePalette::apply()
{
    if (mDocument)
    {
        const auto thumbnailScene = mDocument->thumbnailScene();
        const auto count = mDocument->pageCount();

        auto newSize = mFormat == DocumentSize::ImportedSize ? QSize{0, 0} : mValue.size;

        if (mPortrait)
        {
            newSize.transpose();
        }

        QList<QUuid> pages;

        for (int i = 0; i < count; ++i)
        {
            if (toolButtonScope->isChecked() || thumbnailScene->thumbnailAt(i)->isSelected())
            {
                if (mDocument->pageSize(i) != newSize || mFormat == DocumentSize::ImportedSize)
                {
                    pages << mDocument->toc()->uuid(i);
                }
            }
        }

        if (!pages.isEmpty())
        {
            UBPageSizeUndoCommand* uc = new UBPageSizeUndoCommand(mDocument, pages, newSize);
            mContainer->undoStack()->push(uc);
        }
    }

    hide();
}

void UBPageSizePalette::undo()
{
    auto documentController = dynamic_cast<UBDocumentController*>(mContainer);

    if (documentController)
    {
        // determine affected document and select it after the undo operation
        auto undoStack = mContainer->undoStack();
        auto undoCommand = undoStack->command(undoStack->index() - 1);
        auto pageSizeUndoCommand = dynamic_cast<const UBPageSizeUndoCommand*>(undoCommand);

        undoStack->undo();

        if (pageSizeUndoCommand)
        {
            auto document = pageSizeUndoCommand->document();
            documentController->selectDocument(document->proxy(), true);
        }
    }
    else if (mContainer)
    {
        mContainer->undoStack()->undo();
    }
}

void UBPageSizePalette::redo()
{
    auto documentController = dynamic_cast<UBDocumentController*>(mContainer);

    if (documentController)
    {
        // determine affected document and select it after the redo operation
        auto undoStack = mContainer->undoStack();
        auto undoCommand = undoStack->command(undoStack->index());
        auto pageSizeUndoCommand = dynamic_cast<const UBPageSizeUndoCommand*>(undoCommand);

        undoStack->redo();

        if (pageSizeUndoCommand)
        {
            auto document = pageSizeUndoCommand->document();
            documentController->selectDocument(document->proxy(), true);
        }
    }
    else if (mContainer)
    {
        mContainer->undoStack()->redo();
    }
}

void UBPageSizePalette::updateSpinBoxes(bool isRatio)
{
    mIsUpdatingSpinBoxes = true;

    spinBoxX->setValue(0);
    spinBoxY->setValue(0);
    doubleSpinBoxWidth->setValue(0.);
    doubleSpinBoxHeight->setValue(0.);

    auto displayedSize = mValue.displayedSize;

    if (displayedSize.isValid())
    {
        if (mPortrait)
        {
            displayedSize.transpose();
        }

        if (isRatio)
        {
            spinBoxX->setValue(displayedSize.width());
            spinBoxY->setValue(displayedSize.height());
        }
        else
        {
            doubleSpinBoxWidth->setValue(displayedSize.width());
            doubleSpinBoxHeight->setValue(displayedSize.height());
        }
    }

    // to avoid flicker, first make one invisible, then the other visible
    if (isRatio)
    {
        doubleSpinBoxWidth->setVisible(!isRatio);
        doubleSpinBoxHeight->setVisible(!isRatio);
        labelX->setVisible(!isRatio);

        spinBoxX->setVisible(isRatio);
        spinBoxY->setVisible(isRatio);
        labelColon->setVisible(isRatio);
    }
    else
    {
        spinBoxX->setVisible(isRatio);
        spinBoxY->setVisible(isRatio);
        labelColon->setVisible(isRatio);

        doubleSpinBoxWidth->setVisible(!isRatio);
        doubleSpinBoxHeight->setVisible(!isRatio);
        labelX->setVisible(!isRatio);
    }

    mIsUpdatingSpinBoxes = false;
}

DocumentSize::Enum UBPageSizePalette::formatFromSize(QSize size, bool& portrait)
{
    portrait = size.width() <= size.height();

    if (portrait)
    {
        size.transpose();
    }

    auto docSizes = UBSettings::settings()->documentSizes;

    auto hasSize = [size](std::pair<DocumentSize::Enum, DocumentSize::Value> formatSize){
        return size == formatSize.second.size
                || size == formatSize.second.size.shrunkBy({0, 0, 1, 0})
                || size == formatSize.second.size.shrunkBy({0, 0, 0, 1})
                || size == formatSize.second.size.shrunkBy({0, 0, 1, 1});
    };

    auto found = std::find_if(docSizes.constKeyValueBegin(), docSizes.constKeyValueEnd(), hasSize);

    if (found != docSizes.constKeyValueEnd())
    {
        return found->first;
    }

    return DocumentSize::CustomSize;
}
