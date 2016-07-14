#ifndef ERLCMD_H
#define ERLCMD_H

#include <QObject>
#include <QSharedPointer>

#if 0
#include <ei.h>
#include <erl_interface.h>

class QSocketNotifier;
typedef QSharedPointer<ETERM> ETERMPtr;

class Erlcmd : public QObject
{
    Q_OBJECT
public:
    explicit Erlcmd(QObject *parent = 0);

    void send(ETERM *response);

signals:
    void messageReceived(ETERMPtr msg);

private slots:
    void process();
    ssize_t tryDispatch();

private:
    QSocketNotifier *stdinNotifier_;
    unsigned char buffer_[1024];
    size_t index_;
};

#endif

#endif // ERLCMD_H
