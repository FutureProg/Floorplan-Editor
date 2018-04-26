#ifndef PROPERTYMANAGER_H
#define PROPERTYMANAGER_H

#include "mainwindow.h"

#include<QObject>

class PropertyManager;

class PropertyManager: public QObject{
    Q_OBJECT
public:
    enum ItemType{
        FLOOR,
        FEATURE
    };

public:
    /*!
     * \brief instance
     * \param parent
     * \return the instance of the property manager
     */
    static PropertyManager* instance(MainWindow* parent = 0){
        static PropertyManager* instance = new PropertyManager(parent);
        return instance;
    }

public slots:
    /**
     * @brief onItemSelected item an item is selected in the list or render area
     * @param item
     * @param itemType
     * @param index
     */
    void onItemSelected(void* item, ItemType itemType, const QModelIndex& index){
        _item = item;
        _itemType = itemType;
        _index = index;
    }

    /*!
     * \brief onItemNameChange change ht item name
     * \param text
     */
    void onItemNameChange(QString text){
        switch(_itemType){
        case FLOOR:
            ((Floor*)_item)->name(text);
            break;
        case FEATURE:
            ((Feature*)_item)->name(text);
            break;
        }
        emit ((BuildingModel*)_index.model())->dataChanged(_index,_index);
    }

    /*!
     * \brief onItemTypeChange change the item type
     * \param typeIndex
     */
    void onItemTypeChange(int typeIndex){
        Feature* feature = (Feature*)_item;
        feature->type(FeatureType(typeIndex));        
    }

private:
    PropertyManager(MainWindow* parent):QObject(parent){
    }

private:    
    void* _item;
    ItemType _itemType;
    QModelIndex _index;


};

#endif // PROPERTYMANAGER_H
