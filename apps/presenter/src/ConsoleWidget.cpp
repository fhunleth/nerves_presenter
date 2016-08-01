#include "ConsoleWidget.h"
#include "DtachClient.h"

#include <QKeyEvent>
#include <QTextBlock>
#include <QScrollBar>

#include <ctype.h>

#define ALPHA 240
#define stringify(x) #x

ConsoleWidget::ConsoleWidget(QWidget *parent) :
    QTextEdit(parent)
{
    QString stylesheet = QString("background-color: rgb(0,0,0,%1); color: white; font-family: DejaVu Sans Mono").arg(ALPHA);
    setStyleSheet(stylesheet);

    ansiBg_ = 0;
    ansiFg_ = 7;
    inverse_ = false;

    client_ = new DtachClient(this);
    connect(client_, SIGNAL(dataReceived(QByteArray)), SLOT(dataReceived(QByteArray)));
    connect(client_, SIGNAL(error()), SLOT(error()));

    client_->attach("/tmp/iex_prompt");

    setOverwriteMode(true);
}

void ConsoleWidget::dataReceived(const QByteArray &data)
{
    for (int i = 0; i < data.count(); i++) {
        char c = data.at(i);
        if (escapeSequence_.isEmpty()) {
            // Not in an escape sequence
            if (c == '\e') {
                flushBuffer();
                escapeSequence_.append(c);
            } else
                buffer_.append(c);
        } else {
            // In an escape sequence
            escapeSequence_.append(c);
            if (processEscapeSequence(escapeSequence_))
                escapeSequence_.clear();
        }
    }
    escapeSequence_.clear();

    flushBuffer();
    ensureCursorVisible();
}

void ConsoleWidget::error()
{
    textCursor().insertText("Error from dtach!!");
}

void ConsoleWidget::flushBuffer()
{
    if (!buffer_.isEmpty()) {

        QTextCharFormat format;
        QColor bg = ansiToColor(inverse_ ? ansiFg_ : ansiBg_);
        if (bg == QColor(0, 0, 0))
            format.clearBackground();
        else
            format.setBackground(bg);
        format.setForeground(ansiToColor(inverse_ ? ansiBg_ : ansiFg_));

        QString text = QString::fromUtf8(buffer_);
        QTextCursor cursor = textCursor();
        for (int i = 0; i < text.length(); i++) {
            QChar c = text.at(i);
            switch (c.unicode()) {
            case '\a':
                break;

            case '\b':
                cursor.movePosition(QTextCursor::Left);
                break;

            case '\r':
                cursor.movePosition(QTextCursor::StartOfLine);
                break;

            case '\n':
                cursor.movePosition(QTextCursor::EndOfBlock);
                cursor.insertBlock();
                break;

            default:
                if (!cursor.atBlockEnd())
                    cursor.deleteChar();
                //if (!isprint(c.unicode()))
                //qDebug("Not printable: %d (%s)", c.unicode(), QString(c).toUtf8().constData());
                cursor.insertText(QString(c), format);
                break;
            }
        }
        setTextCursor(cursor);
        buffer_.clear();
    }
}

bool ConsoleWidget::processEscapeSequence(const QByteArray &seq)
{
    if (seq.length() < 3)
        return false;

    // Throw out unknown escape sequences.
    if (seq.at(0) != '\e' || seq.at(1) != '[')
        return true;

    // Must end with a letter
    char cmd = seq.at(seq.count() - 1);
    if (!isalpha(cmd))
        return false;

    const int maxNumbers = 5;
    int num[maxNumbers] = {0};
    int numCount = 0;

    bool gotDigit = false;
    for (int i = 2; i < seq.length() && numCount < maxNumbers; i++) {
        char c = seq.at(i);
        if (isdigit(c)) {
            num[numCount] = num[numCount] * 10 + (c - '0');
            gotDigit = true;
        } else {
            if (gotDigit)
                numCount++;
            gotDigit = false;
        }
    }

    QTextCursor cursor = textCursor();
    switch (cmd) {
    case 'A':
        //cursor.movePosition(QTextCursor::Up, QTextCursor::MoveAnchor, numCount == 0 ? 1 : num[0]);
        break;
    case 'B':
        //cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, numCount == 0 ? 1 : num[0]);
        break;
    case 'C':
        cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, numCount == 0 ? 1 : num[0]);
        break;
    case 'D':
        cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, numCount == 0 ? 1 : num[0]);
        break;
    case 'm':
        if (numCount != 1) {
            qDebug("%s: %c(%d): %d %d %d %d %d", seq.mid(1).constData(), cmd, numCount, num[0], num[1], num[2], num[3], num[4]);
            break;
        }

        switch (num[0]) {
        case 0:
            ansiBg_ = 0;
            ansiFg_ = 7;
            inverse_ = false;
            break;
        case 1:
            if (ansiFg_ < 8)
                ansiFg_ += 8;
            break;
        case 2:
        case 22:
            if (ansiFg_ >= 8)
                ansiFg_ -= 8;
            break;
        case 7:
            inverse_ = true;
            break;

        case 27:
            inverse_ = false;
            break;

        case 30:
        case 31:
        case 32:
        case 33:
        case 34:
        case 35:
        case 36:
        case 37:
            ansiFg_ = num[0] - 30 + (ansiFg_ < 8 ?  0 : 8);
            break;

        case 40:
        case 41:
        case 42:
        case 43:
        case 44:
        case 45:
        case 46:
        case 47:
            ansiBg_ = num[0] - 30 + (ansiBg_ < 8 ?  0 : 8);
            break;

        default:
            qDebug("%s: %c(%d): %d %d %d %d %d", seq.mid(1).constData(), cmd, numCount, num[0], num[1], num[2], num[3], num[4]);
            break;
        }
        break;

    default:
        qDebug("%s: %c(%d): %d %d %d %d %d", seq.mid(1).constData(), cmd, numCount, num[0], num[1], num[2], num[3], num[4]);
        break;
    }

    setTextCursor(cursor);
    return true;
}

