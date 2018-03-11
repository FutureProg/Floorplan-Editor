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

protected:
    void paintEvent(QPaintEvent*) override;

private:
    QPen pen;
    Floor* floor;
};

#endif // RENDERAREA_H
