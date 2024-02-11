#pragma once

#include <QWidget>

// forward
class UBFlowLayout;
class QLabel;
class QLayoutItem;

class UBPreferredBackgroundWidget : public QWidget
{
    Q_OBJECT

public:
    explicit UBPreferredBackgroundWidget(QWidget* parent = nullptr);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    void updateStyle() const;
    void savePreferrdBackgrounds() const;

private:
    UBFlowLayout* mLayout;
    QLabel* mPlaceholder{nullptr};
    QLayoutItem* mDraggedItem{nullptr};
    int mDraggedItemPos{-1};

    class BackgroundWidget : public QWidget
    {
    public:
        explicit BackgroundWidget(const QPixmap& pixmap, const QString& text, QWidget* parent = nullptr);

        QPixmap pixmap() const;

    private:
        QPixmap mPixmap;
    };
};
