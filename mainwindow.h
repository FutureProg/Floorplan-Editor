#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QTreeWidgetItem>

#include "diagrammodels.h"
#include "filereader.h"
#include "renderarea.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:    
    //! create a new building using floorplan images
    void newBuilding();
    //! open the stair linker dialog window
    void openStairLinker(Feature*,Floor*);
    //! open a .bldg file
    void openFile();
    //! save to a .bldg file
    void saveFile(){
        Building *bldg = building->getModel();
        filepath = FileWriter::instance()->saveFile(this,bldg,filepath);
        QFileInfo f(filepath);
        setWindowTitle(f.fileName());
    }
    //! save to a new .bldg file
    void saveAs(){
        Building *bldg = building->getModel();
        filepath = FileWriter::instance()->saveFile(this,bldg,"",true);
        QFileInfo f(filepath);
        setWindowTitle(f.fileName());
    }
    //! export the OSM file
    void exportToOSM(){
        FileWriter::instance()->exportFile(this,building->getModel(),OSM);
    }
    //! list item selection changed
    void listItemSelected(const QModelIndex& index);
    //! change the selected item
    void setSelectedItem(Feature* feature);

private:
    Ui::MainWindow *ui;
    DiagramModels::BuildingModel *building;
    RenderArea* renderArea;

    QMap<QString,FeatureType>* typeOptions;
    QString filepath;
};

#endif // MAINWINDOW_H
