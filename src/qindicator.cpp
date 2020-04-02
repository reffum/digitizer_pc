#include <QPainter>
#include "qindicator.h"

QIndicator::QIndicator(QWidget *parent) : QWidget(parent)
{
    color = Qt::red;
    setMaximumSize({25, 25});
    setMinimumSize({25, 25});
}

void QIndicator::setColor(Qt::GlobalColor color )
{
    this->color = color;
    update();
}

void QIndicator::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.scale(0.9,0.9);

    QRadialGradient grad(width()/2, height()/2, width()/2);
    grad.setColorAt(0, Qt::white);
    grad.setColorAt(1, color);

    painter.setBrush(grad);
    painter.drawEllipse(rect());
}
