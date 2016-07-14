#ifndef CONSOLEWIDGET_H
#define CONSOLEWIDGET_H

#include <QTextEdit>

class DtachClient;

class ConsoleWidget : public QTextEdit
{
    Q_OBJECT
public:
    explicit ConsoleWidget(QWidget *parent = 0);

signals:
    void inputReceived(QString);

protected:
    void keyPressEvent(QKeyEvent *e);

    // Consume mouse events since the user isn't supposed
    // to be able to move the cursor.
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseDoubleClickEvent(QMouseEvent *e);

private slots:
    void dataReceived(const QByteArray &data);
    void error();

private:
    void flushBuffer();
    bool processEscapeSequence(const QByteArray &seq);
    static QColor ansiToColor(int code);

private:
    DtachClient *client_;

    int ansiFg_;
    int ansiBg_;
    bool inverse_;

    QByteArray escapeSequence_;
    QByteArray buffer_;
};

#endif // CONSOLEWIDGET_H
