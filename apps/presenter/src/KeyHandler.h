#ifndef SWITCHHELPER_H
#define SWITCHHELPER_H

#include <QObject>

class QLabel;
class SwitcherWidget;
class DtachClient;

class KeyHandler : public QObject
{
    Q_OBJECT
public:
    explicit KeyHandler(SwitcherWidget *switcher, QLabel *webView, QObject *parent = 0);

protected:
    bool eventFilter(QObject *, QEvent *);

private:
    SwitcherWidget *switcher_;
    QLabel *webView_;
    DtachClient *client_;
};

#endif // SWITCHHELPER_H
