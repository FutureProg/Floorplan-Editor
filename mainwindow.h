#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>

#include "diagrammodels.h"
#include "filereader.h"

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

private:
    Ui::MainWindow *ui;
    DiagramModels::BuildingModel *building;
};

#endif // MAINWINDOW_H
