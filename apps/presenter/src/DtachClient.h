#ifndef DTACHCLIENT_H
#define DTACHCLIENT_H

#include <QObject>

class QLocalSocket;

class DtachClient : public QObject
{
    Q_OBJECT
public:
    explicit DtachClient(QObject *parent = 0);

    void attach(const QString &filename);

signals:
    void error();
    void dataReceived(QByteArray);

public slots:
    void sendData(const QByteArray &data);

private slots:
    void connected();
    void readyRead();

private:
    QLocalSocket *socket_;
};

#endif // DTACHCLIENT_H
