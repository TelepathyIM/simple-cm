#include "MainWindow.hpp"
#include <QApplication>

#include <TelepathyQt/Debug>

int main(int argc, char *argv[])
{
    Tp::enableDebug(true);
    Tp::enableWarnings(true);

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
