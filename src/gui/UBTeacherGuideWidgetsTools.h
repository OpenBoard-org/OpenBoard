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

#ifndef UBTEACHERGUIDEWIDGETSTOOLS_H
#define UBTEACHERGUIDEWIDGETSTOOLS_H

#include <QObject>
#include <QTreeWidgetItem>
#include <QTextEdit>
#include <QLabel>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QLineEdit>
#include <QMimeData>
#include <QStackedWidget>
#include <QWebView>

#include "customWidgets/UBMediaWidget.h"

#define TG_USER_ROLE_MIME_TYPE (Qt::UserRole+50)


class QTreeWidget;
class QVBoxLayout;
class QComboBox;
class QTextEdit;
class QWidget;
class UBTGAdaptableText;
class QDomElement;
class UBMediaWidget;

typedef struct
{
    QString name;
    QMap<QString,QString> attributes;
}tUBGEElementNode;


class iUBTGSaveData
{
public:
    virtual tUBGEElementNode* saveData() = 0;
};


class UBAddItem : public QTreeWidgetItem
{
public:
    explicit UBAddItem(const QString &strings, int addSubItemWidgetType, QTreeWidget* parent = 0);
    ~UBAddItem();

signals:

public slots:
};

class UBTGActionWidget : public QWidget, public iUBTGSaveData
{
    Q_OBJECT

public:
    explicit UBTGActionWidget(QTreeWidgetItem* widget, QWidget* parent = 0,const char* name = "UBTGActionWidget");
    ~UBTGActionWidget();
    void update();
    tUBGEElementNode* saveData();
    void initializeWithDom(QDomElement element);

private:
    QVBoxLayout* mpLayout;
    QComboBox* mpOwner;
    UBTGAdaptableText* mpTask;

protected:
    QTreeWidgetItem* mpTreeWidgetItem;
};


class UBTGAdaptableText : public QTextEdit
{
    Q_OBJECT
public:
    explicit UBTGAdaptableText(QTreeWidgetItem* widget = 0, QWidget *parent = 0, const char* name = "UBTGAdaptableText");
    void showText(const QString & text);
    void bottomMargin(int newValue);
    void setPlaceHolderText(QString text);
    QString text();
    void setInitialText(const QString& text);
    void setMaximumLength(int length);

public slots:
    void onTextChanged();

protected:
    void keyPressEvent(QKeyEvent* e);
    void keyReleaseEvent(QKeyEvent* e);
    void showEvent(QShowEvent* e);

private:
    int mBottomMargin;
    QTreeWidgetItem* mpTreeWidgetItem;
    int mMinimumHeight;
    bool mHasPlaceHolder;
    QString mPlaceHolderText;
    bool mIsUpdatingSize;
    int mMaximumLength;
};


class UBDraggableWeb : public QWebView
{
    Q_OBJECT
public:
    explicit UBDraggableWeb(QString& relativePath, QWidget* parent = 0);

private:
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);

    QString mRelativePath;
    QPoint mDragStartPosition;
    bool mDragStarted;
};

class UBTGMediaWidget : public QStackedWidget , public iUBTGSaveData
{
    Q_OBJECT
public:
    UBTGMediaWidget(QTreeWidgetItem* widget = 0, QWidget* parent = 0, const char* name = "UBTGMediaWidget");
    UBTGMediaWidget(QString mediaPath, QTreeWidgetItem* widget = 0, QWidget* parent = 0, bool forceFlashMediaType = false, const char *name = "UBTGMediaWidget");
    ~UBTGMediaWidget();
    tUBGEElementNode* saveData();
    void initializeWithDom(QDomElement element);
    void removeSource();

protected:
    void dragEnterEvent(QDragEnterEvent* event);
    void dropEvent(QDropEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void hideEvent(QHideEvent* event);
    void showEvent(QShowEvent* event);

private:
    void parseMimeData(const QMimeData* pMimeData);
    void createWorkWidget(bool forceFlashMediaType = false);
    void updateSize();

    QTreeWidgetItem* mpTreeWidgetItem;
    QLabel* mpDropMeWidget;
    QWidget* mpWorkWidget;
    QVBoxLayout* mpLayout;
    UBTGAdaptableText* mpTitle;
    QLabel* mpMediaLabelWidget;
    UBMediaWidget* mpMediaWidget;
    UBDraggableWeb* mpWebView;
    QString mMediaPath;
    bool mIsPresentationMode;
    QString mMediaType;
    bool mIsInitializationMode;
};


class UBTGUrlWidget : public QWidget , public iUBTGSaveData
{
    Q_OBJECT
public:
    UBTGUrlWidget(QWidget* parent = 0, const char* name = "UBTGUrlWidget");
    ~UBTGUrlWidget();
    tUBGEElementNode* saveData();
    void initializeWithDom(QDomElement element);

public slots:
    void onUrlEditionFinished();

private:
    QVBoxLayout* mpLayout;
    QLineEdit* mpTitle;
    QLineEdit* mpUrl;
};

class UBTGDraggableTreeItem : public QTreeWidget
{
    Q_OBJECT
public:
    UBTGDraggableTreeItem(QWidget* parent = 0, const char* name = "UBTGDraggableTreeItem");

private:
    QMimeData* mimeData(const QList<QTreeWidgetItem *> items) const;
};

#endif // UBTEACHERGUIDEWIDGETSTOOLS_H
