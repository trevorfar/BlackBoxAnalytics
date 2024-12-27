#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "gps_data.h"
#include "select_boat_dialog.h"
#include <QMediaMetaData>
#include <QTimer>

struct VideoData {
    QString filePath;
    qint64 creationTimestamp;
    qint64 endTimestamp;
};

QVector<VideoData> videoDataList;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),
    mediaPlayer(new QMediaPlayer(this)),
    videoItem(new QGraphicsVideoItem())
{
    ui->setupUi(this);
    ui->SpeedSlider->setMinimum(0);            // Lowest value
    ui->SpeedSlider->setMaximum(3);           // Highest value
    ui->SpeedSlider->setTickInterval(1);       // Distance between ticks
    ui->SpeedSlider->setSingleStep(1);         // Ensure snapping to ticks
    ui->SpeedSlider->setPageStep(1);
    ui->SpeedSlider->setTickPosition(QSlider::TicksBelow);

    this->setWindowTitle("Black-Box Analytics");

    // Create a QCustomPlot object
    customPlot = new QCustomPlot();

    // Set up sample data
    QVector<double> x(101), y(101);  // Generate data
    for (int i=0; i<101; ++i) {
        y[i] = i / 10.0  - 5.0;  // y goes from -5 to 5
        x[i] = y[i] * y[i];  // x = y^2
    }
    customPlot->xAxis->setVisible(false);
    customPlot->yAxis->setVisible(false);

    if(customPlot->graphCount() > 0){
        customPlot->graph(0)->setData(x, y);
        customPlot->graph(0)->setLineStyle(QCPGraph::lsNone);  // No Line style to connect points
        customPlot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));  // Adds circles for each point
    }else {
        qDebug() << "No graphs added";
    }

    customPlot->rescaleAxes();
    customPlot->resize(ui->graphicsView->size());

    // Add QCustomPlot widget to the QGraphicsScene
    QGraphicsScene *scene = new QGraphicsScene();
    scene->addWidget(customPlot);

    QGraphicsScene *videoScene = new QGraphicsScene();
    videoItem->setSize(QSizeF(480, 360));
    videoScene->addItem(videoItem);

    ui->videoGraphicsView->setScene(videoScene);
    mediaPlayer->setVideoOutput(videoItem);

    ui->graphicsView->setScene(scene);
    sliderTimer = new QTimer(this);


    //Inilize global variables
    windowSizeX = 0.0005;
    windowSizeY = 0.00005;

}

MainWindow::~MainWindow()
{
    delete ui;
}

// Unused
void MainWindow::on_pushButton_clicked(){}

void MainWindow::on_uploadFileButton_clicked()
{
    QString csvFileName = QFileDialog::getOpenFileName(
        this,
        "Select a CSV File",
        QDir::homePath(),
        "CSV Files (*.csv);;All Files (*)"
        );

    if (csvFileName.isEmpty()) {
        qDebug() << "No CSV file selected";
        return;  // Exit if no CSV file is selected
    }

    if (uploadedFiles.contains(csvFileName)){
        QMessageBox::information(this, "Duplicate File", "This file has already been uploaded.");
        return;
    }


    // Process CSV file
    gps_data *gpsd = new gps_data();
    gpsd->loadCSVToArray(csvFileName);

    gpsd->calculateOffset(datas);
    datas.append(*gpsd);

    uploadedFiles.insert(csvFileName);

    for (int i = 0; i < datas.size(); i++) {
        datas[i].calculateOffset(datas);
    }
    gpsd->createWindow();

    customPlot->addGraph();
    on_horizontalSlider_valueChanged(ui->horizontalSlider->value());
    if (ui->horizontalSlider->maximum() < gpsd->data.size()) {
        ui->horizontalSlider->setMaximum(gpsd->data.size());
    }

    qDebug() << "Selected files: CSV -" << csvFileName ;
}


void MainWindow::on_horizontalSlider_sliderMoved(int position)
{
}


void MainWindow::on_horizontalSlider_valueChanged(int value)
{
    if(lastPos + 1 == value && sliderSpeed == 1000){
        replot(value);
        lastPos = value;
        return;
    }
    replot(value);
    lastPos = value;
    handleVideoForSliderPosition(currentTime);
}

void MainWindow::replot(int value){

    for(int i = 0; i < datas.size(); i++){
        auto d = datas[i];

        d.index = ((value == -1) ? ui->horizontalSlider->value() : value) + d.indexOffSet;

        // Get data index at slider
        graphsWindow->getIndex(d.index);

        d.createWindow();

        // Sets color
        customPlot->graph(i)->setPen(QPen(colors[i]));
        // Plot Data
        customPlot->graph(i)->setData(d.getXs(), d.getYs());
        customPlot->graph(i)->setLineStyle(QCPGraph::lsNone);  // No Line style to connect points
        customPlot->graph(i)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));  // Adds circles for each point

        // Create graphs
        graphsWindow->velGraph();
        graphsWindow->accGraph();

        // Centers Plot
        if (i == index_of_metric_to_display) { //always center on boat 0 for now, Change 0 to global variable for select
            //set x range
            if (!d.getXs().isEmpty()) {
                double currentX = d.getXs().last();
                customPlot->xAxis->setRange(currentX - windowSizeX / 2.0, currentX + windowSizeX / 2.0);
            }
            //set y range
            if (!d.getYs().isEmpty()) {
                double currentY = d.getYs().last();
                customPlot->yAxis->setRange(currentY - windowSizeY / 2.0, currentY + windowSizeY / 2.0);
            }
        }
        customPlot->replot();
        // Update Metric Labels
        if (i == index_of_metric_to_display) {
            ui->metricLabel1->setText("COG: " + d.getCOG());
            ui->metricLabel2->setText("SOG: " + d.getSOG());
            }
            //1721832744917
            currentTime = d.getTimeStamp();
    }
}

