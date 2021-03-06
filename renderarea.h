#ifndef RENDERAREA_H
#define RENDERAREA_H

#include <cmath>

#include <QWidget>
#include <QPen>
#include <QStack>
#include <QQueue>

#include "diagrammodels.h"

using namespace DiagramModels;

//! THe Editor Action Types for Undo functionality
typedef enum{
    ADD_POINT,
    DELETE_FEATURE,
    ADD_FEATURE,
    SELECT_FEATURE, // data = array[2] <= [0] = previous, [1] = current
    MOVE_FEATURE // data = Feature*,QPoint(dx,dy)*
}EditorActionType;
//! The move data type for editor action types for Undo Functionality
typedef struct{
    Feature* feature;
    QPoint* delta;
}moveData;
//! Editor Actions
typedef struct{
    EditorActionType type;
    void* data;
}EditorAction;

//! The state of the render area, either SELECT, DRAG, or EDIT
typedef enum{
    SELECT,
    DRAG,
    EDIT
}RenderAreaState;

/*!
 * \brief The RenderArea class Responsible for drawing and editing the floorplans
 */
class RenderArea : public QWidget
{
    Q_OBJECT
public:    
    explicit RenderArea(QWidget *parent = nullptr);

    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent *evt) override;
    void mousePressEvent(QMouseEvent *)override;    
    void mouseDoubleClickEvent(QMouseEvent *) override;

    //! set the floor
    void floor(Floor* floor){
        _floor=floor;
        if(!_redoQueue.keys().contains(_floor)){
            _redoQueue[_floor] = QQueue<EditorAction>();
            _undoStack[_floor] = QStack<EditorAction>();
        }
        update();
    }
    //! get the floor
    Floor* floor(){return _floor;}

    //! remove the selected feature from the floor
    void removeSelectedFeature();

signals:
    /*!
     * \brief featureListChanged update to the feature list needed
     * \param feature the new or deleted feature
     */
    void featureListChanged(Feature* feature);
    /*!
     * \brief selectedFeatureChanged the selected feature has been changed
     * \param feature the newly selected feature
     */
    void selectedFeatureChanged(Feature* feature);
    /*!
     * \brief openStairsDialog open the stair linking dialog
     * \param feature
     * \param floor
     */
    void openStairsDialog(Feature* feature, Floor* floor);

