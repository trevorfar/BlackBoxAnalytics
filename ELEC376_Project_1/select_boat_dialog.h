#ifndef SELECT_BOAT_DIALOG_H
#define SELECT_BOAT_DIALOG_H
#include "gps_data.h"
#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QVector>
#include <QString>
#include "mainwindow.h"
class select_boat_dialog : public QDialog
{
    Q_OBJECT
public:
    // QVector<gps_data> datas;
    explicit select_boat_dialog(QVector<gps_data> &data, int &index_of_metric_to_display, MainWindow *main, QWidget *parent = nullptr)
    : QDialog(parent) {
        // Set window title
        setWindowTitle("GPS Data Dialog");

        // Create a vertical layout for the entire dialog
        QVBoxLayout *mainLayout = new QVBoxLayout(this);

        // Add labels and buttons
        createLabelButtonColumn(data, index_of_metric_to_display, main);

        // Set layout for the dialog
        setLayout(mainLayout);
    }

    void createLabelButtonColumn(const QVector<gps_data> &data, int &index_of_metric_to_display, MainWindow *main)
    {
        QVBoxLayout *columnLayout = new QVBoxLayout;

        for (int i = 0; i < data.size(); i++) {
            // Access the current item using the index i
            const auto item = data[i];

            // Create a horizontal layout for each label-button pair
            QHBoxLayout *rowLayout = new QHBoxLayout;

            // Create label and buttons for each gps_data object
            QLabel *nameLabel = new QLabel(item.name, this);
            QPushButton *setMetricButton = new QPushButton("Set Metric", this);
            QPushButton *removeButton = new QPushButton("Remove", this);

            // Connect the set metric button to set the metric
            connect(setMetricButton, &QPushButton::clicked, this, [item, &index_of_metric_to_display, main, i]() {
                qDebug() << "Metric Set for " << item.name;
                index_of_metric_to_display = i;
                main->replot();
            });

            // Connect the remove button to the delete function in MainWindow
            connect(removeButton, &QPushButton::clicked, this, [main, rowLayout, columnLayout, &data, item]() {
                qDebug() << "Removing data at index " << data.indexOf(item);

                // Call the deleteData function in MainWindow
                main->deleteData(data.indexOf(item));

                // Remove all widgets in the row layout
                while (QLayoutItem *item = rowLayout->takeAt(0)) {
                    if (QWidget *widget = item->widget()) {
                        widget->deleteLater();
                    }
                    delete item; // Delete the layout item
                }

                // Remove the row layout itself from the column layout
                columnLayout->removeItem(rowLayout);
                delete rowLayout; // Delete the row layout
            });

            // Add label and buttons to the row layout
            rowLayout->addWidget(nameLabel);
            rowLayout->addWidget(setMetricButton);
            rowLayout->addWidget(removeButton);


            // Add the row layout to the column layout
            columnLayout->addLayout(rowLayout);
        }

        // Add the complete column layout to the main layout of the dialog
        layout()->addItem(columnLayout);
    }
};

#endif // SELECT_BOAT_DIALOG_H
