//
// qindicator.h
// Круглый индикатор.
//

#ifndef QINDICATOR_H
#define QINDICATOR_H

#include <QWidget>

class QIndicator : public QWidget
{
    Q_OBJECT
public:
    QIndicator(QWidget *parent = NULL);

    // Устанавливает цвет индикатора
    void setColor(Qt::GlobalColor color );

private:
    void paintEvent(QPaintEvent*);

    // Цвет индикатора
    Qt::GlobalColor color;
};

#endif // QINDICATOR_H
