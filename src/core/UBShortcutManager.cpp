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

#include "UBShortcutManager.h"
#include "core/UBSettings.h"
#include "frameworks/UBPlatformUtils.h"
#include "gui/UBMainWindow.h"

#include <QAction>
#include <QDebug>
#include <QWidget>

// property names
static const char* defaultShortcutProperty("defaultShortcut");
static const char* descriptionProperty("description");
static const char* mouseButtonProperty("mouseButton");
static const char* tabletButtonProperty("tabletButton");

UBShortcutManager* UBShortcutManager::sShortcutManager = nullptr;

UBShortcutManager::UBShortcutManager() : mIgnoreCtrl(false)
{
    actionsOfGroup(tr("Common"));
}

UBShortcutManager *UBShortcutManager::shortcutManager()
{
    if (!sShortcutManager)
    {
        sShortcutManager = new UBShortcutManager;
    }

    return sShortcutManager;
}

void UBShortcutManager::addActions(const QString& group, const QList<QAction*> actions, QWidget *widget)
{
    // save default shortcuts for later
    for (QAction* action : actions)
    {
        if (!action->isSeparator())
        {
            if (widget && !widget->actions().contains(action))
            {
                // associate actions with widget to make sure they are triggered when this widget is visible
                widget->addAction(action);
            }

            QKeySequence shortcut = action->shortcut();

            if (!shortcut.isEmpty())
            {
                action->setProperty(defaultShortcutProperty, shortcut.toString());
            }

            action->setProperty(descriptionProperty, action->toolTip());

            QStringList settings = UBSettings::settings()->value("Shortcut/" + action->objectName()).toStringList();

            if (settings.size() == 3)
            {
                if (!settings[0].isEmpty())
                {
                    action->setShortcut(settings[0]);
                }

                if (int button = settings[1].toInt())
                {
                    action->setProperty(mouseButtonProperty, button);
                    mMouseActions[static_cast<Qt::MouseButton>(button)] = action;
                }

                if (int button = settings[2].toInt())
                {
                    action->setProperty(tabletButtonProperty, button);
                    mTabletActions[static_cast<Qt::MouseButton>(button)] = action;
                }
            }

            QString oldGroup = groupOfAction(action);

            if (oldGroup.isEmpty()) {
                actionsOfGroup(group) << action;
            }
            else
            {
                // remove from oldGroup, add to Common, which is always first group
                actionsOfGroup(oldGroup).removeAll(action);
                mActionGroups[0].second << action;
            }
        }
    }
}

