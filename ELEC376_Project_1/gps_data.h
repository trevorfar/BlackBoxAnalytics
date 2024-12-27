#ifndef GPS_DATA_H
#define GPS_DATA_H
#include <QFile>
#include <QTextStream>
#include <QVector>
#include <QStringList>
#include <QDebug>
#include <QString>

class gps_data
{
public:
    QVector<QVector<QString>> data;     // Holds all data from csv file
    QVector<QVector<QString>> window;   // Holds window of data from csv file
    int index = 0;                      // Holds base index in data
    int indexOffSet = 0;                // Offset from main index - first file uploaded
    long double firstTime = 0;
    int windowSize = 60;
    QString name = "";
    Qt::GlobalColor color;


    QString toString();
    gps_data();
    void setColor(Qt::GlobalColor c);
    void loadCSVToArray(const QString filepath);
    void createWindow();
    void incrementWindow();
    void calculateOffset(QVector<gps_data> &datalist);
    QVector<double> getXs();
    QVector<double> getYs();
    qint64 getTimeStamp();
    QString getSOG();
    QString getCOG();

    QString getFilePath() const;

    bool operator==(const gps_data &other) const {
        qDebug() << other.name << "   " << name;
        return name == other.name && firstTime == other.firstTime;
    }
private:
    QString filepath;
};

#endif // GPS_DATA_H
