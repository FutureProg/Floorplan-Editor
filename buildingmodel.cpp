#include "diagrammodels.h"

#include <QDebug>

using namespace DiagramModels;

BuildingModel::BuildingModel(Building *data, QObject *parent):QAbstractItemModel(parent){
    building = data;
}

QVariant BuildingModel::data(const QModelIndex &index, int role) const{
    if(role != Qt::DisplayRole)
        return QVariant();
    DModels* m = static_cast<DModels*>(index.internalPointer());
    if(m->modelType() == "FLOOR"){
        return building->floors()[index.row()]->name();
    }else{
        Feature* feature = static_cast<Feature*>(index.internalPointer());
        return feature->name();
    }
}

Qt::ItemFlags BuildingModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return QAbstractItemModel::flags(index);
}

QVariant BuildingModel::headerData(int section, Qt::Orientation orientation,int role) const{
    return QVariant();
}


QModelIndex BuildingModel::index(int row, int column, const QModelIndex &parent) const{
    Floor* floor;
    if(parent.isValid()){        
        floor = static_cast<Floor*>(parent.internalPointer());
        if(row >= floor->features().size()){
            return QModelIndex();
        }
        return createIndex(row,column,floor->features()[row]);
    }else{
        if(row >= building->floorCount()){
            return QModelIndex();
        }
        return createIndex(row,column,building->floors()[row]);
    }
}

QModelIndex BuildingModel::parent(const QModelIndex &index) const{
    if(!index.isValid()){
        return QModelIndex();
    }

    DModels* dm = static_cast<DModels*>(index.internalPointer());
    if(dm->modelType() == "FLOOR"){
        return QModelIndex();
    }else if(dm->modelType() == "FEATURE"){
        Feature *f = static_cast<Feature*>(index.internalPointer());
        return createIndex(f->floor()->floorIndex(),0,f->floor());
    }
}

int BuildingModel::rowCount(const QModelIndex &parent) const{    
    if(!parent.isValid()){
        return building->floorCount();
    }        
    Floor* f = static_cast<Floor*>(parent.internalPointer());
    return f->features().count();
}

int BuildingModel::columnCount(const QModelIndex &parent) const{
    return 1;
}


BuildingModel::~BuildingModel(){
    delete building;
}
