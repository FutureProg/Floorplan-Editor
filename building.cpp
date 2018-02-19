#include "diagrammodels.h"
using namespace DiagramModels;

#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

Building::Building(QString name, QList<DiagramModels::Floor*> floors):_floors(floors),_name(name){}

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
            QJsonObject f = features[i].toObject();
            QJsonArray boundXY = f["bounds"].toArray();
            QPolygon bounds(boundXY.size()/2);
            for(int _j = 0; _j < boundXY.size()-1;_j+=2){
                int x = boundXY[j].toString().toInt();
                int y = boundXY[j+1].toString().toInt();
                bounds << QPoint(x,y);
            }
            QString type = f["type"].toString();
            FeatureType fType;
            if(type == "STAIRS")    fType = FeatureType::STAIRS;
            if(type == "ROOM")      fType = FeatureType::ROOM;
            int link = -1;
            if( f.contains("link")){
                link = f["link"].toString().toInt();
            }
            Feature* feature = new Feature(fType,bounds,bFloor);
            qDebug() << "Feature" << j << ": "<<feature;
            bFloor->addFeature(feature);
        }
        _floors << bFloor;
    }

}

Building::~Building(){    
    qDeleteAll(_floors);
}