void UBShortcutManager::addMainActions(UBMainWindow *mainWindow)
{
    addActions(tr("Common"), {
                   mainWindow->actionStylus,
                   mainWindow->actionBoard,
                   mainWindow->actionWeb,
                   mainWindow->actionDocument,
                   mainWindow->actionDesktop,
                   mainWindow->actionLibrary,
                   mainWindow->actionVirtualKeyboard,
                   mainWindow->actionOpenTutorial,
                   mainWindow->actionHideApplication,
                   mainWindow->actionCut,
                   mainWindow->actionCopy,
                   mainWindow->actionPaste,
                   mainWindow->actionQuit
               }, mainWindow);

    addActions(tr("Board"), {
                   mainWindow->actionUndo,
                   mainWindow->actionRedo,
                   mainWindow->actionNewPage,
                   mainWindow->actionDuplicatePage,
                   mainWindow->actionImportPage,
                   mainWindow->actionBack,
                   mainWindow->actionForward,
                   mainWindow->actionAdd,
                   mainWindow->actionClearPage,
                   mainWindow->actionEraseItems,
                   mainWindow->actionEraseAnnotations,
                   mainWindow->actionEraseBackground
               }, mainWindow);

    addActions(tr("Stylus Palette"),{
                   mainWindow->actionPen,
                   mainWindow->actionEraser,
                   mainWindow->actionMarker,
                   mainWindow->actionSelector,
                   mainWindow->actionPlay,

                   mainWindow->actionHand,
                   mainWindow->actionZoomIn,
                   mainWindow->actionZoomOut,

                   mainWindow->actionPointer,
                   mainWindow->actionLine,
                   mainWindow->actionText,
                   mainWindow->actionCapture
               }, mainWindow);

    if(UBPlatformUtils::hasVirtualKeyboard())
    {
        addActions(tr("Stylus Palette"),{ mainWindow->actionVirtualKeyboard }, mainWindow);
    }

    addActions(tr("Stylus Palette"),{ mainWindow->actionSnap }, mainWindow);

    addActions(tr("Lines and colours"), {
                   mainWindow->actionLineSmall,
                   mainWindow->actionLineMedium,
                   mainWindow->actionLineLarge,
                   mainWindow->actionEraserSmall,
                   mainWindow->actionEraserMedium,
                   mainWindow->actionEraserLarge,
                   mainWindow->actionColor0,
                   mainWindow->actionColor1,
                   mainWindow->actionColor2,
                   mainWindow->actionColor3,
                   mainWindow->actionColor4
               }, mainWindow);

    addActions(tr("Background"), {
                   mainWindow->actionBackgrounds,
                   mainWindow->actionPlainLightBackground,
                   mainWindow->actionCrossedLightBackground,
                   mainWindow->actionRuledLightBackground,
                   mainWindow->actionPlainDarkBackground,
                   mainWindow->actionCrossedDarkBackground,
                   mainWindow->actionRuledDarkBackground,
                   mainWindow->actionDefaultGridSize,
                   mainWindow->actionDrawIntermediateGridLines
               }, mainWindow);

    addActions(tr("Podcast"), {
                   mainWindow->actionPodcastRecord //,
                   // mainWindow->actionPodcastPause currently not activated in UBPodcastRecordingPalette
               }, mainWindow);

    // add builtIn actions
    QList<QAction*> actions;

    QAction* action = new QAction(this);
    action->setText(mainWindow->actionBack->text());
    action->setToolTip(mainWindow->actionBack->toolTip());
    action->setShortcuts( { QKeySequence(Qt::Key_Up), QKeySequence(Qt::Key_PageUp), QKeySequence(Qt::Key_Left) } );
    action->setProperty("builtIn", true);
    actions << action;

    action = new QAction(this);
    action->setText(mainWindow->actionForward->text());
    action->setToolTip(mainWindow->actionForward->toolTip());
    action->setShortcuts( { QKeySequence(Qt::Key_Down), QKeySequence(Qt::Key_PageDown), QKeySequence(Qt::Key_Right), QKeySequence(Qt::Key_Space) } );
    action->setProperty("builtIn", true);
    actions << action;

    action = new QAction(this);
    action->setText(tr("First scene"));
    action->setToolTip(tr("Show first scene"));
    action->setShortcuts( { QKeySequence(Qt::Key_Home) } );
    action->setProperty("builtIn", true);
    actions << action;

    action = new QAction(this);
    action->setText(tr("Last scene"));
    action->setToolTip(tr("Show last scene"));
    action->setShortcuts( { QKeySequence(Qt::Key_End) } );
    action->setProperty("builtIn", true);
    actions << action;

    action = new QAction(this);
    action->setText(mainWindow->actionNewPage->text());
    action->setToolTip(mainWindow->actionNewPage->toolTip());
    action->setShortcuts( { QKeySequence(Qt::Key_Insert) } );
    action->setProperty("builtIn", true);
    actions << action;

    action = new QAction(this);
    action->setText(mainWindow->actionZoomIn->text());
    action->setToolTip(mainWindow->actionZoomIn->toolTip());
    action->setShortcuts( { QKeySequence(Qt::CTRL | Qt::Key_Plus) } );
    action->setProperty("builtIn", true);
    actions << action;

    action = new QAction(this);
    action->setText(mainWindow->actionZoomOut->text());
    action->setToolTip(mainWindow->actionZoomOut->toolTip());
    action->setShortcuts( { QKeySequence(Qt::CTRL | Qt::Key_Minus) } );
    action->setProperty("builtIn", true);
    actions << action;

    action = new QAction(this);
    action->setText(tr("Zoom reset"));
    action->setToolTip(tr("Reset zoom factor"));
    action->setShortcuts( { QKeySequence(Qt::CTRL | Qt::Key_0) } );
    action->setProperty("builtIn", true);
    actions << action;

    action = new QAction(this);
    action->setText(tr("Scroll left"));
    action->setToolTip(tr("Scroll page left"));
    action->setShortcuts( { QKeySequence(Qt::CTRL | Qt::Key_Left) } );
    action->setProperty("builtIn", true);
    actions << action;

    action = new QAction(this);
    action->setText(tr("Scroll right"));
    action->setToolTip(tr("Scroll page right"));
    action->setShortcuts( { QKeySequence(Qt::CTRL | Qt::Key_Right) } );
    action->setProperty("builtIn", true);
    actions << action;

    action = new QAction(this);
    action->setText(tr("Scroll up"));
    action->setToolTip(tr("Scroll page up"));
    action->setShortcuts( { QKeySequence(Qt::CTRL | Qt::Key_Up) } );
    action->setProperty("builtIn", true);
    actions << action;

    action = new QAction(this);
    action->setText(tr("Scroll down"));
    action->setToolTip(tr("Scroll page down"));
    action->setShortcuts( { QKeySequence(Qt::CTRL | Qt::Key_Down) } );
    action->setProperty("builtIn", true);
    actions << action;

    addActions(tr("Built-in (not editable)"), actions);

    // load ignoreCtrl setting
    ignoreCtrl(UBSettings::settings()->value("Shortcut/IgnoreCtrl").toBool());
}

