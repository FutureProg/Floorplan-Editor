#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "diagrammodels.h"
#include "propertymanager.h"

#include <stdio.h>
#include <QDebug>
#include <QMap>
#include <QStringListModel>
#include <QStandardItem>
#include <QGridLayout>
#include <QStandardItemModel>
#include <QStringList>


using namespace DiagramModels;
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),ui(new Ui::MainWindow){
    ui->setupUi(this);

    QGridLayout* renderLayout = new QGridLayout();
    renderArea = new RenderArea(this);
    renderLayout->addWidget(renderArea,0,0);
    ui->render_container->setLayout(renderLayout);

    // Setup properties area
    PropertyManager* manager = PropertyManager::instance(this);
    typeOptions = new QMap<QString,FeatureType>();
    typeOptions->insert("Stairs",FeatureType::STAIRS);
    typeOptions->insert("Room",FeatureType::ROOM);
    ui->selection_props_type->addItems(typeOptions->keys());
    ui->selection_props_type->setDisabled(true);

    // Connect UI events
    connect(ui->actionOpen,SIGNAL(triggered(bool)),this,SLOT(openFile()));    
    connect(ui->building_list_view,SIGNAL(clicked(QModelIndex)),this,SLOT(listItemSelected(QModelIndex)));    
    connect(ui->selection_props_name,SIGNAL(textChanged(QString)),manager,SLOT(onItemNameChange(QString)));    
    connect(ui->actionEditLayout,SIGNAL(triggered(bool)),renderArea,SLOT(setEditing(bool)));
    connect(ui->actionUndo,SIGNAL(triggered(bool)),renderArea,SLOT(undo()));
    connect(ui->actionRedo,SIGNAL(triggered(bool)),renderArea,SLOT(redo()));
    connect(renderArea,SIGNAL(selectedFeatureChanged(Feature*)),this,SLOT(setSelectedItem(Feature*)));
}

void MainWindow::openFile(){
    QString path = QFileDialog::getOpenFileName(this,"Open building file","","Building files (*.bldg)");
    if(!path.isNull() && !path.isEmpty()){
        Building* bldg= FileReader::loadBuidling(path);
        building = new BuildingModel(bldg,this);
        ui->building_list_view->setModel(building);        
        qDebug() << "Loaded building: " << bldg->name();
    }
}

void MainWindow::setSelectedItem(Feature* feature){
    Floor* floor = renderArea->floor();
    int floorRow = floor->floorIndex();
    // get the row of the floor
    /*for(; floorRow < building->floorCount();floorRow++){
        if(building->at(floorRow) == floor){
            break;
        }
    }
    if(floorRow >= building->floorCount())return;*/

    // get the row of the feature
    int featureRow = 0;
    for(; featureRow < floor->features().count();featureRow++){
        if(floor->features()[featureRow] == feature)break;
    }
    if(featureRow >= floor->features().count()) return;

    QModelIndex index = building->index(floorRow,0);
    index = building->index(featureRow,0,index);

    ui->building_list_view->setCurrentIndex(index);
    listItemSelected(index);
}

void MainWindow::listItemSelected(const QModelIndex& index){
    PropertyManager* manager = PropertyManager::instance(this);

    // Get the item reference to pass to the property manager for modification
    void* item;
    int r = index.row();
    PropertyManager::ItemType itemType = PropertyManager::ItemType::FLOOR;
    Floor* floor;
    if(index.parent().isValid()){
        int floorRow = index.parent().row();
        item = this->building->at(floorRow,r);
        itemType = PropertyManager::ItemType::FEATURE;
        floor = this->building->at(floorRow);
    }else{
        item = this->building->at(r);
        floor = this->building->at(r);
    }
    if(item == NULL) return;
    manager->onItemSelected(item,itemType,index);

    // update the UI
    if(itemType == PropertyManager::ItemType::FEATURE){
        ui->selection_props_type->setEnabled(true);
        Feature* feature = this->building->at(index.parent().row(),r);
        ui->selection_props_type->setCurrentIndex(feature->type());
        renderArea->setSelectedFeature(feature);
    }else{
        ui->selection_props_type->setDisabled(true);
    }
    ui->selection_props_name->setText(index.data().toString());

    // Update the render area
    renderArea->floor(floor);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete renderArea;
    delete building;
}
