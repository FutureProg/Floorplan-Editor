#include "diagrammodels.h"
using namespace DiagramModels;

#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QStringList>

Building::Building(QString name, QList<DiagramModels::Floor*> floors):_floors(floors),_name(name){}

QJsonObject Building::toJson(){
    QJsonObject obj;
    obj["version_id"] = 1;
    obj["name"] = this->name();
    QJsonArray fArray;
    for(Floor* f : floors()){
        QJsonObject floor;
        floor["name"] = f->name();  // copy name of floor
        QJsonArray featArray;
        for(Feature* feat : f->features()){
            QJsonObject featObj;
            QJsonArray boundXY;
            for(QPoint p : feat->bounds()){
                QJsonValue x(p.x());
                QJsonValue y(p.y());
                boundXY.append(x);
                boundXY.append(y);
            }
            featObj["bounds"] = boundXY;
            featObj["type"] = QJsonValue::fromVariant(feat->typeToString());
            featObj["name"] = feat->name();

            QJsonArray conArray;
            for(FeatureConnection con: feat->connections()){
                QJsonObject conobj;
                conobj["floor"] = con.floor_index;
                conobj["feature"] = con.feature_index;
                conArray.append(conobj);
            }
            featObj["connections"] = conArray;

            featArray.append(featObj);
        }
        floor["features"] = featArray;
        fArray.append(floor);
    }
    obj["floors"] = fArray;
    return obj;
}

Building::Building(QJsonDocument document){
    QJsonObject object = document.object();
    double version_id = object["version_id"].toDouble();
    qDebug() << "Version ID: " << version_id;
    QString buildingName = object["name"].toString();
    QJsonArray floors = object["floors"].toArray();
    _name = buildingName;
    for(int i = 0; i < floors.size();i++){
        QJsonObject floor = floors[i].toObject();
        QString name = floor["name"].toString();
        QJsonArray features = floor["features"].toArray();       
        Floor* bFloor = new Floor(i,name);
        qDebug() << "Add floor: " << name << "::" << bFloor;
        for(int j = 0; j < features.size();j++){
            QJsonObject f = features[j].toObject();
            QJsonArray boundXY = f["bounds"].toArray();
            qDebug() << "Found feature";
            QPolygon bounds;
            for(int _j = 0; _j < boundXY.size()-1;_j+=2){
                int x = (int)boundXY[_j].toDouble();
                int y = (int)boundXY[_j+1].toDouble();
                bounds << QPoint(x,y);                
            }            
            qDebug() << "Bounds: " << bounds;
            QString type = f["type"].toString();
            FeatureType fType;
            if(type == "STAIRS")    fType = FeatureType::STAIRS;
            else if(type == "ROOM")      fType = FeatureType::ROOM;
            int link = -1;
            if( f.contains("link")){
                link = f["link"].toString().toInt();
            }
            QString featName = type;
            qDebug() << type;
            if(f.contains("name")){
                featName = f["name"].toString();
            }

            QSet<FeatureConnection> featureConnections;
            if(f.keys().contains("connections")){
                QJsonArray connections = f["connections"].toArray();
                for(int k = 0; k < connections.size();k++){
                    QJsonObject con = connections[k].toObject();
                    FeatureConnection connection;
                    connection.feature_index = (int)con["feature"].toDouble();
                    connection.floor_index = (int)con["floor"].toDouble();
                    featureConnections << connection;
                }
            }
            Feature* feature = new Feature(fType,bounds,bFloor);
            feature->connections(featureConnections);
            qDebug() << "Connections found";
            feature->name(featName);
            bFloor->addFeature(feature);
            qDebug() << "Feature" << featName << ": "<<feature;
        }
        _floors << bFloor;
    }

}

Building::~Building(){    
    qDeleteAll(_floors);
}
