#include "filewriter.h"

#include <QFileDialog>
#include <QFile>
#include <QIODevice>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
//#include <QDataStream>

FileWriter::FileWriter()
{
}

using namespace DiagramModels;
QString FileWriter::saveFile(QWidget* context, Building* building,QString filepath,bool saveAs){
    if(saveAs || filepath == ""){
        QString nfilepath = QFileDialog::getSaveFileName(context, "Save building file", building->name(),"building file (*.bldg)");
        if(nfilepath.isEmpty())
            return filepath;
        filepath = nfilepath;
    }
    QFile file(filepath);
    if(!file.open(QIODevice::WriteOnly)){
        QMessageBox::information(context,"Unable to save to file at " + filepath,file.errorString());
        return QString();
    }
    QJsonDocument doc(building->toJson());
    file.write(doc.toJson());
    file.close();
    return filepath;
}

void FileWriter::exportFile(QWidget* context, Building* bldg, ExportFormat frmt){
    QString filepath = QFileDialog::getSaveFileName(context,"Save OSM File",QString(),"OSM Format (*.xml)");
    if(filepath.isEmpty())return;
    QFile file(filepath);
    if(!file.open(QIODevice::WriteOnly)){
        QMessageBox::information(context,"Unable to export OSM file to " + filepath,file.errorString());
    }

}
