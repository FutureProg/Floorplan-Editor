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

void MainWindow::listItemSelected(const QModelIndex& index){
    PropertyManager* manager = PropertyManager::instance(this);

    // Get the item reference to pass to the property manager for modification
    void* item;
    int r = index.row();
    PropertyManager::ItemType itemType = PropertyManager::ItemType::FLOOR;
    if(index.parent().isValid()){
        int floorRow = index.parent().row();
        item = this->building->at(floorRow,r);
        itemType = PropertyManager::ItemType::FEATURE;
    }else{
        item = this->building->at(r);
    }
    if(item == NULL) return;
    manager->onItemSelected(item,itemType,index);

    // update the UI
    if(itemType == PropertyManager::ItemType::FEATURE){
        ui->selection_props_type->setEnabled(true);
        Feature* feature = this->building->at(index.parent().row(),r);
        ui->selection_props_type->setCurrentIndex(feature->type());
    }else{
        ui->selection_props_type->setDisabled(true);
    }
    ui->selection_props_name->setText(index.data().toString());
}

MainWindow::~MainWindow()
{
    delete ui;
    delete renderArea;
    delete building;
}
