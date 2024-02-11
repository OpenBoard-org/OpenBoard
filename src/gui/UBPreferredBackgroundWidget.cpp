#include "UBPreferredBackgroundWidget.h"

#include "board/UBBoardController.h"
#include "core/UBApplication.h"
#include "gui/UBBackgroundManager.h"
#include "gui/UBFlowLayout.h"



UBPreferredBackgroundWidget::UBPreferredBackgroundWidget(QWidget* parent)
    : QWidget(parent)
{
    setAcceptDrops(true);

    mLayout = new UBFlowLayout(this, -1, 0, 0);
    mLayout->setSizeConstraint(QLayout::SetMaximumSize);
    setLayout(mLayout);

    auto* bgManager = UBApplication::boardController->backgroundManager();
    const auto backgrounds = bgManager->backgrounds();

    for (const auto& background : backgrounds)
    {
        auto pixmap = bgManager->createButtonPixmap(*background, false, false);
        auto text = background->description(QLocale().name().left(2));
        auto* label = new BackgroundWidget{pixmap, text, this};
        label->setProperty("uuid", background->uuid());
        label->setToolTip(text);

        mLayout->addWidget(label);
    }

    mPlaceholder = new QLabel(this);
    QPixmap pixmap(QSize{128, 128});
    pixmap.fill(QColor(Qt::transparent));

    QPainter painter(&pixmap);
    QPen pen = painter.pen();
    pen.setWidth(4);
    pen.setColor(QColor(0x888888));
    pen.setStyle(Qt::DashLine);
    painter.setPen(pen);
    painter.drawRoundedRect(6, 20, 115, 88, 15, 15);

    mPlaceholder->setPixmap(pixmap);
    mPlaceholder->setVisible(false);

    updateStyle();
}

void UBPreferredBackgroundWidget::dragEnterEvent(QDragEnterEvent* event)
{
    event->accept();
    QWidget::dragEnterEvent(event);
}

void UBPreferredBackgroundWidget::dragMoveEvent(QDragMoveEvent* event)
{
    QScrollArea* scrollArea = dynamic_cast<QScrollArea*>(parentWidget()->parentWidget());

    if (scrollArea)
    {
        QPoint p = mapFromGlobal(event->pos());
        p = event->pos();
        scrollArea->ensureVisible(p.x(), p.y(), 15);
    }

    QWidget* child = childAt(event->pos());

    while (child && child->parentWidget() != this)
    {
        child = child->parentWidget();
    }

    if (!child)
    {
        return;
    }

    //is the child the placeholder label?
    if (child == mPlaceholder)
    {
        return;
    }

    // move the placeholder to the appropiate place
    auto pl = mLayout->indexOf(mPlaceholder);
    auto ix = mLayout->indexOf(child);

    if (pl >= 0 && ix >= 0)
    {
        QLayoutItem* plItem = mLayout->takeAt(pl);

        mLayout->insertItem(ix, plItem);
        updateStyle();
    }
}

void UBPreferredBackgroundWidget::dropEvent(QDropEvent* event)
{
    QWidget* child = childAt(event->pos());

    // child must be placeholder
    if (child != mPlaceholder)
    {
        return;
    }

    auto ix = mLayout->indexOf(child);

    if (ix < 0)
    {
        return;
    }

    mLayout->replaceAt(ix, mDraggedItem);
    mPlaceholder->setVisible(false);
    updateStyle();

    event->acceptProposedAction();
}

void UBPreferredBackgroundWidget::mousePressEvent(QMouseEvent* event)
{
    auto* child = childAt(event->pos());

    while (child && child->parentWidget() != this)
    {
        child = child->parentWidget();
    }

    auto* backgroundWidget = dynamic_cast<BackgroundWidget*>(child);

    if (!backgroundWidget)
    {
        return;
    }

    mDraggedItemPos = mLayout->indexOf(backgroundWidget);
    QLayoutItem* item = new QWidgetItem(mPlaceholder);
    mDraggedItem = mLayout->replaceAt(mDraggedItemPos, item);
    mPlaceholder->setVisible(true);
    mDraggedItem->widget()->setVisible(false);

    QPixmap pixmap = backgroundWidget->pixmap();

    QMimeData* mimeData = new QMimeData;
    QDrag* drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->setPixmap(pixmap);
    drag->setHotSpot(event->pos() - backgroundWidget->pos());
    const auto dropAction = drag->exec(Qt::MoveAction);

    if (dropAction != Qt::MoveAction)
    {
        // abort DnD and put dragged item at original position
        if (mDraggedItem && mDraggedItemPos >= 0)
        {
            auto ix = mLayout->indexOf(mPlaceholder);

            if (ix >= 0)
            {
                mLayout->takeAt(ix);
                mLayout->insertItem(mDraggedItemPos, mDraggedItem);
                mPlaceholder->setVisible(false);
            }
        }
    }
    else
    {
        savePreferrdBackgrounds();
    }

    if (mDraggedItem && mDraggedItem->widget())
    {
        mDraggedItem->widget()->setVisible(true);
    }
}

void UBPreferredBackgroundWidget::updateStyle() const
{
    for (int i = 0; i < mLayout->count(); ++i)
    {
        QWidget* widget = mLayout->itemAt(i)->widget();
        QLabel* icon = widget->findChild<QLabel*>("backgroundIcon");

        if (icon)
        {
            icon->setStyleSheet(i < 6 ? "background-color: #bbddff" : "");
        }
    }
}

void UBPreferredBackgroundWidget::savePreferrdBackgrounds() const
{
    QList<QUuid> uuidList;

    for (int i = 0; i < mLayout->count() && i < 6; ++i)
    {
        uuidList << mLayout->itemAt(i)->widget()->property("uuid").toUuid();
    }

    UBApplication::boardController->backgroundManager()->savePreferredBackgrounds(uuidList);
}

UBPreferredBackgroundWidget::BackgroundWidget::BackgroundWidget(const QPixmap& pixmap, const QString& text, QWidget* parent)
    : QWidget{parent}
    , mPixmap{pixmap}
{
    QLabel* icon = new QLabel{this};
    icon->setPixmap(pixmap);
    icon->setAlignment(Qt::AlignCenter);
    icon->setObjectName("backgroundIcon");

    QLabel* label = new QLabel{this};
    label->setText(text);
    label->setAlignment(Qt::AlignCenter);
    label->setWordWrap(true);
    label->setMaximumWidth(pixmap.width());

    QVBoxLayout* layout = new QVBoxLayout{this};
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(icon);
    layout->addWidget(label);
    setLayout(layout);
}

QPixmap UBPreferredBackgroundWidget::BackgroundWidget::pixmap() const
{
    return mPixmap;
}
