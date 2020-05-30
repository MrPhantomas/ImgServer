#include "server.h"
#include <QByteArray>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QPixmap>

#include <QDirIterator>


Server::Server(QObject *parent) :
    QObject(parent)
{
    initialization();
    server = new QTcpServer(this);
    qDebug() << "server listen = " << server->listen(QHostAddress::Any, port);
    connect(server, SIGNAL(newConnection()), this, SLOT(incommingConnection()));
}

void Server::initialization() // получение настроек
{
    //Создание папки bin
    if(!QDir("bin").exists())
    {
        QDir().mkdir("bin");
    }
    //Создание папки log
    if(!QDir("log").exists())
    {
        QDir().mkdir("log");
    }
    //Создание папки etc
    if(!QDir("etc").exists())
    {
        QDir().mkdir("etc");
    }
    //Проверка наличия .ini файла
    if(!QFile(initPath).exists())
    {
        createInit();
        initAsDefault();
        return;
    }

    QFile file(initPath);
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream in(&file);
        bool ok;
        QString port_ = in.readLine();
        QString path_ = in.readLine();

        port = port_.toUInt(&ok,10);
        path = path_;

        if(!ok || path_ == "")
        {
            qDebug() << ".ini file corrupted... initializate as default";
            createInit();
            initAsDefault();
        }
        else
        {
            qDebug() << "initialization succes";
            qDebug() << "port = "<<port;
            qDebug() << "path = "<<path;
        }
     }
     else
     {
        qDebug() << "cant open .ini file";
        initAsDefault();
     }
     file.close();
}

void Server::createInit() // получение настроек
{
    QFile file(initPath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out<<"6666"<<endl;
        out<<"Images"<<endl;
        if(!QDir("Images").exists())
            QDir().mkdir("Images");
    }
    else
    {
        qDebug() << "Create .ini failed";
        return;
    }
    file.close();
    qDebug() << "Create .ini success";
}

void Server::initAsDefault() // получение настроек
{
    qDebug() << "initialization with default configuration";
    port = 6666;
    qDebug() << "port = "<<port;
    path = "Images";
    qDebug() << "path = "<<path;
    if(!QDir("Images").exists())
        QDir().mkdir("Images");
}


void Server::incommingConnection() // обработчик подключений
{
    //Прослойка, служит для разделения раздела с описаниями файла (имя размер итд)
    QString prob_1("@FhK#-12");
    //Прослойка, служит для разделения раздела с описаниями файла и раздела с данными файла
    QString prob_2("@FhK#-12ddawd-?1^");

    QTcpSocket * socket = server->nextPendingConnection();
    QDir dir(path);
    if(!dir.exists())//Проверка наличия папки с картинками
    {
        qDebug() << "Директория "<<this->path<<" не существует.";
        socket->close();
        return;
    }
    QByteArray bmpArray;
    QDirIterator itr(path, QStringList()<<"*.bmp", QDir::Files); //ищем только bmp файлы
    while(itr.hasNext())
    {
        QFile img(itr.next());
        QFileInfo fInfo(img.fileName());
        QString fileName = fInfo.completeBaseName()+"."+fInfo.completeSuffix(); //Получение имени файла
        if(img.open(QIODevice::ReadOnly))
        {
            QString fileSize = QString::number(img.size());//Получение размера файла
            qDebug() << "img - "<<fileName<<" open size("<<fileSize<<")";
            if(!bmpArray.size())
                bmpArray.append(fileName+prob_1+fileSize+prob_2);
            else
                bmpArray.append(prob_2+fileName+prob_1+fileSize+prob_2);
            bmpArray.append(img.readAll());
        }
        else
        {
            qDebug() << "img - "<<fileName<<" cant open size("<<img.size()<<")";
        }
        img.close();
    }
    qDebug() << "total bmpArray size "<<bmpArray.size();
    socket->write(bmpArray);
    socket->waitForBytesWritten();
    socket->close();
}
