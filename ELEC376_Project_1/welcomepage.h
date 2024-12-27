#ifndef WELCOMEPAGE_H
#define WELCOMEPAGE_H

#include <QWidget>
#include "mainwindow.h"

namespace Ui {
class WelcomePage;
}

class WelcomePage : public QWidget
{
    Q_OBJECT

public:
    explicit WelcomePage(QWidget *parent = nullptr);
    ~WelcomePage();

private slots:
    void on_continueButton_clicked();

private:
    Ui::WelcomePage *ui;
    MainWindow *mainWindow = nullptr;
};

#endif // WELCOMEPAGE_H
