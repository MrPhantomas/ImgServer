#include "log.h"
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QTextStream>
#include <QDebug>
#include <QHostAddress>
Log::Log(QString path, QObject *parent) : QObject(parent) , path(path)
{
    checkLogFile();
}

void Log::createLog(QString message) //обычное логирование
{
    QString time = curTime();
    QFile file(path+"/.log");

    if(file.open(QIODevice::Append | QIODevice::Text))
    {
        QTextStream fout(&file);
        QString log = "[ " + time + " ] " + message + "\n";
        fout << log;
    }
    file.close();
}

void Log::createLog(QTcpSocket * socket, QString message) // логирование с указанием ip и порта для Tcp
{
    createLog(" " + socket->peerAddress().toString() + " port " + QString::number(socket->peerPort()) + " " + message);
}

void Log::createLog(QString ip, QString port, QString message) // логирование с указанием ip и порта
{
    QString time = curTime();
    QFile file(path + "/.log" );

    if(file.open(QIODevice::Append | QIODevice::Text))
    {
        QTextStream fout(&file);
        QString log = "[ " + time + " ] " + ip + " port " + port + " " + message + "\n";
        fout << log;
    }
    file.close();
}

void Log::checkLogFile() //Проверка существовния и создание папки для логирования
{
    if(!QDir(path).exists())
    {
        QDir().mkdir(path);
    }
    if(!QFile(path+"/.log").exists())
    {
        QFile file(path+"/.log");
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        file.close();
    }
}

QString Log::curTime() //Функция возвращает текущее время в нужном формате
{
    return QDateTime::currentDateTime().toString("dd:MM:yyyy hh:mm:ss:zzz");
}

Log* operator<< (Log* obj, QString msg) //Перегрузка для удобства
{
    obj->createLog(msg);
    return obj;
}

