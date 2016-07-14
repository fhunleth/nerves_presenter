#include "ErlWebView.h"

ErlWebView::ErlWebView(QWebView *webView, QObject *parent) :
    QObject(parent),
    webView_(webView)
{
    //cmdprocessor_ = new Erlcmd(this);
    //connect(cmdprocessor_, SIGNAL(messageReceived(ETERMPtr)), SLOT(handleMessage(ETERMPtr)));
}

#if 0
void ErlWebView::handleMessage(ETERMPtr msg)
{
    ETERM *emsg = msg.data();

    // Commands are of the form {Command, Arguments}:
    // { atom(), [term()] }

    ETERM *cmd = erl_element(1, emsg);
    ETERM *args = erl_element(2, emsg);
    if (cmd == NULL || args == NULL)
        qFatal("Expecting { cmd, args }");

    ETERM *resp = 0;
    if (strcmp(ERL_ATOM_PTR(cmd), "set_url") == 0) {
        ETERM *eurl = erl_hd(args);
        if (eurl == NULL || !ERL_IS_BINARY(eurl))
            qFatal("error: didn't get url");

//        QUrl url(QString::fromUtf8((const char *) ERL_BIN_PTR(eurl), ERL_BIN_SIZE(eurl)));
//        webView_->setUrl(url);
    } else {
        resp = erl_format((char*) "error");
    }

    if (resp) {
        cmdprocessor_->send(resp);
        erl_free_term(resp);
    }

    erl_free_term(cmd);
    erl_free_term(args);
}
#endif
