#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include "qcustomplot.h"
#include "gps_data.h"
#include "graphs.h"
#include <QMediaPlayer>
#include <QGraphicsVideoItem>
#include <QSet>



QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QCustomPlot *customPlot;
    QVector<gps_data> datas;
    QVector<Qt::GlobalColor> colors = {Qt::blue, Qt::red, Qt::black};
    int index_of_metric_to_display = 0;
    graphs *graphsWindow = new graphs(this);
    double windowSizeX = 0.0005;
    double windowSizeY = 0.00005;
    void printDatas();
    void deleteData(int index);
    void replot(int value = -1);
    void pauseMedia();
    void unPauseMedia();
    void handleVideoForSliderPosition(qint64 currTime);
    QString currentVideoPath;
    QTimer *sliderTimer;
    void moveSliderToFirstVideo();
    qint64 lastPos = 0;
    int isPlaying = 0;
    qint64 currentTime = 0;


private slots:
    void on_pushButton_clicked();

    void on_uploadFileButton_clicked();


    void on_horizontalSlider_sliderMoved(int position);

    void on_horizontalSlider_valueChanged(int value);

    void on_selectBoat_clicked();

    void on_viewAnalytics_clicked();

    void on_minusButton_clicked();

    void on_plusButton_clicked();

    void on_playPauseButton_clicked();

    void on_uploadVideoButton_clicked();


    void on_skipToFirstVideo_clicked();


    void on_SpeedSlider_valueChanged(int value);

private:
    Ui::MainWindow *ui;
    QMediaPlayer *mediaPlayer;
    QGraphicsVideoItem *videoItem;
    int skipIndex = 0;
    QSet<QString> uploadedFiles;
    int sliderSpeed = 1000;
    int previousSliderValue = 0;

};
#endif // MAINWINDOW_H