void UBShortcutManager::addActionGroup(QActionGroup *actionGroup)
{
    mActionGroupHistoryMap[actionGroup] = new UBActionGroupHistory(actionGroup);
}

void UBShortcutManager::removeActionGroup(QActionGroup *actionGroup)
{
    if (mActionGroupHistoryMap.contains(actionGroup))
    {
        delete mActionGroupHistoryMap[actionGroup];
        mActionGroupHistoryMap.remove(actionGroup);
    }
}

bool UBShortcutManager::handleMouseEvent(QMouseEvent *event)
{
    if (mMouseActions.contains(event->button()))
    {
        QAction* action = mMouseActions[event->button()];

        if (!action->isCheckable() || !action->actionGroup() || !action->isChecked())
        {
            action->trigger();
        }

        return true;
    }

    return false;
}

bool UBShortcutManager::handleTabletEvent(QTabletEvent *event)
{
    if (mTabletActions.contains(event->button()))
    {
        QAction* action = mTabletActions[event->button()];

        if (!action->isCheckable() || !action->actionGroup() || !action->isChecked())
        {
            action->trigger();
        }

        return true;
    }

    return false;
}

bool UBShortcutManager::handleKeyReleaseEvent(QKeyEvent *event)
{
    for (UBActionGroupHistory* actionGroupHistory : mActionGroupHistoryMap.values())
    {
        if (actionGroupHistory->keyReleased(event)) {
            return true;
        }
    }

    return false;
}

int UBShortcutManager::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    int rows = 0;

    for (auto& actionGroup : mActionGroups)
    {
        ++rows;
        rows += actionGroup.second.size();
    }

    return rows;
}

int UBShortcutManager::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 5;
}

