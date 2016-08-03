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
    void setWindowSize(int col, int row, int xpixel, int ypixel);

signals:
    void error();
    void dataReceived(QByteArray);

public slots:
    void sendData(const QByteArray &data);

private slots:
    void connected();
    void disconnected();
    void readyRead();

private:
    QLocalSocket *socket_;

    int cachedCol_;
    int cachedRow_;
    int cachedXPixel_;
    int cachedYPixel_;
};

#endif // DTACHCLIENT_H
