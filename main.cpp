#include <QCoreApplication>
#include "updateapp.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    UpdateApp *update = new UpdateApp(1020);
    return a.exec();
}
