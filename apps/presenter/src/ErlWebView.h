#ifndef ERLWEBVIEW_H
#define ERLWEBVIEW_H

#include <QObject>
#include "Erlcmd.h"

class QWebView;

class ErlWebView : public QObject
{
    Q_OBJECT
public:
    explicit ErlWebView(QWebView *webView, QObject *parent = 0);
    
private slots:
    //void handleMessage(ETERMPtr msg);

private:
    //Erlcmd *cmdprocessor_;
    QWebView *webView_;
};

#endif // ERLWEBVIEW_H
