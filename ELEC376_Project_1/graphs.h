#ifndef GRAPHS_H
#define GRAPHS_H

#include <QMainWindow>
#include <QWidget>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>

#include "gps_data.h"

QT_BEGIN_NAMESPACE
namespace Ui { class graphs; }
QT_END_NAMESPACE

class graphs : public QMainWindow
{
    Q_OBJECT

public:
    explicit graphs(QWidget *parent = nullptr);
    QVector<gps_data> datas;
    int index;
    gps_data d;

    // Creates instance for velocity graph
    QLineSeries *series1 = new QLineSeries();
    QChart *chart1 = new QChart();
    QChartView *chartview1 = new QChartView(chart1);

    // Creates instance for acceleration graph
    QLineSeries *series2 = new QLineSeries();
    QChart *chart2 = new QChart();
    QChartView *chartview2 = new QChartView(chart2);

    void getDatas(QVector<gps_data> temp, int index_of_metric_to_display);
    void getIndex(int temp);
    void velGraph();
    void accGraph();

    ~graphs();

private:
    Ui::graphs *ui;


};

#endif // GRAPHS_H
