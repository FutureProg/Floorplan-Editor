#include "renderarea.h"
#include "mainwindow.h"

#include <QPainter>
#include <QDebug>
#include <QKeyEvent>
#include <QPalette>
#include <QPointer>
#include <QInputDialog>

RenderArea::RenderArea(QWidget *parent) : QWidget(parent)
{
    QPalette pal;
    pal.setColor(QPalette::Background,Qt::white);
    setAutoFillBackground(true);
    setPalette(pal);
    setMouseTracking(true);

    pen.setColor(Qt::black);
    pen.setWidth(1);

    _floor = NULL;
    selectedFeature = NULL;
    _state = SELECT;
    _shouldSnapToRoom = true;
    _shouldSnapToDegree = false;
    show();
}

void RenderArea::mouseMoveEvent(QMouseEvent*){
    QPoint pos = QCursor::pos();
    pos = mapFromGlobal(pos);
    if(_state == DRAG){
        QPoint delta;
        delta.setX(pos.x() - _dragLastPoint->x());
        delta.setY(pos.y() - _dragLastPoint->y());
        QPolygon bounds = selectedFeature->bounds();
        QPolygon nbounds;
        for(QPoint p : bounds){
            nbounds << p + delta;
        }
        selectedFeature->bounds(nbounds);
        _dragLastPoint->setX(pos.x());
        _dragLastPoint->setY(pos.y());
    }
    repaint();
}

void RenderArea::keyReleaseEvent(QKeyEvent *evt){
    switch(evt->key()){
    case Qt::Key_C:
        _shouldSnapToRoom = true;
        break;
    case Qt::Key_Shift:
        if(_state == EDIT)_shouldSnapToDegree = false;
    case Qt::Key_S:
        if(evt->modifiers().testFlag(Qt::AltModifier)){
            qDebug() << "ALT PASS";
            if(selectedFeature && selectedFeature->type() == STAIRS){
                qDebug() << "OPEN STAIRS";
                openStairsDialog(selectedFeature,_floor);
            }else{
                qDebug() << "DONT OPEN STAIRS";
            }
        }
    }
}

void RenderArea::keyPressEvent(QKeyEvent *evt){
    switch(evt->key()){
    case Qt::Key_Delete:
    case Qt::Key_Backspace:
        this->removeSelectedFeature();    
    case Qt::Key_Escape:
        if(selectedFeature != NULL){
            EditorAction action;
            action.type = SELECT_FEATURE;
            Feature* f[2];
            f[0] = selectedFeature;
            f[1] = NULL;
            action.data = f;
            pushUndo(action);
            if(selectedFeature->bounds().size() < 3){
                removeSelectedFeature();
            }
            selectedFeature = NULL;
            selectedFeatureChanged(NULL);            
            repaint();
        }
        break;
    case Qt::Key_C:
        if(_state == EDIT)_shouldSnapToRoom = false;
        break;
    case Qt::Key_Shift:
        _shouldSnapToDegree = true;
    }
}

void RenderArea::mouseDoubleClickEvent(QMouseEvent *){
    QPoint mousePos = QCursor::pos();
    mousePos = mapFromGlobal(mousePos);
    switch(_state){
    case SELECT:
    case DRAG:{
        if(selectedFeature && selectedFeature->bounds().containsPoint(mousePos,Qt::OddEvenFill)){
            if(selectedFeature->type() == STAIRS){
                _state = SELECT;
                openStairsDialog(selectedFeature,_floor);                
            }
        }
        break;
    }        
    }
}

void RenderArea::mousePressEvent(QMouseEvent *){
    setFocus();
    QPoint mousePos = QCursor::pos();
    mousePos = mapFromGlobal(mousePos);
    if(selectedFeature &&
            selectedFeature->bounds().containsPoint(mousePos,Qt::OddEvenFill)&&
            _state == SELECT){
        _state = DRAG;
        _dragDelta = new QPoint(0,0);
        _dragOrigin = new QPoint(mousePos);
        _dragLastPoint = new QPoint(mousePos);
    }
}

