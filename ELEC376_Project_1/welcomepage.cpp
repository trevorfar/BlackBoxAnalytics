#include "welcomepage.h"
#include "ui_welcomepage.h"

WelcomePage::WelcomePage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::WelcomePage),
    mainWindow(nullptr)
{
    ui->setupUi(this);

    this->setWindowTitle("Welcome");

    connect(ui->continueButton, &QPushButton::clicked, this, &WelcomePage::on_continueButton_clicked);

}

WelcomePage::~WelcomePage()
{
    delete ui;
}

void WelcomePage::on_continueButton_clicked()
{
    if (!mainWindow){
        mainWindow = new MainWindow();
    }
    mainWindow->show();

    this->close();

}



