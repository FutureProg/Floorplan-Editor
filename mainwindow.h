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
    void openFile();
    void saveFile(){

    };
    void saveAs(){

    };
    void listItemSelected(const QModelIndex& index);

private:
    Ui::MainWindow *ui;
    DiagramModels::BuildingModel *building;
    RenderArea* renderArea;

    QMap<QString,FeatureType>* typeOptions;
};

#endif // MAINWINDOW_H
