#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QDebug>
#include <QByteArray>
#include <QDataStream>
#include <QString>

class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = nullptr);
signals:

public slots:

    void incommingConnection(); // обработчик входящего подключения

private:
    QTcpServer *server; // указатель на сервер
    QString initPath = ".ini";
    quint16 port; // порт
    QString path; // адрес папки с картинками
    void initialization(); //инициализация
    void initAsDefault(); //инициализация в случае отсутствия или повреждения ini файла
    void createInit(); //Создание ini файла в случае его отсутствия или повреждения

signals:

};

#endif // SERVER_H