public slots:
    /*!
     * \brief setSelectedFeature change the selected feature
     * \param feature
     */
    void setSelectedFeature(Feature* feature){
        selectedFeature = feature;
    }  

    /*!
     * \brief setState change the state of the RenderArea
     * \param state
     */
    void setState(RenderAreaState state){
        _state = state;
        repaint();
    }

    /*!
     * \brief setEditing change the editing state via the UI
     * \param b
     */
    void setEditing(bool b){
        setState(b? EDIT : SELECT);
    }

    /*!
     * \brief pushUndo add an Action to the change stack
     * \param action
     */
    void pushUndo(EditorAction action){
        _redoQueue[_floor].clear();
        _undoStack[_floor].push(action);
    }

    /*!
     * \brief undo undo the last action in the change stack
     */
    void undo(){
        if(_undoStack[_floor].empty()) return;
        EditorAction action = _undoStack[_floor].pop();
        qDebug() << "Undo: " << action.type << " : " << action.data;
        _redoQueue[_floor].append(action);
        if(action.type == ADD_POINT){
            QPolygon bounds = selectedFeature->bounds();
            bounds.removeLast();
            selectedFeature->bounds(bounds);
        }
        else if(action.type == DELETE_FEATURE){
            Feature* f = static_cast<Feature*>(action.data);
            if(f){
                _floor->addFeature(f);
            }else{
                qDebug() << "Tried to undo deletion of feature, bad cast";
            }
        }
        else if(action.type == ADD_FEATURE){
            Feature** f = static_cast<Feature**>(action.data);
            if(f){
                selectedFeature = f[0];
                _floor->removeFeature(f[1]);
            }else{
                qDebug() << "Tried to undo addition of feature, bad cast";
            }
        }
        else if(action.type == SELECT_FEATURE){
            Feature** f = static_cast<Feature**>(action.data);
            selectedFeature = f[0];
        }
        else if(action.type == MOVE_FEATURE){
            moveData* data = static_cast<moveData*>(action.data);
            Feature* f = data->feature;
            QPoint* delta = data->delta;
            QPolygon nbounds;
            for(QPoint p : f->bounds()){
                nbounds << p-*delta;
            }
            f->bounds(nbounds);
        }
        repaint();
    }

    /*!
     * \brief redo redo the first change on the redo queue
     */
    void redo(){
        if(_redoQueue[_floor].empty())return;
        EditorAction action = _redoQueue[_floor].dequeue();
        _undoStack[_floor].push(action);
        if(action.type == ADD_POINT){
            QPolygon bounds = selectedFeature->bounds();
            QPoint* point = static_cast<QPoint*>(action.data);
            bounds << *point;
            selectedFeature->bounds(bounds);
        }
        else if(action.type == DELETE_FEATURE){
            Feature* f = static_cast<Feature*>(action.data);
            if(f){
                _floor->removeFeature(f);
            }else{
                qDebug() << "Tried to undo deletion of feature, bad cast";
            }
        }
        else if(action.type == ADD_FEATURE){
            Feature* f = static_cast<Feature*>(action.data);
            if(f){
                _floor->addFeature(f);
            }else{
                qDebug() << "Tried to undo addition of feature, bad cast";
            }
        }
        else if(action.type == SELECT_FEATURE){
            Feature** f = static_cast<Feature**>(action.data);
            selectedFeature = f[1];
        }
        else if(action.type == MOVE_FEATURE){
            moveData* data = static_cast<moveData*>(action.data);
            Feature* f = data->feature;
            QPoint* delta = data->delta;
            QPolygon nbounds;
            for(QPoint p : f->bounds()){
                nbounds << p+*delta;
            }
            f->bounds(nbounds);
        }
        repaint();
    }

protected:
    void paintEvent(QPaintEvent*) override;    
    void keyPressEvent(QKeyEvent*) override;
    void keyReleaseEvent(QKeyEvent *evt) override;

private:    

    /*!
     * \brief snapToRoom snaps point to the corner of the closest room
     * \param point
     * \param alpha
     * \return the corner QPoint of the nearest room to point
     */
    QPoint snapToRoom(QPoint point,int alpha = 10){
        for(Feature* f: _floor->features()){
            for(QPoint p : f->bounds()){
                if((point - p).manhattanLength() <= alpha){
                    return p;
                }
            }
        }
        return point;
    }

    /*!
     * \brief snapToDegree snaps everything to a 90 degree angle
     * \param point
     * \param snapAngle
     * \return the adjusted qpoint
     */
    QPoint snapToDegree(QPoint point, float snapAngle = 45){
        if(selectedFeature->bounds().empty())return point;
        QPoint prev = selectedFeature->bounds().last();
        float angle = atan2(prev.y() - point.y(), prev.x() - point.x());
        angle = angle * 180/M_PI;
        angle = snapAngle * (float) round(angle/snapAngle);
        if(lround(angle) % 90 == 0){
            if(abs(angle) == 180 || angle == 0) point.setY(prev.y());
            else point.setX(prev.x());
        }
        return point;
    }

private:
    QPen pen;
    Floor* _floor;
    Feature* selectedFeature;   
    RenderAreaState _state;

    bool _shouldSnapToRoom; // defaults to true
    bool _shouldSnapToDegree; // defaults to false (0º,45º,90º,etc)
    QPoint* _dragDelta, *_dragOrigin,*_dragLastPoint;

    QMap<Floor*,QStack<EditorAction>> _undoStack;
    QMap<Floor*,QQueue<EditorAction>> _redoQueue;

};

#endif // RENDERAREA_H
