#include "DtachClient.h"
#include <pty.h>

#include <QLocalSocket>

enum
{
    MSG_PUSH	= 0,
    MSG_ATTACH	= 1,
    MSG_DETACH	= 2,
    MSG_WINCH	= 3,
    MSG_REDRAW	= 4,
};

enum
{
    REDRAW_UNSPEC	= 0,
    REDRAW_NONE	= 1,
    REDRAW_CTRL_L	= 2,
    REDRAW_WINCH	= 3,
};

/* The client to master protocol. */
struct packet
{
    unsigned char type;
    unsigned char len;
    union
    {
        unsigned char buf[sizeof(struct winsize)];
        struct winsize ws;
    } u;
};


DtachClient::DtachClient(QObject *parent) :
    QObject(parent),
    socket_(0),
    cachedCol_(80),
    cachedRow_(24),
    cachedXPixel_(0),
    cachedYPixel_(0)
{
}

void DtachClient::attach(const QString &filename)
{
    if (socket_)
        socket_->close();
    socket_ = new QLocalSocket(this);
    connect(socket_, SIGNAL(error(QLocalSocket::LocalSocketError)), SIGNAL(error()));
    connect(socket_, SIGNAL(disconnected()), SIGNAL(error()));
    connect(socket_, SIGNAL(connected()), SLOT(connected()));
    connect(socket_, SIGNAL(readyRead()), SLOT(readyRead()));

    socket_->connectToServer(filename);
}

void DtachClient::setWindowSize(int col, int row, int xpixel, int ypixel)
{
    packet pkt;
    pkt.type = MSG_REDRAW;
    pkt.len = REDRAW_WINCH;
    pkt.u.ws.ws_col = cachedCol_ = col;
    pkt.u.ws.ws_row = cachedRow_ = row;
    pkt.u.ws.ws_xpixel = cachedXPixel_ = xpixel;
    pkt.u.ws.ws_ypixel = cachedYPixel_ = ypixel;

    if (socket_)
        socket_->write((const char *) &pkt, sizeof(pkt));
}

void DtachClient::sendData(const QByteArray &data)
{
    if (!socket_)
        return;

    int index = 0;
    while (index < data.length()) {
        packet pkt;
        pkt.type = MSG_PUSH;
        pkt.len = qMin(data.length() - index, (int) sizeof(pkt.u.buf));
        memcpy(pkt.u.buf, data.constData() + index, pkt.len);
        index += pkt.len;

        socket_->write((const char *) &pkt, sizeof(pkt));
    }
}

void DtachClient::connected()
{
    // Tell the master that we want to attach.
    packet pkt;
    pkt.type = MSG_ATTACH;
    socket_->write((const char *) &pkt, sizeof(pkt));

    // Set the window width and height
    setWindowSize(cachedCol_, cachedRow_, cachedXPixel_, cachedYPixel_);

    // We would like a redraw, too.
    pkt.type = MSG_REDRAW;
    pkt.len = REDRAW_CTRL_L;
    socket_->write((const char *) &pkt, sizeof(pkt));
}

void DtachClient::readyRead()
{
    QByteArray data = socket_->read(4096);
    emit dataReceived(data);
}
