#pragma once


#include <QLayout>
#include <QStyle>

class UBFlowLayout : public QLayout
{
public:
    explicit UBFlowLayout(QWidget* parent, int margin = -1, int hSpacing = -1, int vSpacing = -1);
    explicit UBFlowLayout(int margin = -1, int hSpacing = -1, int vSpacing = -1);
    ~UBFlowLayout();

    void addItem(QLayoutItem* item) override;
    void insertItem(int index, QLayoutItem* item);
    int horizontalSpacing() const;
    int verticalSpacing() const;
    Qt::Orientations expandingDirections() const override;
    bool hasHeightForWidth() const override;
    int heightForWidth(int) const override;
    int count() const override;
    QLayoutItem* itemAt(int index) const override;
    QSize minimumSize() const override;
    void setGeometry(const QRect& rect) override;
    QSize sizeHint() const override;
    QLayoutItem* takeAt(int index) override;
    QLayoutItem* replaceAt(int index, QLayoutItem* item);

private:
    int doLayout(const QRect& rect, bool testOnly) const;
    int smartSpacing(QStyle::PixelMetric pm) const;

    QList<QLayoutItem*> itemList;
    int m_hSpace;
    int m_vSpace;
};
