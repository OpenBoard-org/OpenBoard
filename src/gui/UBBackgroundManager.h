#pragma once

#include "gui/UBBackgroundRuling.h"

#include <QObject>

// forward
class QAction;

class UBBackgroundManager : public QObject
{
    Q_OBJECT
public:
    explicit UBBackgroundManager(QObject *parent = nullptr);

    void scan();
    void addBackground(UBBackgroundRuling& background);
    const QList<UBBackgroundRuling>& backgrounds() const;
    QAction* backgroundAction(const UBBackgroundRuling& background, bool dark);
    void updateAction(QAction* action, bool dark) const;
    const UBBackgroundRuling* background(const QUuid& uuid) const;
    const UBBackgroundRuling* guessBackground(bool crossed, bool ruled, bool intermediateLines) const;

private:
    void scan(const QString& dirname);
    QByteArray renderToSvg(const UBBackgroundRuling& background, bool dark) const;
    QPixmap createButtonPixmap(const UBBackgroundRuling& background, bool dark, bool on) const;
    QIcon createButtonIcon(const UBBackgroundRuling& background, bool dark) const;

private:
    QList<UBBackgroundRuling> mBackgrounds;
};

