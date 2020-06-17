#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QUdpSocket>
#include <QTcpServer>
#include <QTcpSocket>

#include <QDebug>
#include <QByteArray>
#include <QDataStream>
#include <QString>
#include "log.h"

class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = nullptr);
signals:

public slots:

    void incommingConnection(); // обработчик входящего подключения TCP протокола
    void readAndSendMessage(); // обработчик входящего подключения UDP протокола

private:
    QTcpServer *server; // указатель на TCP сервер
    QUdpSocket *udpSocket; // указатель на UDP сокет
    QString initPath = ".ini";
    quint16 port; // порт
    QString imagePath; // адрес папки с картинками
    QString logPath; // адрес папки с картинками
    bool udp = 0;//флаг использования udp  true - UDP  false - TCP
    Log *lg;
    void initialization(); //инициализация
    void initAsDefault(); //инициализация в случае отсутствия или повреждения ini файла
    void createInit(); //Создание ini файла в случае его отсутствия или повреждения
    QByteArray createData();//Функция сбора данных файлов в массив

signals:

};

#endif // SERVER_H