QColor ConsoleWidget::ansiToColor(int code)
{
    switch (code) {
    case 0:  return QColor(0x00, 0x00, 0x00);
    case 1:  return QColor(0xaa, 0x00, 0x00);
    case 2:  return QColor(0x00, 0xaa, 0x00);
    case 3:  return QColor(0xaa, 0x55, 0x00);
    case 4:  return QColor(0x00, 0x00, 0xaa);
    case 5:  return QColor(0xaa, 0x00, 0xaa);
    case 6:  return QColor(0x00, 0xaa, 0xaa);
    case 7:  return QColor(0xaa, 0xaa, 0xaa);
    case 8:  return QColor(0x55, 0x55, 0x55);
    case 9:  return QColor(0xff, 0x55, 0x55);
    case 10: return QColor(0x55, 0xff, 0x55);
    case 11: return QColor(0xff, 0xff, 0x55);
    case 12: return QColor(0x55, 0x55, 0xff);
    case 13: return QColor(0xff, 0x55, 0xff);
    case 14: return QColor(0x55, 0xff, 0xff);
    default:
    case 15: return QColor(0xff, 0xff, 0xff);
    }
}

void ConsoleWidget::keyPressEvent(QKeyEvent *e)
{
    QByteArray ansi;
    e->accept();

    switch (e->key()) {
    case Qt::Key_F2:
        zoomIn();
        break;

    case Qt::Key_F3:
        zoomOut();
        break;

    case Qt::Key_Up:
        ansi = "\e[A";
        break;
    case Qt::Key_Down:
        ansi = "\e[B";
        break;
    case Qt::Key_Right:
        ansi = "\e[C";
        break;
    case Qt::Key_Left:
        ansi = "\e[D";
        break;
    case Qt::Key_Delete:
        ansi = "\e[C\b"; // HACK! "\e[3~" doesn't work
        break;
    case Qt::Key_Home:
        ansi = "\eOH";
        break;
    case Qt::Key_End:
        ansi = "\eOF";
        break;
    case Qt::Key_Backspace:
        ansi = "\b";
        break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        ansi = "\n";
        break;
    case Qt::Key_Tab:
        ansi = "\t";
        break;

    case Qt::Key_PageUp:
        verticalScrollBar()->setValue(verticalScrollBar()->value() - verticalScrollBar()->pageStep());
        break;

    case Qt::Key_PageDown:
        verticalScrollBar()->setValue(verticalScrollBar()->value() + verticalScrollBar()->pageStep());
        break;

    default:
        if (e->modifiers() == Qt::ControlModifier &&
                e->key() >= Qt::Key_A &&
                e->key() <= Qt::Key_Z) {
            char code = (char) (e->key() - Qt::Key_A + 1);
            ansi.append(code);
        } else
            ansi = e->text().toUtf8();
        break;
    }

    if (!ansi.isEmpty())
        client_->sendData(ansi);
}

void ConsoleWidget::mousePressEvent(QMouseEvent *e)
{
    e->accept();
}

void ConsoleWidget::mouseReleaseEvent(QMouseEvent *e)
{
    e->accept();
}

void ConsoleWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
    e->accept();
}

