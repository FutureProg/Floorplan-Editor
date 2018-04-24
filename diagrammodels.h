#ifndef DIAGRAM_MODELS_H
#define DIAGRAM_MODELS_H

#include <QList>
#include <QString>
#include <QPolygon>
#include <QDebug>
#include <QJsonDocument>
#include <QAbstractItemModel>
#include <QMetaEnum>

namespace DiagramModels{
    class Building;
    class Floor;
    class Feature;

    typedef enum{
        ROOM = 0,
        STAIRS = 1,
    }FeatureType;

    typedef enum{
        FEATURE,
        FLOOR
    }DModelType;

    class DModels{
    public:
        virtual DModelType modelType() = 0;
    };

    class Feature:public DModels{
    public:
        explicit Feature(FeatureType type, QPolygon bounds, Floor *floor):_bounds(bounds),_type(type),_floor(floor),_center(boundsCentroid(_bounds)){}

        FeatureType type(){return _type;}
        void type(FeatureType type){_type = type;}
        QString typeToString() const{
            switch(_type){
            case STAIRS: return QString("Stairs");break;
            case ROOM: return QString("Room");break;
            }
        }
        void name(QString name){_name = name;}

        QPolygon bounds(){return _bounds;}        
        void bounds(QPolygon bounds){
            _bounds = bounds;
            _center = boundsCentroid(_bounds);
            qDebug() << "Center of " << name() << ": " << _center;
        }
        DModelType modelType() override{return FEATURE;}
        Floor* floor(){return _floor;}
        QString name(){return _name;}
        QPoint center(){return _center;}

    private:
        QPoint boundsCentroid(QPolygon polygon){
            QPoint re;
            QPoint p1,p2;
            int partial;
            int signedArea;
            int i;
            for(i = 0; i < polygon.size()-1;i++){
                p1 = polygon[i];
                p2 = polygon[i+1];
                partial = p1.x()*p2.y() - p1.y()*p2.x();
                signedArea += partial;
                re.setX(re.x() + (p1.x() + p2.x())*partial);
                re.setY(re.y() + (p1.y() + p2.y())*partial);
            }
            p1 = polygon[i];
            p2 = polygon[0];
            partial = p1.x()*p2.y() - p1.y()*p2.x();
            signedArea += partial;
            re.setX(re.x() + (p1.x() + p2.x())*partial);
            re.setY(re.y() + (p1.y() + p2.y())*partial);

            signedArea /= 2;
            re.setX(6 * signedArea);
            re.setY(6 * signedArea);
            return re;
        }

     private:
        FeatureType _type;
        QPolygon _bounds;
        Floor* _floor;
        QString _name;
        QPoint _center;
    };

    class Floor: public DModels{
    public:
        explicit Floor(int index, QString name):_floorIndex(index),_name(name){
        }
        ~Floor(){
            qDeleteAll(_features);
        }

        QString name(){return _name;}
        void name(QString name){_name = name;}
        int floorIndex(){return _floorIndex;}
        void floorIndex(int nIndex){_floorIndex = nIndex;}        

        DModelType modelType() override{return FLOOR;}

        QList<Feature*> features(){
            return _features;
        }
        void addFeature(Feature* feature){_features << feature;}
        void removeFeature(int index){_features.removeAt(index);}
        void removeFeature(Feature* f){_features.removeOne(f);}

     private:
        int _floorIndex; // 0-indexed
        QList<Feature*> _features;
        QString _name;
    };    

    class Building{
    public:
        explicit Building(QString name, QList<DiagramModels::Floor*> floors);
        explicit Building(QJsonDocument document);
        ~Building();

        QString name(){return _name;}
        QList<DiagramModels::Floor*> floors(){return _floors;}
        int floorCount(){return _floors.length();}

    private:

        QList<DiagramModels::Floor*> _floors;
        QString _name;
    };

    class BuildingModel: public QAbstractItemModel{
        Q_OBJECT

    public:
        explicit BuildingModel(Building* data, QObject* parent = 0);
        ~BuildingModel();

        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        Qt::ItemFlags flags(const QModelIndex &index) const override;
        QVariant headerData(int section, Qt::Orientation orientation,int role = Qt::DisplayRole) const override;
        QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
        QModelIndex parent(const QModelIndex &index) const override;
        int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        int columnCount(const QModelIndex &parent = QModelIndex()) const override;

        Floor* at(int index){
            if(index >= building->floorCount()) return NULL;
            return building->floors()[index];
        }
        Feature* at (int floorIndex, int featureIndex){
            if(floorIndex >= building->floorCount()) return NULL;
            if(featureIndex >= building->floors()[floorIndex]->features().length()) return NULL;
            return building->floors()[floorIndex]->features()[featureIndex];
        }
        int floorCount(){
            return building->floorCount();
        }

    signals:
        void selectedFeatureChanged(Feature* feature);

    private:
        Building* building;
    };




}

#endif // DIAGRAM_MODELS_H
