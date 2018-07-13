#ifndef UBPOPUP_H
#define UBPOPUP_H


#include <QWidget>
#include <QLabel>
#include <QGridLayout>
#include <QPropertyAnimation>
#include <QTimer>

class UBPopUp : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(float popupOpacity READ getPopupOpacity WRITE setPopupOpacity)

    void setPopupOpacity(float opacity);
    float getPopupOpacity() const;

public:
    explicit UBPopUp(QWidget *parent = 0, QString Action="", QString title="", QString pathIcon="");

protected:
    void paintEvent(QPaintEvent *event);

public slots:
    void setPopupText(const QString& text);
    void show();

private slots:
    void hideAnimation();
    void hide();

private:
    QLabel label; // text of message
    QLabel header;  // It contains an icon and a descriptive title.
    QLabel action; // label for the action performed.
    QGridLayout layout;
    QPropertyAnimation animation;
    float popupOpacity;
    QTimer *timer;
};

#endif // UBPOPUP_H
