#include <QTcpServer>         // Класс для создания TCP-сервера.
#include <QTcpSocket>         // Класс для работы с TCP-сокетами.
#include <QCoreApplication>   // Основной класс для консольных приложений.
#include <QList>              // Класс для хранения списка клиентов.
#include <QDataStream>        // Класс для сериализации/десериализации данных.
#include <QDebug>             // Для вывода сообщений отладки.
#include <utility>


class MyTcpServer : public QTcpServer {
    Q_OBJECT
public:
    MyTcpServer(QObject *parent = nullptr) : QTcpServer(parent) {}

protected:
    // Переопределяем метод, который вызывается при входящем соединении.
    void incomingConnection(qintptr socketDescriptor) override {
        QTcpSocket *socket = new QTcpSocket(this);
        if (!socket->setSocketDescriptor(socketDescriptor)) {
            socket->deleteLater();
            return;
        }

        // При отключении клиента удаляем его из списка и рассылаем обновлённое количество.
        connect(socket, &QTcpSocket::disconnected, this, [this, socket]() {
            clients.removeAll(socket);
            socket->deleteLater();
            qDebug() << "The client disconnected. Number of clients:" << clients.size();
            sendClientCount();  // Отправляем обновлённое количество всем клиентам.
        });

        // Добавляем нового клиента и рассылаем обновлённое количество.
        clients.append(socket);
        qDebug() << "A new client has connected. Number of clients:" << clients.size();
        sendClientCount(); // Отправляем обновлённое количество всем клиентам.
    }

private:
    QList<QTcpSocket*> clients;  // Список для хранения активных клиентов.

    // Функция отправляет всем клиентам обновлённое количество подключённых клиентов.
    void sendClientCount() {
        QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);
        qint32 clientCount = clients.size();
        out << clientCount;
        // Проходим по всем подключённым клиентам и отправляем им данные.
        for (QTcpSocket *client : std::as_const(clients)) {
            client->write(block);
        }
    }
};

int main(int argc, char *argv[]){

    QCoreApplication a(argc, argv);

    MyTcpServer server;
    if (!server.listen(QHostAddress::Any, 1234)) {
        qCritical() << "The server could not start!";
        return 1;
    }
    qDebug() << "The server is running on port 1234.";

    return a.exec();
}

#include "main.moc"

