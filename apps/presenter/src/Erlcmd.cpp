#include "Erlcmd.h"

#if 0
#include <QSocketNotifier>
#include <unistd.h>

Erlcmd::Erlcmd(QObject *parent) :
    QObject(parent),
    index_(0)
{
    erl_init(NULL, 0);

    stdinNotifier_ = new QSocketNotifier(STDIN_FILENO, QSocketNotifier::Read, this);
    connect(stdinNotifier_, SIGNAL(activated(int)), SLOT(process()));
}

/**
 * @brief Synchronously send a response back to Erlang
 *
 * @param response what to send back
 */
void Erlcmd::send(ETERM *response)
{
    unsigned char buf[1024];

    if (erl_encode(response, buf + sizeof(uint16_t)) == 0)
        qFatal("erl_encode");

    ssize_t len = erl_term_len(response);
    uint16_t be_len = htons(len);
    memcpy(buf, &be_len, sizeof(be_len));

    len += sizeof(uint16_t);
    ssize_t wrote = 0;
    do {
        ssize_t amount_written = write(STDOUT_FILENO, buf + wrote, len - wrote);
        if (amount_written < 0) {
            if (errno == EINTR)
                continue;

            qFatal("write");
        }

        wrote += amount_written;
    } while (wrote < len);
}

/**
 * @brief Dispatch commands in the buffer
 * @return the number of bytes processed
 */
ssize_t Erlcmd::tryDispatch()
{
    /* Check for length field */
    if (index_ < sizeof(uint16_t))
        return 0;

    uint16_t be_len;
    memcpy(&be_len, buffer_, sizeof(uint16_t));
    ssize_t msglen = ntohs(be_len);
    if (msglen + sizeof(uint16_t) > sizeof(buffer_))
        qFatal("Message too long");

    /* Check whether we've received the entire message */
    if (msglen + sizeof(uint16_t) > index_)
        return 0;

    ETERMPtr emsg(erl_decode(buffer_ + sizeof(uint16_t)), erl_free_term);
    if (emsg.isNull())
        qFatal("erl_decode failed??");

    emit messageReceived(emsg);

    return msglen + sizeof(uint16_t);
}

/**
 * @brief call to process any new requests from Erlang
 */
void Erlcmd::process()
{
    ssize_t amountRead = read(STDIN_FILENO, buffer_, sizeof(buffer_) - index_);
    if (amountRead < 0) {
        /* EINTR is ok to get, since we were interrupted by a signal. */
        if (errno == EINTR)
            return;

        /* Everything else is unexpected. */
        qFatal("read failed");
    } else if (amountRead == 0) {
        /* EOF. Erlang process was terminated. This happens after a release or if there was an error. */
        exit(EXIT_SUCCESS);
    }

    index_ += amountRead;
    for (;;) {
        ssize_t bytesProcessed = tryDispatch();

        if (bytesProcessed == 0) {
            /* Only have part of the command to process. */
            break;
        } else if (index_ > (size_t) bytesProcessed) {
            /* Processed the command and there's more data. */
            memmove(buffer_, &buffer_[bytesProcessed], index_ - bytesProcessed);
            index_ -= bytesProcessed;
        } else {
            /* Processed the whole buffer. */
            index_ = 0;
            break;
        }
    }
}
#endif
