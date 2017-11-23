#include "updateapp.h"

UpdateApp::UpdateApp(int versionApp)
{
    db = QSqlDatabase::addDatabase("QPSQL");
    db.setHostName("127.0.0.1");
    db.setPort(5432);
    db.setDatabaseName("releases");
    db.setUserName("user");
    db.setPassword("getMeUpdate");

    if(!db.open())
    {
        std::cout << db.lastError().databaseText().toStdString() << std::endl;
        return;
    }
    else
        std::cout << "Successful connection to the update server...";

    QSqlQueryModel *queryModel = new QSqlQueryModel;
    QString str_query = "SELECT id FROM update WHERE id > " + QString::number(versionApp) + " ORDER BY id DESC";
    queryModel->setQuery(str_query);
    int idLastVersion = queryModel->data(queryModel->index(0, 0)).toInt();
    if(queryModel->rowCount() <= 0)
    {
        std::cout << "The latest version of the program is installed. Update is not required.";
        return;
    }
    else
    {
        str_query = "SELECT link, name FROM update WHERE id = " + QString::number(idLastVersion) + "";
        queryModel->setQuery(str_query);
        QString link = queryModel->data(queryModel->index(0, 0)).toString();
        QString version = queryModel->data(queryModel->index(0, 1)).toString();
        std::string answer;
        std::cout << "A new version (" << version.toStdString() << ") of the program is available. Download and install? (y / n) ";
        std::cin >> answer;
        if(answer != "y") return;
        else
        {
            // скачиваем обновление по ссылке из БД
            manager = new QNetworkAccessManager(this);
            QNetworkReply *reply = manager->get(QNetworkRequest(QUrl(link)));
            QObject::connect(reply, SIGNAL(finished()), this, SLOT(replyFinished()));
        }
    }
}

void UpdateApp::replyFinished()
{
    QNetworkReply *reply= qobject_cast<QNetworkReply *>(sender());
    if (reply->error() == QNetworkReply::NoError)
    {
        QFile file(QCoreApplication::applicationDirPath() + "\\update.zip");
        if(file.open(QIODevice::WriteOnly))
        {
            QByteArray content = reply->readAll();
            file.write(content); // пишем в файл
        }
        else
        {
            std::cout << "File can't be written." << std::endl;
            return;
        }
        reply->deleteLater();
    }
    else
    {
        std::cout << reply->errorString().toStdString() << std::endl;
        reply->deleteLater();
        return;
    }
    updating();
}

void UpdateApp::updating()
{
    PostMessageA(FindWindowA(NULL, "Trade"), WM_QUIT, 0, 0);
    QFile::rename(QCoreApplication::applicationDirPath() + "\\Trade.exe", QCoreApplication::applicationDirPath() + "\\TradeOld.exe");
    QFile::copy(QCoreApplication::applicationDirPath() + "\\trade.sqlite", QCoreApplication::applicationDirPath() + "\\tradeOld.sqlite");
    // распаковываем архив
    JlCompress::extractDir(QCoreApplication::applicationDirPath() + "\\update.zip", QCoreApplication::applicationDirPath() + "\\");
    // загрузить скрипт
    QFile file(QCoreApplication::applicationDirPath() + "\\script");
    QString str_script;
    if ((file.exists())&&(file.open(QIODevice::ReadOnly)))
    {
        str_script = file.readAll();
        file.close();
    }
    // подсоединиться к БД
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(QCoreApplication::applicationDirPath() + "\\trade.sqlite");
    if(!db.open())
    {
        std::cout << db.lastError().databaseText().toStdString() << std::endl;
        return;
    }
    else
    {
        db.exec("pragma foreign_keys=1;");
        // выполнить скрипт
        QSqlQueryModel *queryModel = new QSqlQueryModel;
        queryModel->setQuery(str_script);
    }
    QFile::remove(QCoreApplication::applicationDirPath() + "\\TradeOld.exe");
    QFile::remove(QCoreApplication::applicationDirPath() + "\\tradeOld.sqlite");
    QFile::remove(QCoreApplication::applicationDirPath() + "\\script");
    std::cout << "The program has been updated." << std::endl;
    QProcess *tradeProcess = new QProcess();
    tradeProcess->start(QCoreApplication::applicationDirPath() + "\\Trade.exe");
}
