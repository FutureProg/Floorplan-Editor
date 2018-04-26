#ifndef FILEWRITER_H
#define FILEWRITER_H

#include <QString>
#include "diagrammodels.h"


typedef enum{
    OSM          // Open Street Maps Export
}ExportFormat;

class FileWriter
{
public:
    /*!
     * \brief instance
     * \return the instance of the FileWriter to be used
     */
    static FileWriter* instance(){
        static FileWriter *writer = new FileWriter();
        return writer;
    }

    /*!
     * \brief saveFile saves the data to a .bldg file in JSON format
     * \param context the calling QWidget
     * \param filepath the path to the file
     * \param saveAs if this is a "save as" operation
     * \return the filepath used, or the original if canceled
     */
    QString saveFile(QWidget* context, DiagramModels::Building*, QString filepath = "", bool saveAs = false);
    /*!
     * \brief exportFile export the file to the desired format
     * \param context the calling QWidget
     * \param frmt the desired format, default being Open Street Maps
     */
    void exportFile(QWidget* context, DiagramModels::Building*, ExportFormat frmt = OSM);

protected:
    FileWriter();
};

#endif // FILEWRITER_H
