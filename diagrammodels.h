/*!
  Contains the models required for editing and displaying the floor plan models
  Present:
  Floor, Building, Feature, BuildingModel
  */

#ifndef DIAGRAM_MODELS_H
#define DIAGRAM_MODELS_H

#include <QList>
#include <QString>
#include <QPolygon>
#include <QDebug>
#include <QJsonDocument>
#include <QAbstractItemModel>
#include <QMetaEnum>
#include <QSet>
#include <QHash>

namespace DiagramModels{
    class Building;
    class Floor;
    class Feature;

    //! Shows what type of feature it is (either room or stairs)
    typedef enum{
        ROOM = 0,
        STAIRS = 1,
    }FeatureType;

    //! Shows what type of model it is (either a feature or floor) for polymorphic purposes
    typedef enum{
        FEATURE,
        FLOOR
    }DModelType;

    //! The base class for all diagram models
    class DModels{
    public:
        virtual DModelType modelType() = 0;
    };

    //! Contains the information needed to show a connection between two different features
    typedef struct FC{
        int floor_index;
        int feature_index;
        bool operator ==(struct FC other) const{
            return other.floor_index == floor_index && other.feature_index == feature_index;
        }
    }FeatureConnection;
    inline uint qHash(const FeatureConnection fc, uint seed = 0){
        return ::qHash(fc.floor_index,seed) + ::qHash(fc.feature_index,seed);
    }

    /*!
     * \brief The Feature class
     * Responsible for holding the information concerning room dimensions and other properties
     */
    class Feature:public DModels{
    public:
        /*!
         * \brief Feature constructor for the Feature model
         * \param type The feature type
         * \param bounds The vectorized bounds of the feature
         * \param floor A reference to the floor the feature is on
         */
        explicit Feature(FeatureType type, QPolygon bounds, Floor *floor):_bounds(bounds),_type(type),_floor(floor),_center(boundsCentroid(_bounds)){}

        //! Return the type of the feature
        FeatureType type(){return _type;}

        //! Set the feature type
        void type(FeatureType type){_type = type;}
        //! Convert the type from an enum to a string and return it
        QString typeToString() const{
            switch(_type){
            case STAIRS: return QString("Stairs");break;
            case ROOM: return QString("Room");break;
            }
        }
        //! Return the name of the feature (e.g. "Room 201")
        void name(QString name){_name = name;}
        //! add a connection between floors or features
        void addConnection(int floor_index, int feature_index){
            FeatureConnection con = {floor_index,feature_index};
            _connections << con;
        }
        //! Get a set of the values for the connections
        QSet<FeatureConnection> connections(){
            return _connections;
        }
        //! set the values for connections
        void connections(QSet<FeatureConnection> connections){
            _connections = connections;
        }
        //! Get the bounds of the feature
        QPolygon bounds(){return _bounds;}
        //! Set the bounds of the feature, recalculates the center
        void bounds(QPolygon bounds){
            _bounds = bounds;
            _center = boundsCentroid(_bounds);
            qDebug() << "Center of " << name() << ": " << _center;
        }
        //! Returns a FEATURE model type
        DModelType modelType() override{return FEATURE;}
        //! Get a reference to the floor the feature is on
        Floor* floor(){return _floor;}
        //! Get the feature name
        QString name(){return _name;}
        //! Get the center of the feature in XY
        QPoint center(){return _center;}


    private:
        /*!
         * \brief boundsCentroid find the center of the bounds of the feature
         * \param polygon the bounds to calculate
         * \return the center point of the feature
         */
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
            if(i == 0)return QPoint();
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
        FeatureType _type; //! The type of the feature
        QPolygon _bounds; //! The bounds of the feature
        Floor* _floor; //! What floor the feature is on
        QString _name; //! The name of the feature
        QPoint _center; //! The stored and recalculated center of the feature
        QSet<FeatureConnection> _connections; //! The connections the feature has
    };