QVariant UBShortcutManager::data(const QModelIndex &index, int role) const
{
    QString group;
    QAction* action = getAction(index, &group);

    switch (role)
    {
    case Qt::DisplayRole:
    case Qt::ToolTipRole:
        switch (index.column())
        {
        case 0:
            return action ? action->text() : group;

        case 1:
            return action ? action->property(descriptionProperty) : "";

        case 2:
        {
            if (!action)
            {
                return "";
            }

            QStringList result;

            for (const QKeySequence& shortcut : action->shortcuts())
            {
                result << shortcut.toString();
            }

            return result.join(", ");
        }

        case 3:
            return action ? buttonName(static_cast<Qt::MouseButton>(action->property(mouseButtonProperty).toInt())) : QVariant();

        case 4:
            return action ? buttonName(static_cast<Qt::MouseButton>(action->property(tabletButtonProperty).toInt())) : QVariant();
        }
        break;

    case Qt::FontRole:
    {
        QFont groupFont;
        groupFont.setBold(true);
        groupFont.setItalic(true);

        QFont disabledFont;
        disabledFont.setItalic(true);

        return action ? (action->property("builtIn").toBool() ? disabledFont : QVariant()) : groupFont;
    }

    case UBShortcutManager::ActionRole:
        return action && !action->property("builtIn").toBool();

    case UBShortcutManager::GroupHeaderRole:
        return !action;

    case UBShortcutManager::PrimaryShortcutRole:
        return (index.column() == 2 && action) ? action->shortcut().toString() : QVariant();

    case Qt::DecorationRole:
    case Qt::EditRole:
    case Qt::StatusTipRole:
    case Qt::WhatsThisRole:
    case Qt::SizeHintRole:
    case Qt::TextAlignmentRole:
    case Qt::BackgroundRole:
    case Qt::ForegroundRole:
    case Qt::CheckStateRole:
    case Qt::InitialSortOrderRole:
        return QVariant();
    }

    return QVariant();
}

QVariant UBShortcutManager::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Orientation::Horizontal && role == Qt::DisplayRole)
    {
        switch (section)
        {
        case 0:
            return tr("Command");

        case 1:
            return tr("Description");

        case 2:
            return tr("Key Sequence");

        case 3:
            return tr("Mouse Button");

        case 4:
            return tr("Tablet Button");
        }
    }

    return QVariant();
}

bool UBShortcutManager::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_UNUSED(role)

    if (!index.isValid())
    {
        return false;
    }

    QAction* action = getAction(index);

    switch (index.column())
    {
    case 2:
    {
        QKeySequence keySequence(value.toString());

        action->setShortcut(keySequence);
        updateSettings(action);
        emit dataChanged(index, index);
        return true;
    }

    case 3:
        action->setProperty(mouseButtonProperty, value);

        for (Qt::MouseButton key : mMouseActions.keys())
        {
            if (mMouseActions[key] == action)
            {
                mMouseActions.remove(key);
                break;
            }
        }

        mMouseActions[static_cast<Qt::MouseButton>(value.toInt())] = action;
        updateSettings(action);
        emit dataChanged(index, index);
        return true;

    case 4:
        action->setProperty(tabletButtonProperty, value);

        for (Qt::MouseButton key : mTabletActions.keys())
        {
            if (mTabletActions[key] == action)
            {
                mTabletActions.remove(key);
                break;
            }
        }

        mTabletActions[static_cast<Qt::MouseButton>(value.toInt())] = action;
        updateSettings(action);
        emit dataChanged(index, index);
        return true;
    }

    return false;
}

bool UBShortcutManager::resetData(const QModelIndex &index)
{
    QAction* action = getAction(index);

    if (action)
    {
        QKeySequence shortcut(action->property(defaultShortcutProperty).toString());
        action->setShortcut(shortcut);
        mMouseActions.remove(static_cast<Qt::MouseButton>(action->property(mouseButtonProperty).toInt()));
        mTabletActions.remove(static_cast<Qt::MouseButton>(action->property(tabletButtonProperty).toInt()));
        action->setProperty(mouseButtonProperty, QVariant());
        action->setProperty(tabletButtonProperty, QVariant());
        updateSettings(action);
        emit dataChanged(index.siblingAtColumn(2), index.siblingAtColumn(4));
        return true;
    }

    return false;
}

bool UBShortcutManager::checkData(const QModelIndex &index, const QVariant &value) const
{
    int col = index.column();

    if (col < 2)
    {
        return true;
    }

    for (int row = 0; row < rowCount(); ++row)
    {
        if (data(index.siblingAtRow(row)).toString().split(", ").contains(value.toString()))
        {
            // duplicate value
            return false;
        }
    }

    return true;
}

