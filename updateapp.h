#ifndef UPDATEAPP_H
#define UPDATEAPP_H
#include <QObject>
#include <QtSql>
#include <QNetworkAccessManager>
#include <QtNetwork>
#include <QFile>
#include <QProcess>
#include <iostream>
#include <windows.h>
#include <JlCompress.h>

class UpdateApp : public QObject
{
Q_OBJECT
public:
    UpdateApp(int versionApp);
private:
    QSqlDatabase db;
    QNetworkAccessManager *manager;

    void replyFinished();
    void updating();
};

#endif // UPDATEAPP_H
