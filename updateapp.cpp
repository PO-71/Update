#include "updateapp.h"

UpdateApp::UpdateApp(int versionApp)
{
    QString host = "127.0.0.1";
    int port = 5432;
    std::string answer, host_t;
    std::cout << "Host: 127.0.0.1, port: 5432. Confirm? (y / n) ";
    std::cin >> answer;
    if(answer != "y")
    {
        std::cout << "Host: ";
        std::cin >> host_t;
        std::cout << "Port: ";
        std::cin >> port;
        host = QString::fromStdString(host_t);
    }
    db = QSqlDatabase::addDatabase("QPSQL");
    db.setHostName(host);
    db.setPort(port);
    db.setDatabaseName("releases");
    db.setUserName("user");
    db.setPassword("getMeUpdate");

    if(!db.open())
    {
        std::cout << "Error: " << db.lastError().databaseText().toStdString() << std::endl;
        return;
    }
    else
        std::cout << "Successful connection to the update server..." << std::endl;

    QSqlQueryModel *queryModel = new QSqlQueryModel;
    //QString str_query = "SELECT id FROM update WHERE id > " + QString::number(versionApp) + " ORDER BY id DESC";
    QString str_query = "SELECT id FROM update WHERE id > 1010 ORDER BY id DESC";
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
        QString version = queryModel->data(queryModel->index(0, 1)).toString();
        std::cout << "A new version (" << version.toStdString() << ") of the program is available. Download and install? (y / n) ";
        std::cin >> answer;
        if(answer != "y") return;
        else
        {
            // скачиваем обновление по ссылке из БД
            QString link = queryModel->data(queryModel->index(0, 0)).toString();
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
        QFile file(QCoreApplication::applicationDirPath() + "/update.zip");
        if(file.open(QIODevice::WriteOnly))
        {
            QByteArray content = reply->readAll();
            file.write(content); // пишем в файл
        }
        else
        {
            std::cout << "Error: File 'update.zip' can't be written." << std::endl;
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
    if(!PostMessageA(FindWindowA(NULL, "Trade"), WM_QUIT, 0, 0))
    {
        std::cout << "Error: The application 'Trade.exe' can't be closed." << std::endl;
        return;
    }
    else
    {
        bool copyApp = false, copyDB = false;
        copyApp = QFile::copy(QCoreApplication::applicationDirPath() + "/Trade.exe", QCoreApplication::applicationDirPath() + "/TradeOld.exe");
        copyDB = QFile::copy(QCoreApplication::applicationDirPath() + "/trade.sqlite", QCoreApplication::applicationDirPath() + "/tradeOld.sqlite");
        if(!(copyApp && copyDB))
        {
            std::cout << "Error: The application 'Trade.exe' or/and the database 'trade.sqlite' can't be copy." << std::endl;
            return;
        }
        // распаковываем архив
        JlCompress::extractDir(QCoreApplication::applicationDirPath() + "/update.zip", QCoreApplication::applicationDirPath() + "/");
        // загрузить скрипт
        QFile file(QCoreApplication::applicationDirPath() + "/script");
        QString str_script;
        if((file.exists()) && (file.open(QIODevice::ReadOnly)))
        {
            str_script = file.readAll();
            file.close();
        }
        else
        {
            std::cout << "Error: The script for updating the database wasn't received." << std::endl;
            QFile::remove(QCoreApplication::applicationDirPath() + "/Trade.exe");
            QFile::copy(QCoreApplication::applicationDirPath() + "/TradeOld.exe", QCoreApplication::applicationDirPath() + "/Trade.exe");
            QFile::remove(QCoreApplication::applicationDirPath() + "/script");
            QFile::remove(QCoreApplication::applicationDirPath() + "/update.zip");
            return;
        }
        // подсоединиться к БД
        db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName(QCoreApplication::applicationDirPath() + "/trade.sqlite");
        if(!db.open())
        {
            std::cout << db.lastError().databaseText().toStdString() << std::endl;
            QFile::remove(QCoreApplication::applicationDirPath() + "/Trade.exe");
            QFile::copy(QCoreApplication::applicationDirPath() + "/TradeOld.exe", QCoreApplication::applicationDirPath() + "/Trade.exe");
            QFile::remove(QCoreApplication::applicationDirPath() + "/script");
            QFile::remove(QCoreApplication::applicationDirPath() + "/update.zip");
            return;
        }
        else
        {
            db.exec("pragma foreign_keys=1;");
            // выполнить скрипт
            QSqlQueryModel *queryModel = new QSqlQueryModel;
            queryModel->setQuery(str_script);
        }
        // очистка от временных файлов. Копию БД оставляем на всякий случай, чтобы можно было восстановить
        bool removeOldApp = false, removeScript = false, removeUpdate = false;
        removeOldApp = QFile::remove(QCoreApplication::applicationDirPath() + "/TradeOld.exe");
        removeScript = QFile::remove(QCoreApplication::applicationDirPath() + "/script");
        removeUpdate = QFile::remove(QCoreApplication::applicationDirPath() + "/update.zip");
        if(!removeOldApp)
            std::cout << "Warning: The application 'TradeOld.exe' can't be remove." << std::endl;
        if(!removeScript)
            std::cout << "Warning: The file 'script' can't be remove." << std::endl;
        if(!removeUpdate)
            std::cout << "Warning: The archive 'update.zip' can't be remove." << std::endl;
        std::cout << "The program has been updated. Close this window." << std::endl;
        QProcess *tradeProcess = new QProcess();
        tradeProcess->start(QCoreApplication::applicationDirPath() + "/Trade.exe");
    }
}