void MainWindow::handleVideoForSliderPosition(qint64 currTime) {
    bool videoMatched = false;

    for (const auto &videoData : videoDataList) {
        if (currTime >= videoData.creationTimestamp && currTime <= videoData.endTimestamp) {
            videoMatched = true;

            // Update media source only if it's a different video
            if (currentVideoPath != videoData.filePath) {
                currentVideoPath = videoData.filePath;
                mediaPlayer->setSource(QUrl::fromLocalFile(videoData.filePath));
                mediaPlayer->pause(); // Start playing the new video
            }

            // Adjust playback position
            qint64 videoPosition = currTime - videoData.creationTimestamp;
            mediaPlayer->setPosition(videoPosition);
            if(isPlaying) mediaPlayer->play();
            // Ensure the video item is visible
            if (videoItem && !videoItem->isVisible()) {
                videoItem->setVisible(true);
            }
            break; // Stop after finding the matching video
        }
    }

    if (!videoMatched) {
        pauseMedia(); // Pause if no video matches
        currentVideoPath.clear(); // Reset the current video path

        // Ensure the video item is hidden
        if (videoItem && videoItem->isVisible()) {
            videoItem->setVisible(false);
        }
    }
}


void MainWindow::on_selectBoat_clicked()
{
    // Shows Dialog Box
    select_boat_dialog dialog(datas, index_of_metric_to_display, this);
    dialog.exec();
}

void MainWindow::on_viewAnalytics_clicked()
{
    // Get the global gps data
    graphsWindow->getDatas(datas, index_of_metric_to_display);

    // Create graphs
    graphsWindow->velGraph();
    graphsWindow->accGraph();

    // Show the new window
    graphsWindow->show();
}

void MainWindow::deleteData(int index){
    //remove file from uploadedFiles
    QString csvfilePath = datas[index].getFilePath();
    uploadedFiles.remove(csvfilePath);
    qDebug() << "Removed file from uploadedFiles:" << csvfilePath;

    // Remove Data
    datas.removeAt(index);
    customPlot->removeGraph(index);

    // Clear plot
    for (int  i = 0; i < customPlot->graphCount(); i++){
        customPlot->graph(i)->setData({0}, {0});
    }
    customPlot->replot();

    // Recaltulate offsets
    for (auto &data : datas){
        data.calculateOffset(datas);
    }

    // Replot
    on_horizontalSlider_valueChanged(ui->horizontalSlider->value());
}



void MainWindow::on_minusButton_clicked()
{
    windowSizeX *= 1.25;
    windowSizeY *= 1.25;
    replot(ui->horizontalSlider->value());
}

void MainWindow::on_plusButton_clicked()
{
    windowSizeX *= 0.75;
    windowSizeY *= 0.75;
    replot(ui->horizontalSlider->value());
}

void MainWindow::moveSliderToFirstVideo() {
    if (videoDataList.isEmpty() || datas.isEmpty()) {
        QMessageBox::warning(this, "No Videos or Data", "Ensure videos and data are uploaded.");
        return;
    }

    if(skipIndex >= videoDataList.size()){
        skipIndex = 0;
    }

    // Find the first video timestamp
    qint64 videoTimestamp = videoDataList[skipIndex].creationTimestamp;
    qDebug() << "FIrst Vid" << videoTimestamp;

    for (int i = 0; i <datas.size(); i++) {
        const auto &dataVector = datas[i].data;
        for (int j = 0; j < dataVector.size(); j++) {
            QString timestampStr = dataVector[j][0];
            bool conversionOk = false;
            double timestampDouble = timestampStr.toDouble(&conversionOk);
            if (conversionOk) {
               qint64 timestamp = static_cast<qint64>(timestampDouble);
                if(timestamp >= videoTimestamp){
                    ui->horizontalSlider->setValue(j);
                    on_horizontalSlider_valueChanged(j);
                    skipIndex++;
                    return;
                }
            }
        }
    }
}



