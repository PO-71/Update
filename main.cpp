#include <QCoreApplication>
#include "updateapp.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    std::string ver = argv[0];
    std::cout << ver << std::endl;
    UpdateApp *update = new UpdateApp(QString::fromStdString(ver).toInt());
    return a.exec();
}