bool UBShortcutManager::hasCtrlConflicts(const QKeySequence &additionalShortcut) const
{
    QSet<QKeySequence> shortcuts;
    QSet<QKeySequence> ctrlShortcuts;

    for (int row = 1; row < rowCount(); ++row)
    {
        QAction* action = getAction(index(row, 0));

        if (!action)
        {
            continue;
        }

        if (action->property("builtIn").toBool())
        {
            for (const QKeySequence& shortcut : action->shortcuts())
            {
                shortcuts << shortcut;
            }
        }
        else
        {
            shortcuts << action->shortcut();
            ctrlShortcuts << QKeySequence(action->shortcut()[0] ^ Qt::CTRL);
        }

    }

    if (!additionalShortcut.isEmpty())
    {
        shortcuts << additionalShortcut;
        ctrlShortcuts << QKeySequence(additionalShortcut[0] ^ Qt::CTRL);
    }

    return shortcuts.intersects(ctrlShortcuts);
}

QString UBShortcutManager::buttonName(Qt::MouseButton button)
{
    switch (button)
    {
    case Qt::LeftButton: return tr("Left", "MouseButton");
    case Qt::RightButton: return tr("Right", "MouseButton");
    case Qt::MiddleButton: return tr("Middle", "MouseButton");
    case Qt::BackButton: return tr("Back", "MouseButton");
    case Qt::ForwardButton: return tr("Forward", "MouseButton");
    case Qt::TaskButton: return tr("Task", "MouseButton");
    case Qt::ExtraButton4: return tr("Extra", "MouseButton") + "4";
    case Qt::ExtraButton5: return tr("Extra", "MouseButton") + "5";
    case Qt::ExtraButton6: return tr("Extra", "MouseButton") + "6";
    case Qt::ExtraButton7: return tr("Extra", "MouseButton") + "7";
    case Qt::ExtraButton8: return tr("Extra", "MouseButton") + "8";
    case Qt::ExtraButton9: return tr("Extra", "MouseButton") + "9";
    case Qt::ExtraButton10: return tr("Extra", "MouseButton") + "10";
    case Qt::ExtraButton11: return tr("Extra", "MouseButton") + "11";
    case Qt::ExtraButton12: return tr("Extra", "MouseButton") + "12";
    case Qt::ExtraButton13: return tr("Extra", "MouseButton") + "13";
    case Qt::ExtraButton14: return tr("Extra", "MouseButton") + "14";
    case Qt::ExtraButton15: return tr("Extra", "MouseButton") + "15";
    case Qt::ExtraButton16: return tr("Extra", "MouseButton") + "16";
    case Qt::ExtraButton17: return tr("Extra", "MouseButton") + "17";
    case Qt::ExtraButton18: return tr("Extra", "MouseButton") + "18";
    case Qt::ExtraButton19: return tr("Extra", "MouseButton") + "19";
    case Qt::ExtraButton20: return tr("Extra", "MouseButton") + "20";
    case Qt::ExtraButton21: return tr("Extra", "MouseButton") + "21";
    case Qt::ExtraButton22: return tr("Extra", "MouseButton") + "22";
    case Qt::ExtraButton23: return tr("Extra", "MouseButton") + "23";
    case Qt::ExtraButton24: return tr("Extra", "MouseButton") + "24";
    default: return "";
    }
}

Qt::MouseButton UBShortcutManager::buttonIndex(QString button)
{
    for (unsigned int index = Qt::LeftButton; index < Qt::AllButtons; index <<= 1)
    {
        Qt::MouseButton but = static_cast<Qt::MouseButton>(index);

        if (button == buttonName(but))
        {
            return but;
        }
    }

    return Qt::NoButton;
}

