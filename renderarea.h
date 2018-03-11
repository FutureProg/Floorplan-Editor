#ifndef RENDERAREA_H
#define RENDERAREA_H

#include <QWidget>
#include <QPen>

#include "diagrammodels.h"

using namespace DiagramModels;
class RenderArea : public QWidget
{
    Q_OBJECT
public:
    explicit RenderArea(QWidget *parent = nullptr);

    void mouseMoveEvent(QMouseEvent*) override;

    void floor(Floor* floor){_floor=floor;update();}
    Floor* floor(){return _floor;}

protected:
    void paintEvent(QPaintEvent*) override;

private:
    QPen pen;
    Floor* _floor;
};

#endif // RENDERAREA_H