void RenderArea::mouseReleaseEvent(QMouseEvent *){
    if(!_floor)return;
    QPoint mousePos = QCursor::pos();
    mousePos = mapFromGlobal(mousePos);
    if(_state == SELECT){
        bool nSelect = false;
        for(Feature* feature: _floor->features()){
            if(feature->bounds().containsPoint(mousePos,Qt::OddEvenFill)){
                EditorAction action;
                Feature* f[2];
                f[0] = selectedFeature;
                f[1] = feature;
                action.data = f;
                action.type = SELECT_FEATURE;
                pushUndo(action);
                selectedFeature = feature;
                selectedFeatureChanged(selectedFeature);
                nSelect = true;
            }            
        }        
        repaint();
        if(nSelect) return;
        if(selectedFeature && selectedFeature->bounds().size() < 3){
            removeSelectedFeature();
        }
        selectedFeature = NULL;        
        selectedFeatureChanged(selectedFeature);
        repaint();
    }
    else if(_state == DRAG){
        _dragDelta = new QPoint(mousePos - *_dragOrigin);
        EditorAction action;
        action.type = MOVE_FEATURE;
        moveData* data = new moveData();
        data->feature = selectedFeature;
        data->delta = _dragDelta;
        action.data = data;
        pushUndo(action);
        _state = SELECT;
    }
    else if(_state == EDIT){
        QPoint editPoint = mousePos;
        if(_shouldSnapToDegree){
            editPoint = snapToDegree(editPoint);
        }
        if(_shouldSnapToRoom){
            editPoint = snapToRoom(editPoint);
        }
        if(selectedFeature == NULL){
            QPolygon bounds;
            bounds << editPoint;
            Feature* f = new Feature(FeatureType::ROOM,bounds,_floor);
            f->name("New Room");
            Feature* a[2];
            a[1] = f;
            a[0] = selectedFeature;
            EditorAction action;
            action.data = a;
            action.type = ADD_FEATURE;
            pushUndo(action);
            _floor->addFeature(f);            
            selectedFeature = f;
            featureListChanged(f);
            selectedFeatureChanged(selectedFeature);
        }else{
            QPolygon bounds = selectedFeature->bounds();
            bounds << editPoint;
            selectedFeature->bounds(bounds);
            EditorAction action;
            action.data = &editPoint;
            action.type = ADD_POINT;
            pushUndo(action);
        }
    }
    repaint();
}

void RenderArea::removeSelectedFeature(){
    if(selectedFeature){
        _floor->removeFeature(selectedFeature);
        EditorAction action;
        action.data = selectedFeature;
        action.type = DELETE_FEATURE;
        pushUndo(action);
        selectedFeature = NULL;        
        featureListChanged(NULL);
        selectedFeatureChanged(NULL);
        repaint();
    }
}

void RenderArea::paintEvent(QPaintEvent*){    
    if(_floor == NULL){
        return;
    }
    QPoint mousePos = QCursor::pos();
    mousePos = mapFromGlobal(mousePos);
    QPainter painter(this);
    painter.eraseRect(0,0,width(),height());
    painter.setPen(pen);    
    for(Feature* feature: _floor->features()){
        if(feature == selectedFeature){
            if(_state == EDIT) painter.setBrush(Qt::NoBrush);
            else painter.setBrush(Qt::blue);
        }else if(_state != EDIT && feature->bounds().containsPoint(mousePos,Qt::OddEvenFill)){
            painter.setBrush(QBrush(QColor(0,0,255,100)));
        }else if(feature->connections().size() > 0){
            painter.setBrush(Qt::yellow);
        }else{
            painter.setBrush(Qt::lightGray);
        }
        painter.drawPolygon(feature->bounds());
        painter.drawText(feature->center(),feature->name());

    }
    if(_state == EDIT && selectedFeature != NULL){
        painter.setBrush(Qt::black);
        for(QPoint p : selectedFeature->bounds()){ // draw the points for the bounds
            painter.drawEllipse(p,5,5);
        }
        QLine previewLine;
        previewLine.setP1(selectedFeature->bounds().last());
        QPoint editPoint = QCursor::pos();
        editPoint = mapFromGlobal(editPoint);
        editPoint = editPoint;
        if(_shouldSnapToDegree){
            editPoint = snapToDegree(editPoint);
        }
        if(_shouldSnapToRoom){
            editPoint = snapToRoom(editPoint);
        }
        QPen previewPen(painter.pen());
        previewPen.setColor(Qt::gray);
        painter.setPen(previewPen);
        previewLine.setP2(editPoint);
        painter.drawLine(previewLine); // draw the preview
        //previewLine.setP1(selectedFeature->bounds().first());
        //painter.drawLine(previewLine); // draw the preview lines
        painter.setPen(pen);
    }
}
