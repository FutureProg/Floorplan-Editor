#ifndef FILEREADER_H
#define FILEREADER_H

#include <QString>
#include <QFile>
#include <QByteArray>

#include "diagrammodels.h"

using namespace DiagramModels;
class FileReader
{
public:
    FileReader();

    static Building* loadBuidling(QString filename){
        QFile file(filename);
        if(!file.open(QIODevice::ReadOnly)){
            qWarning("Failed to open file.");
            return 0;
        }
        QByteArray data = file.readAll();
        Building* building = new Building(QJsonDocument::fromJson(data));

        return building;
    };
};

#endif // FILEREADER_H
