/*
 * Copyright (C) 2015-2024 Département de l'Instruction Publique (DIP-SEM)
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



#pragma once

#include "gui/UBBackgroundRuling.h"

#include <QObject>

// forward
class QAction;
class QDomDocument;

class UBBackgroundManager : public QObject
{
    Q_OBJECT

public:
    explicit UBBackgroundManager(QObject* parent = nullptr);

    void scan();
    void addBackground(UBBackgroundRuling& background);
    void deleteBackground(const QUuid& uuid);
    const QList<const UBBackgroundRuling*> backgrounds() const;
    QAction* backgroundAction(const UBBackgroundRuling& background, bool dark);
    void updateAction(QAction* action, bool dark) const;
    const UBBackgroundRuling* background(const QUuid& uuid) const;
    const UBBackgroundRuling* guessBackground(bool crossed, bool ruled, bool intermediateLines) const;
    QPixmap createButtonPixmap(const UBBackgroundRuling& background, bool dark, bool on) const;
    QIcon createButtonIcon(const UBBackgroundRuling& background, bool dark) const;
    void savePreferredBackgrounds(QList<QUuid>& uuidList);

signals:
    void preferredBackgroundChanged();
    void backgroundListChanged();

private:
    void scan(const QString& dirname, bool userProvided);
    QByteArray renderToSvg(const UBBackgroundRuling& background, bool dark) const;
    QPixmap createButtonPixmap(const QDomDocument& bgDoc, bool dark, bool on) const;

private:
    QList<UBBackgroundRuling> mBackgrounds;
};