void MainWindow::on_playPauseButton_clicked()
{
    if (mediaPlayer->playbackState() == mediaPlayer->PausedState || mediaPlayer->playbackState() == mediaPlayer->StoppedState) {
        mediaPlayer->play();  // Start playback
        isPlaying = 1;
    } else {
        mediaPlayer->pause();  // Pause playback
        isPlaying = 0;
    }

    if (!sliderTimer->isActive()) {
        connect(sliderTimer, &QTimer::timeout, this, [this]() {
            // Move the slider forward
            int currentValue = ui->horizontalSlider->value();
            int maxValue = ui->horizontalSlider->maximum();

            if (currentValue < maxValue) {
                ui->horizontalSlider->setValue(currentValue + 1);
            } else {
                sliderTimer->stop();  // Stop the timer if it reaches the end
                isPlaying = 0;
            }
        });
        sliderTimer->start(sliderSpeed);  // Move slider every second
    } else if (sliderTimer->isActive()) {
        sliderTimer->stop();
        isPlaying = 0;
    }

}


void MainWindow::on_uploadVideoButton_clicked() {
    QStringList fileNames = QFileDialog::getOpenFileNames(this, "Select Video Files", "", "Video Files (*.mp4 *.avi *.mkv *.mov)");

    if (fileNames.isEmpty()) {
        return;
    }

    if (videoDataList.size() + fileNames.size() > 10) {
        QMessageBox::warning(this, "Limit Exceeded", "You can only upload up to 10 videos in total.");
        return;
    }

    qint64 hardcodedTimestamps[10];

    for(int i =0; i < 10; i++){
        hardcodedTimestamps[i] = 1721916321000 + i*150000;
    }

    int timestampIndex = 0;

     for (const QString &fileName : fileNames) {
    //     // Show a dialog to get the creation timestamp
    //     // QDialog dateTimeDialog(this);
    //     // dateTimeDialog.setWindowTitle("Enter Video Creation Date and Time");

    //     // QVBoxLayout layout(&dateTimeDialog);
    //     // QLabel label("Enter creation date and time (MM-DD-YYYY HH:mm:ss):");
    //     // QDateTimeEdit dateTimeEdit;
    //     // dateTimeEdit.setDisplayFormat("MM-dd-yyyy HH:mm:ss");
    //     // dateTimeEdit.setCalendarPopup(true);
    //     // dateTimeEdit.setDateTime(QDateTime::currentDateTime());

    //     // QPushButton okButton("OK");
    //     // QPushButton cancelButton("Cancel");

    //     // layout.addWidget(&label);
    //     // layout.addWidget(&dateTimeEdit);
    //     // layout.addWidget(&okButton);
    //     // layout.addWidget(&cancelButton);

    //     // connect(&okButton, &QPushButton::clicked, &dateTimeDialog, &QDialog::accept);
    //     // connect(&cancelButton, &QPushButton::clicked, &dateTimeDialog, &QDialog::reject);

        //if (dateTimeDialog.exec() == QDialog::Accepted) {
             qint64 creationTimestamp = 0;//= dateTimeEdit.dateTime().toMSecsSinceEpoch();

            // Uncomment this for testing purposes to overwrite user input with a hardcoded timestamp
            if (timestampIndex < sizeof(hardcodedTimestamps) / sizeof(hardcodedTimestamps[0])) {
                creationTimestamp = hardcodedTimestamps[timestampIndex++];
            }

            // Create a QMediaPlayer instance to load the media and get the duration
            QMediaPlayer *mediaPlayer = new QMediaPlayer(this);
            mediaPlayer->setSource(QUrl::fromLocalFile(fileName));

            // Wait for the media player to load and retrieve the duration
            QEventLoop loop;
            connect(mediaPlayer, &QMediaPlayer::durationChanged, &loop, &QEventLoop::quit);
            connect(mediaPlayer, &QMediaPlayer::mediaStatusChanged, [&](QMediaPlayer::MediaStatus status) {
                if (status == QMediaPlayer::LoadedMedia || status == QMediaPlayer::InvalidMedia) {
                    loop.quit();
                }
            });

            mediaPlayer->play();
            loop.exec();

            qint64 duration = mediaPlayer->duration();
            if (duration > 0) {
                qint64 endTimestamp = creationTimestamp + duration;
                VideoData newVideoData;
                newVideoData.filePath = fileName;
                newVideoData.creationTimestamp = creationTimestamp;
                newVideoData.endTimestamp = endTimestamp;
                videoDataList.append(newVideoData);
            } else {
                qDebug() << "Could not retrieve video duration for file:" << fileName;
            }

            mediaPlayer->deleteLater();
        }


    //}
}

void MainWindow::pauseMedia()
{
    if (mediaPlayer->playbackState() == QMediaPlayer::PlayingState) {
        mediaPlayer->stop();

        if (videoItem) {
            videoItem->setVisible(false);
        }
    }
}

void MainWindow::unPauseMedia()
{
    if (mediaPlayer->playbackState() == QMediaPlayer::PausedState) {
        if (videoItem) {
            videoItem->setVisible(true);
        }
    }
}




void MainWindow::on_skipToFirstVideo_clicked()
{
    moveSliderToFirstVideo();

}




void MainWindow::on_SpeedSlider_valueChanged(int value)
{
    static const int tickValues[] = {1000, 750, 500, 250};  // Tick value mapping
    sliderSpeed = tickValues[value];
    qDebug() << "Slider Speed:" << sliderSpeed;
    if (sliderTimer->isActive()) {
        sliderTimer->start(sliderSpeed);
    }

}

