#include "welcomepage.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);


    WelcomePage welcome;
    welcome.show();

    return a.exec();
}
