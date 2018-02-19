#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "diagrammodels.h"

#include <stdio.h>
#include <QDebug>
#include <QStringListModel>
#include <QStandardItem>
#include <QStandardItemModel>


using namespace DiagramModels;
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),ui(new Ui::MainWindow){
    ui->setupUi(this);

    connect(ui->actionOpen,SIGNAL(triggered(bool)),this,SLOT(openFile()));
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

MainWindow::~MainWindow()
{
    delete ui;
    delete building;
}
