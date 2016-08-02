#ifndef CONSOLEWIDGET_H
#define CONSOLEWIDGET_H

#include <QTextEdit>

class DtachClient;

class ConsoleWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ConsoleWidget(QWidget *parent = 0);
    ~ConsoleWidget();

    void setFontSize(int pointSize);
    int fontSize() const;

    void clear();

    void setColor(quint8 color);

    void moveCursorUp(int count);
    void moveCursorDown(int count);
    void moveCursorLeft(int count);
    void moveCursorRight(int count);
    void moveCursorStartOfLine();

    void scrollUp();

    void print(const QString &text);
    void putchar(QChar c);

    void zoomIn();
    void zoomOut();

signals:
    void inputReceived(QString);

protected:
    void paintEvent(QPaintEvent *);
    void timerEvent(QTimerEvent *);

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
    quint8 getCurrentColor() const;
    void flushBuffer();
    bool processEscapeSequence(const QByteArray &seq);

private:
    DtachClient *client_;

    QFont font_;
    QSize cellSize_;
    int baseline_;

    QColor colors_[16];

    struct Cell {
        QChar c;
        quint8 color;
    };

    int consoleWidth_;
    int consoleHeight_;
    int cursorX_;
    int cursorY_;
    quint8 currentColor_;
    bool currentInverse_;
    bool blink_;

    Cell *cells_;

    QByteArray escapeSequence_;
    QByteArray buffer_;
};

#endif // CONSOLEWIDGET_H
