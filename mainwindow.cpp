#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "diagrammodels.h"
#include "propertymanager.h"

#include <stdio.h>
#include <QDebug>
#include <QtGlobal>
#include <QMap>
#include <QStringListModel>
#include <QStandardItem>
#include <QGridLayout>
#include <QStandardItemModel>
#include <QFile>
#include <QFileInfo>
#include <QStringList>
#include <QProcess>
#include <QInputDialog>


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
    connect(ui->selection_props_type,SIGNAL(currentIndexChanged(int)),manager,SLOT(onItemTypeChange(int)));
    connect(ui->actionEditLayout,SIGNAL(triggered(bool)),renderArea,SLOT(setEditing(bool)));
    connect(ui->actionUndo,SIGNAL(triggered(bool)),renderArea,SLOT(undo()));
    connect(ui->actionRedo,SIGNAL(triggered(bool)),renderArea,SLOT(redo()));
    connect(ui->actionSave,SIGNAL(triggered(bool)),this,SLOT(saveFile()));
    connect(ui->actionSave_As,SIGNAL(triggered(bool)),this,SLOT(saveAs()));
    connect(renderArea,SIGNAL(selectedFeatureChanged(Feature*)),this,SLOT(setSelectedItem(Feature*)));
    connect(ui->actionNew,SIGNAL(triggered(bool)),this,SLOT(newBuilding()));
    connect(renderArea,SIGNAL(openStairsDialog(Feature*,Floor*)),this,SLOT(openStairLinker(Feature*,Floor*)));
}

void MainWindow::newBuilding(){
    QStringList files = QFileDialog::getOpenFileNames(this,"Select floorplan image files",QString(),"PNG file (*.png)");
    QString jarPath= "JAR";
    QString pythonPath = "./"; //"/Users/Nick/Documents/School/McMaster/Year4/Capstone/OCR/";
    QString readerPath = "Reader/";
    QList<QFileInfo> dataFilesInfo;
    QList<QFileInfo> imageFilesInfo;
    for(QString imageFile: files){
        qputenv(qPrintable("PATH"),qgetenv("PATH") + ":/usr/local/bin");
        system(qPrintable("/Library/Frameworks/Python.framework/Versions/3.4/bin/python3 RoomNameReader.py -i " + imageFile + "> output.txt"));
        QString fname = QFileInfo(imageFile).fileName();
        fname.chop(4);
        const QString dataPath = pythonPath + "TesseractOutput/" + fname +"_data(processed).txt";
        const QString scannedImagePath = pythonPath + "TesseractOutput/" + fname + "_textless.png";
        QFileInfo datafileinfo = QFileInfo(dataPath);
        if(!datafileinfo.exists()) qWarning() << "File not found " << datafileinfo.filePath();
        dataFilesInfo << datafileinfo;
        imageFilesInfo << scannedImagePath;
    }

    for(int i = 0; i < dataFilesInfo.size();i++){
        system(qPrintable("java -classpath " + readerPath + " ImageReader \"" + imageFilesInfo[i].absoluteFilePath() + "\" \"" + dataFilesInfo[i].absoluteFilePath()+"\""));
    }



    Building* bldg= FileReader::loadBuidling("file.txt");
    building = new BuildingModel(bldg,this);
    ui->building_list_view->setModel(building);
    qDebug() << "Loaded building: " << bldg->name();
    filepath = "";
    setWindowTitle("--New Building--");


    /*args << jarPath;
    args << files;
    process.start("java",args);*/
}

void MainWindow::openStairLinker(Feature *feature, Floor *floor){
    QStringList floorNames;
    for(Floor* f: building->getModel()->floors()){
        if(f != floor) floorNames << f->name();
    }
    bool ok;
    QString floorName = QInputDialog::getItem(this,"Select floor to connect to","Floor:",floorNames,0,false,&ok);
    if(!ok || floorName.isEmpty())return;
    qDebug() << floorName;
    Floor* ofloor;
    for(Floor* f: building->getModel()->floors()){
        if(f->name() == floorName){
            ofloor = f;
            break;
        }
    }
    if(ofloor == NULL)return;
    QStringList roomNames;
    QMap<QString,int> roomNameIndex;
    int i = 0;
    for(Feature* f: ofloor->features()){
        if(f->type() == ROOM){
            roomNames << f->name();
            roomNameIndex[f->name()] = i;
        }
        i++;
    }
    QString roomName = QInputDialog::getItem(this,"Select the room to connect to","Room:",roomNames,0,false,&ok);
    if(!ok || roomName.isEmpty())return;
    feature->addConnection(ofloor->floorIndex(),roomNameIndex[roomName]);
    ofloor->features()[roomNameIndex[roomName]]->addConnection(floor->floorIndex(),floor->features().indexOf(feature));
}

void MainWindow::openFile(){
    QString path = QFileDialog::getOpenFileName(this,"Open building file","","Building files (*.bldg)");
    if(!path.isNull() && !path.isEmpty()){
        Building* bldg= FileReader::loadBuidling(path);
        building = new BuildingModel(bldg,this);
        ui->building_list_view->setModel(building);        
        qDebug() << "Loaded building: " << bldg->name();
        filepath = path;
        QFileInfo f(filepath);
        setWindowTitle(f.fileName());
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
        ui->connections_label->setText("");
        QString connectionText = "";
        for(FeatureConnection con: feature->connections()){
            QString conFloorName = building->at(con.floor_index)->name();
            QString conFeatName = building->at(con.floor_index,con.feature_index)->name();
            connectionText += conFloorName + " => " + conFeatName + "\n\r";
        }
        ui->connections_label->setText(connectionText);
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