void UBShortcutManager::ignoreCtrl(bool ignore)
{
    if (ignore == mIgnoreCtrl) {
        return;
    }

    mIgnoreCtrl = ignore;

    for (auto& actionGroup : mActionGroups)
    {
        for (QAction* action : actionGroup.second)
        {
            if (!action->property("builtIn").toBool())
            {
                QList<QKeySequence> shortcuts = action->shortcuts();

                if (ignore && !shortcuts.empty() && shortcuts[0][0] & Qt::CTRL) {
                    QKeySequence noCtrl(shortcuts[0][0] ^ Qt::CTRL);
                    shortcuts << noCtrl;
                    action->setShortcuts(shortcuts);
                }
                else if (!ignore && shortcuts.size() > 1)
                {
                    action->setShortcuts( { shortcuts[0] } );
                }
            }
        }
    }

    UBSettings::settings()->setValue("Shortcut/IgnoreCtrl", ignore);
    emit dataChanged(index(0, 2), index(rowCount(), 2));
}

QString UBShortcutManager::groupOfAction(const QAction *action) const
{
    for (auto& actionGroup : mActionGroups)
    {
        for (QAction* actionInGroup : actionGroup.second)
        {
            if (action == actionInGroup)
            {
                return actionGroup.first;
            }
        }
    }

    return QString();
}

QList<QAction *> &UBShortcutManager::actionsOfGroup(const QString &group)
{
    for (auto& actionGroup : mActionGroups)
    {
        if (actionGroup.first == group) {
            return actionGroup.second;
        }
    }

    QPair<QString,QList<QAction*>> actionGroup;
    actionGroup.first = group;
    mActionGroups << actionGroup;

    return mActionGroups.last().second;
}

QAction *UBShortcutManager::getAction(const QModelIndex &index, QString *group) const
{
    int row = index.row();

    for (auto& actionGroup : mActionGroups)
    {
        if (row == 0)
        {
            if (group)
            {
                *group = actionGroup.first;
            }

            return nullptr;
        }

        if (row <= actionGroup.second.size())
        {
            if (group)
            {
                *group = actionGroup.first;
            }

            return actionGroup.second[row - 1];
        }

        --row;
        row -= actionGroup.second.size();
    }

    if (group)
    {
        group->clear();
    }

    return nullptr;
}

void UBShortcutManager::updateSettings(const QAction *action) const
{
    QString key = "Shortcut/" + action->objectName();
    QString keySequence = action->shortcut().toString();
    QString defaultSequence = action->property(defaultShortcutProperty).toString();
    int mouseButton = action->property(mouseButtonProperty).toInt();
    int tabletButton = action->property(tabletButtonProperty).toInt();

    if (keySequence == defaultSequence && mouseButton == 0 && tabletButton == 0)
    {
        // back to default, delete settings
        UBSettings::settings()->setValue(key, QVariant());
    }
    else
    {
        QStringList list;
        list << keySequence;
        list << QString("%1").arg(mouseButton);
        list << QString("%1").arg(tabletButton);
        UBSettings::settings()->setValue(key, list);
    }
}

// ---------- UBActionGroupHistory ----------

UBActionGroupHistory::UBActionGroupHistory(QActionGroup *parent)
    : QObject(parent)
    , mActionGroup(parent)
    , mCurrentAction(parent->checkedAction())
    , mPreviousAction(nullptr)
    , mRevertingAction(nullptr)
{
    connect(parent, &QActionGroup::triggered, this, &UBActionGroupHistory::triggered);
}

void UBActionGroupHistory::triggered(QAction *action)
{
    if (mCurrentAction != action)
    {
        mPreviousAction = mCurrentAction;
        mCurrentAction = action;
    }
}

bool UBActionGroupHistory::keyReleased(QKeyEvent *event)
{
    int key = event->key() & ~Qt::KeyboardModifierMask;

    for (QAction* action : mActionGroup->actions())
    {
        QKeySequence keySequence = action->shortcut();

        if (keySequence.count() > 0)
        {
            int actionKey = action->shortcut()[0] & ~Qt::KeyboardModifierMask;

            if (key == actionKey)
            {
                if (event->isAutoRepeat())
                {
                    if (!mRevertingAction)
                    {
                        mRevertingAction = mPreviousAction;;
                    }
                }
                else if (mRevertingAction)
                {
                    mRevertingAction->trigger();
                    mRevertingAction = nullptr;
                }

                return true;
            }
        }
    }

    return false;
}
