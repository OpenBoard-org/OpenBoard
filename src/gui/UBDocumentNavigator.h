#ifndef UBDOCUMENTNAVIGATOR_H
#define UBDOCUMENTNAVIGATOR_H

#include <QResizeEvent>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QThread>

#include "document/UBDocumentProxy.h"
#include "UBThumbnailWidget.h"

#define NO_PAGESELECTED		    -1

class UBDocumentNavigator : public QGraphicsView
{
    Q_OBJECT
public:
    UBDocumentNavigator(QWidget* parent=0, const char* name="documentNavigator");
    ~UBDocumentNavigator();

    void setDocument(UBDocumentProxy* document);
    void setNbColumns(int nbColumns);
    int nbColumns();
    void setThumbnailMinWidth(int width);
    int thumbnailMinWidth();
    int selectedPageNumber();
    UBDocumentProxy* currentDoc();

signals:
    void changeCurrentPage();

protected:
    virtual void resizeEvent(QResizeEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);

private slots:
    void addNewPage();
    void onSelectionChanged();

private:
    void setGraphicsItems(QList<QGraphicsItem*> items, QStringList labels);
    void refreshScene();
    void updateSpecificThumbnail(int iPage);
    int border();
    void generateThumbnails();

    /** The scene */
    QGraphicsScene* mScene;
    /** The current selected item */
    UBSceneThumbnailPixmap* mCrntItem;
    /** The current document */
    UBDocumentProxy* mCrntDoc;
    /** The list of current thumbnails */
    QList<QGraphicsItem*> mThumbnails;
    /** The list of current label items */
    QList<UBThumbnailTextItem*> mLabels;
    /** The list of current labels */
    QStringList mLab;
    /** The current number of columns */
    int mNbColumns;
    /** The current thumbnails width */
    int mThumbnailWidth;
    /** The current thumbnails minimum width */
    int mThumbnailMinWidth;
    /** A flag indicating that a thumbnail refresh is in progress */
    bool bNavig;
};

#endif // UBDOCUMENTNAVIGATOR_H