    /*!
     * \brief The Floor class
     */
    class Floor: public DModels{
    public:
        /*!
         * \brief Floor constructor
         * \param index the index of the floor in the building
         * \param name the name of the floor (e.g. "Floor 2")
         */
        explicit Floor(int index, QString name):_floorIndex(index),_name(name){
        }
        ~Floor(){
            qDeleteAll(_features);
        }

        //! Get the name
        QString name(){return _name;}
        //! Set the name
        void name(QString name){_name = name;}
        //! Get the floor index
        int floorIndex(){return _floorIndex;}
        //! Set the floor index
        void floorIndex(int nIndex){_floorIndex = nIndex;}        

        //! Return FLOOR as the model type
        DModelType modelType() override{return FLOOR;}

        //! Get a list of the features the floor has
        QList<Feature*> features(){
            return _features;
        }
        //! Add a feature to the floor
        void addFeature(Feature* feature){_features << feature;}
        //! Remove a feature from the \param index
        void removeFeature(int index){_features.removeAt(index);}
        //! Remove feature \param f
        void removeFeature(Feature* f){_features.removeOne(f);}

     private:
        int _floorIndex; //! 0-indexed floor levels
        QList<Feature*> _features; //! List of features on the floor
        QString _name; //! The name of the floor
    };    

    class Building{
    public:
        /*!
         * \brief Building constructor
         * \param name the name of the building
         * \param floors the floors of the building
         */
        explicit Building(QString name, QList<DiagramModels::Floor*> floors);
        /*!
         * \brief Building constructor
         * \param document the json representation of the building
         */
        explicit Building(QJsonDocument document);
        ~Building();

        //! get the building name
        QString name(){return _name;}        
        //! Get the list of Floors
        QList<DiagramModels::Floor*> floors(){return _floors;}
        //! Get the number of floors
        int floorCount(){return _floors.length();}

        /*!
         * \brief toJson creates a JSON representation of the building
         * \return a QJSONObject representing the building
         */
        QJsonObject toJson();

    private:

        QList<DiagramModels::Floor*> _floors;
        QString _name;
    };

    class BuildingModel: public QAbstractItemModel{
        Q_OBJECT

    public:
        explicit BuildingModel(Building* data, QObject* parent = 0);
        ~BuildingModel();
        /*!
         * \brief data
         * \param index
         * \param role
         * \return the data stored under the given role for the item referred to by the index.
         */
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        /*!
         * \brief flags
         * \param index
         * \return the item flags for the given index.
         */
        Qt::ItemFlags flags(const QModelIndex &index) const override;
        /*!
         * \brief headerData
         * \param section
         * \param orientation
         * \param role
         * \return the data for the given role and section in the header with the specified orientation.
         */
        QVariant headerData(int section, Qt::Orientation orientation,int role = Qt::DisplayRole) const override;
        QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
        QModelIndex parent(const QModelIndex &index) const override;
        int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        int columnCount(const QModelIndex &parent = QModelIndex()) const override;

        /*!
         * \brief at
         * \param index
         * \return the floor at \param index
         */
        Floor* at(int index){
            if(index >= building->floorCount()) return NULL;
            return building->floors()[index];
        }
        /*!
         * \brief at
         * \param floorIndex
         * \param featureIndex
         * \return  the Feature on floor \param floorIndex and feature indexed \param featureIndex
         */
        Feature* at (int floorIndex, int featureIndex){
            if(floorIndex >= building->floorCount()) return NULL;
            if(featureIndex >= building->floors()[floorIndex]->features().length()) return NULL;
            return building->floors()[floorIndex]->features()[featureIndex];
        }        
        //! Returns the number of floors in the building
        int floorCount(){
            return building->floorCount();
        }

        //! Returns a reference to the building
        Building* getModel(){
            return building;
        }

    signals:
        //! The selected feature has been changed in the list
        void selectedFeatureChanged(Feature* feature);

    private:
        Building* building;
    };




}

#endif // DIAGRAM_MODELS_H
