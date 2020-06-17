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
    lg = new Log(logPath);
    lg->createLog("daemon server start");

    if(!udp)
    {
        //если сокет TCP
        server = new QTcpServer(this);
        qDebug() << "server listen = " << server->listen(QHostAddress::Any, port);
        connect(server, SIGNAL(newConnection()), this, SLOT(incommingConnection()));
    }
    else
    {
        //если socket UDP
        udpSocket = new QUdpSocket(this);
        udpSocket->bind(QHostAddress::Any, port);
        connect(udpSocket, SIGNAL(readyRead()), this, SLOT(readAndSendMessage()));
    }
}

void Server::initialization() // получение настроек
{
    //Создание папки bin
    if(!QDir("bin").exists())
    {
        QDir().mkdir("bin");
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
    //Считывание данных из файла .ini
    QFile file(initPath);
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream in(&file);
        bool ok;
        bool ok_2;
        QString port_ = in.readLine();//порт
        QString imagePath_ = in.readLine();// папка с картинками
        QString logPath_ = in.readLine();// папка для логирования
        QString upd_ = in.readLine();// флаг использования UDP
        port_.toUInt(&ok,10);
        upd_.toInt(&ok_2, 10);
        if(!ok || !ok_2 || imagePath_ == "" || logPath_ == "" )
        {
            //если какая то переменная в файле не верна
            qDebug() << ".ini file corrupted... initializate as default";
            createInit();
            initAsDefault();
        }
        else
        {
            port = port_.toUInt(&ok,10);
            imagePath = imagePath_;
            logPath = logPath_;
            udp = upd_.toInt(&ok_2, 10);
            qDebug() << "initialization succes";
            qDebug() << "port = "<<port;
            qDebug() << "image path = "<<imagePath;
            qDebug() << "log path = "<<logPath;
            qDebug() << "use "<<(udp?"UPD":"TCP")<<" socket";
        }
     }
     else
     {
        //если файл конфига не открывается
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
        out<<"log"<<endl;
        out<<"0"<<endl;
    }
    else
    {
        qDebug() << "Create .ini failed";
        return;
    }
    file.close();
    qDebug() << "Create .ini success";
}

void Server::initAsDefault() // инициализация по умолчанию
{
    qDebug() << "initialization with default configuration";
    port = 6666;
    qDebug() << "port = "<<port;
    imagePath = "Images";
    qDebug() << "path = "<<imagePath;
    if(!QDir("Images").exists())
        QDir().mkdir("Images");
    logPath = "log";
    qDebug() << "log path = "<<logPath;
    udp = false;
    qDebug() << "use "<<(udp?"UPD":"TCP")<<" socket";
}


void Server::incommingConnection() // обработчик подключений TCP
{
    QTcpSocket * socket = server->nextPendingConnection();
    lg->createLog(socket, " connected ");
    QDir dir(imagePath);
    if(!dir.exists())//Проверка наличия папки с картинками
    {
        //если папки нет то закрываем соединение
        qDebug() << "Директория "<<this->imagePath<<" не существует.";
        socket->close();
        lg->createLog(socket, + " closed, not exists path with images ");
        return;
    }
    //Получение данных картинок
    QByteArray bmpArray = createData();

    qDebug() << "total bmpArray size "<<bmpArray.size();
    socket->write(bmpArray);
    socket->waitForBytesWritten();
    lg->createLog(socket, " packet send ");
    lg->createLog(socket, " closed ");
    socket->close();
}

void Server::readAndSendMessage() // обработчик UDP
{
    QHostAddress senderHost;
    quint16 senderPort;
    QByteArray junk;
    //Получение сообщения для опрделения ip и порта
    junk.resize(udpSocket->pendingDatagramSize());
    udpSocket->readDatagram(junk.data(), junk.size(), &senderHost, &senderPort);
    lg->createLog(senderHost.toString(), QString::number(senderPort), " recive datagram ");
    int len = 1000;//размер пакетов
    //Получение данных картинок
    QByteArray bmpArray = createData();
    //фрагментируем массив по len и отправляем
    int i = 0;
    do
    {
        udpSocket->writeDatagram(bmpArray.mid(len*(i++), (i*len>bmpArray.size()?bmpArray.size()-(i-1)*len: len)), senderHost, senderPort);
        udpSocket->waitForBytesWritten();
    }
    while(i*len<bmpArray.size());
    qDebug() << "send "<<++i<<" datagramms";
    qDebug() << "total bmpArray size "<<bmpArray.size();
    lg->createLog(senderHost.toString(), QString::number(senderPort), " packet send ");

}

QByteArray Server::createData() //Функция сбора данных файлов в массив
{
    //Прослойка, служит для разделения раздела с описаниями файла (имя размер итд)
    QString prob_1("@FhK#-12");
    //Прослойка, служит для разделения раздела с описаниями файла и раздела с данными файла
    QString prob_2("@FhK#-12ddawd-?1^");

    //Прослойка, служит для разделения раздела с описаниями файла и раздела с данными файла
    QString prob_3("@Fgaw-12awdd435dawd-?1^");

    QByteArray bmpArray;
    QDirIterator itr(imagePath, QStringList()<<"*.bmp", QDir::Files); //ищем только bmp файлы

    //Проход по всем отфильтрованным файлам в папке
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
            lg->createLog("add " + fileName + " size " + fileSize + " into packet");
        }
        else
        {
            lg->createLog("can't open " + fileName);
            qDebug() << "img - "<<fileName<<" cant open size("<<img.size()<<")";
        }
        img.close();
    }
    bmpArray.append(prob_3);
    return bmpArray;
}
