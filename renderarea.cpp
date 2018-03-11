#include "renderarea.h"

#include <QPainter>
#include <QPalette>

RenderArea::RenderArea(QWidget *parent) : QWidget(parent)
{
    QPalette pal;
    pal.setColor(QPalette::Background,Qt::white);
    setAutoFillBackground(true);
    setPalette(pal);

    pen.setColor(Qt::black);
    pen.setWidth(4);
    floor = nullptr;

    show();
}

void RenderArea::mouseMoveEvent(QMouseEvent* evt){

}

void RenderArea::paintEvent(QPaintEvent*){
    if(floor == nullptr){
        return;
    }
    QPainter painter(this);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    for(Feature* feature: floor->features()){
        painter.drawPolygon(feature->bounds());
    }
}
