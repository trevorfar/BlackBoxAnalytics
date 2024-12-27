#include "graphs.h"
#include "ui_graphs.h"

#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>


graphs::graphs(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::graphs)
{
    ui->setupUi(this);

    this->setWindowTitle("Graphs");

}

graphs::~graphs()
{
    delete ui;
}

void graphs::getDatas(QVector<gps_data> temp, int index_of_metric_to_display)
{
    if(temp.isEmpty() || index_of_metric_to_display >= temp.size()) {
        qDebug() << "Invalid data or index out of range.";
        return;
    }
    else {
        datas = temp;
        d = datas[index_of_metric_to_display];
    }
}

void graphs::getIndex(int temp)
{
    index = temp;
}

void graphs::velGraph()
{
    series1->clear();

    if (chart1->series().contains(series1))
        chart1->removeSeries(series1);

    for (int i = 0; i <= d.windowSize; i++)
    {
        d.index = i + index - d.windowSize;
        d.createWindow();

        double y = d.getSOG().toDouble();
        series1->append(i, y);
    }

    chart1->addSeries(series1);
    chart1->legend()->hide();
    chart1->createDefaultAxes();
    chart1->setTitle("Speed Over Ground");

    auto *axisX = new QValueAxis();
    auto *axisY = new QValueAxis();

    axisX->setTitleText("Time (seconds)");
    axisY->setTitleText("Speed (m/s)");

    chart1->setAxisX(axisX, series1);
    chart1->setAxisY(axisY, series1);

    chartview1->setRenderHint(QPainter::Antialiasing);
    ui->leftGraph->addWidget(chartview1);
}

void graphs::accGraph()
{
    series2->clear();
    if (chart2->series().contains(series2))
        chart2->removeSeries(series2);

    for (int i = 0; i <= d.windowSize; i++)
    {
        d.index = i + index - d.windowSize;
        d.createWindow();

        double y1 = d.getSOG().toDouble();
        d.index = d.index + 1;
        d.createWindow();
        double y2 = d.getSOG().toDouble();

        double accel = (y2 - y1)/2;
        series2->append(i, accel);
    }

    chart2->addSeries(series2);
    chart2->legend()->hide();
    chart2->createDefaultAxes();
    chart2->setTitle("Acceleration Over Ground");

    auto *axisX = new QValueAxis();
    auto *axisY = new QValueAxis();

    axisX->setTitleText("Time (seconds)");
    axisY->setTitleText("Acceleration (m/s²)");

    chart2->setAxisX(axisX, series2);
    chart2->setAxisY(axisY, series2);


    chartview2->setRenderHint(QPainter::Antialiasing);
    ui->rightGraph->addWidget(chartview2);
}
