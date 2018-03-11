#include "renderarea.h"

#include <QPainter>
#include <QDebug>
#include <QPalette>

RenderArea::RenderArea(QWidget *parent) : QWidget(parent)
{
    QPalette pal;
    pal.setColor(QPalette::Background,Qt::white);
    setAutoFillBackground(true);
    setPalette(pal);

    pen.setColor(Qt::black);
    pen.setWidth(1);

    _floor = NULL;
    show();
}

void RenderArea::mouseMoveEvent(QMouseEvent* evt){

}

void RenderArea::paintEvent(QPaintEvent*){
    if(_floor == NULL){
        return;
    }
    QPainter painter(this);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    for(Feature* feature: _floor->features()){
        painter.drawPolygon(feature->bounds());
    }
}
