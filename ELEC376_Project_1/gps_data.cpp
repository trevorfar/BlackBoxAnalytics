#include "gps_data.h"
#include <QString>
#include <iostream>
#include <string>
#include <QDateTime>


gps_data::gps_data() {}

void gps_data::loadCSVToArray(const QString filePath) {
    this->filepath = filePath;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open file:" << filePath;
        name = "error";
    }else{

        QTextStream in(&file);

        // Extract file name
        QString fileNameWithExt = filePath.section('/', -1); // Works for both / and \ separators
        name = fileNameWithExt.section('.', 0, 0);


        // Skips first 200 lines
        QString line = in.readLine();
        for (int i = 0; i < 200; i ++){
            line = in.readLine();              // Read each line
            // qDebug() << line;
        }


        while (!in.atEnd()) {
            line = in.readLine();              // Read each line
            QStringList row = line.split(',');         // Split by commas
            data.append(row.toVector());               // Convert row to QVector and add to data
        }
        file.close();
        std::string stdStr = data[0][0].toStdString();
        firstTime = std::stold(stdStr);
        qDebug() << data[0];
        qDebug() << stdStr;
    }
    // qDebug() << data;

}

void gps_data::calculateOffset(QVector<gps_data> &datalist){
    if (datalist.size() > 0){
        // find smallest start time
        long double smallest = datalist[0].firstTime;
        for(auto d : datalist){
            if (d.firstTime < smallest){
                smallest = d.firstTime;
            }
        }

        // Calculated offset
        indexOffSet = smallest - firstTime;
        indexOffSet = indexOffSet / 1000;
        index += indexOffSet;

        qDebug() << indexOffSet;
    }
}

void gps_data::createWindow(){
    window.clear();
    if (index >= 0 && index < data.size() - windowSize) {
        for (int i = index; i < index + windowSize; i++){
            window.append(data[i]);
        }
    }
}

void gps_data::incrementWindow(){
    index++;
    if (index >= 0 && index < data.size() - windowSize){
        window.removeFirst();
        window.append(data[index + windowSize]);
    }
}

QVector<double> gps_data::getXs(){
    QVector<double> data;
    for(int i = 0; i < window.size(); i++){
        double x  = window[i][1].toDouble();
        data.append(x);
        // qDebug() << data;
    }
    return data;
}

QString gps_data::toString() {
    return QString("Index: %1, Xs: %2, Ys: %3, Offset: %4")
    .arg(index)
        .arg(getXs().isEmpty() ? "Empty" : QString::number(getXs().size()))
        .arg(getYs().isEmpty() ? "Empty" : QString::number(getYs().size()))
        .arg(indexOffSet);
}

QVector<double> gps_data::getYs(){
    QVector<double> data;

    for (int i = 0; i < window.size(); i++) {
        // Convert latitude from degrees to radians
        double latitudeDeg = window[i][2].toDouble();
        double latitudeRad = latitudeDeg * M_PI / 180.0;

        // Apply Mercator projection for latitude
        double y = std::log(std::tan(M_PI / 4.0 + latitudeRad / 2.0));

        // Append the transformed y-value
        data.append(y);
    }

    return data;
}

QString gps_data::getFilePath() const {
    return filepath;
}

QString gps_data::getCOG(){
    if (index >= 0 && index < data.size() - windowSize){
        QString temp = window[window.size() - 1][4];
        return temp;
    }
    return "none";
}

QString gps_data::getSOG(){
    if (index >= 0 && index < data.size() - windowSize){
        QString temp = window[window.size() - 1][3];
        return temp;
    }
    return "none";
}

qint64 gps_data::getTimeStamp() {
    if (index >= 0 && index < data.size()) {
        QString timestampStr = data[index][0]; // Get the timestamp string
        bool conversionOk = false;
        double timestampDouble = timestampStr.toDouble(&conversionOk);
        if (conversionOk) {
            return static_cast<qint64>(timestampDouble);
        }
    }
    return -1;
}


void gps_data::setColor(Qt::GlobalColor c){
    color = c;
}
