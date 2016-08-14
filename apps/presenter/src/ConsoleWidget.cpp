#include "ConsoleWidget.h"
#include "DtachClient.h"

#include <QKeyEvent>
#include <QTextBlock>
#include <QScrollBar>
#include <QPainter>

#include <ctype.h>

#define ALPHA 240
#define stringify(x) #x

#define DTACH_PIPE  "/tmp/iex_prompt"

ConsoleWidget::ConsoleWidget(QWidget *parent) :
    QWidget(parent),
    bufferRegion_(0, 0, 200, 100), // max possible on screen at a time
    visibleRegion_(0, 0, 80, 24), // what's currently visible
    cursorLocation_(0, 0),
    blink_(false)
{
    // ANSI colors
    foregroundColors_[0] =  QColor(0x00, 0x00, 0x00);
    foregroundColors_[1] =  QColor(0xaa, 0x00, 0x00);
    foregroundColors_[2] =  QColor(0x00, 0xaa, 0x00);
    foregroundColors_[3] =  QColor(0xaa, 0x55, 0x00);
    foregroundColors_[4] =  QColor(0x00, 0x00, 0xaa);
    foregroundColors_[5] =  QColor(0xaa, 0x00, 0xaa);
    foregroundColors_[6] =  QColor(0x00, 0xaa, 0xaa);
    foregroundColors_[7] =  QColor(0xaa, 0xaa, 0xaa);
    foregroundColors_[8] =  QColor(0x55, 0x55, 0x55);
    foregroundColors_[9] =  QColor(0xff, 0x55, 0x55);
    foregroundColors_[10] = QColor(0x55, 0xff, 0x55);
    foregroundColors_[11] = QColor(0xff, 0xff, 0x55);
    foregroundColors_[12] = QColor(0x55, 0x55, 0xff);
    foregroundColors_[13] = QColor(0xff, 0x55, 0xff);
    foregroundColors_[14] = QColor(0x55, 0xff, 0xff);
    foregroundColors_[15] = QColor(0xff, 0xff, 0xff);

    for (int i = 0; i < 16; i++) {
        backgroundColors_[i] = foregroundColors_[i];
        backgroundColors_[i].setAlpha(192);
    }

    cells_ = new Cell[bufferRegion_.height() * bufferRegion_.width()];
    currentColor_ = 0x07; // white on black background
    currentInverse_ = false;

    client_ = new DtachClient(this);
    connect(client_, SIGNAL(dataReceived(QByteArray)), SLOT(dataReceived(QByteArray)));
    connect(client_, SIGNAL(error()), SLOT(error()));

    client_->attach(DTACH_PIPE);

    setFontSize(12);
    clear();

    startTimer(500);
}

ConsoleWidget::~ConsoleWidget()
{
    delete[] cells_;
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
}

void ConsoleWidget::error()
{
    moveCursorStartOfLine();
    moveCursorDown(2);
    print("Error from dtach. Check '" DTACH_PIPE "'!");
    moveCursorDown(1);
    moveCursorStartOfLine();
}

void ConsoleWidget::flushBuffer()
{
    if (!buffer_.isEmpty()) {
        QString text = QString::fromUtf8(buffer_);
        for (int i = 0; i < text.length(); i++) {
            QChar c = text.at(i);
            switch (c.unicode()) {
            case '\a':
                // Ignore beeps.
                break;

            case '\b':
                moveCursorLeft(1);
                break;

            case '\r':
                moveCursorStartOfLine();
                break;

            case '\n':
                moveCursorDown(1);
                break;

            default:
                putchar(c);
                break;
            }
        }
        buffer_.clear();
    }
    update();
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

    switch (cmd) {
    case 'A':
        moveCursorUp(numCount == 0 ? 1 : num[0]);
        update();
        break;
    case 'B':
        moveCursorDown(numCount == 0 ? 1 : num[0]);
        update();
        break;
    case 'C':
        moveCursorRight(numCount == 0 ? 1 : num[0]);
        update();
        break;
    case 'D':
        moveCursorLeft(numCount == 0 ? 1 : num[0]);
        update();
        break;
    case 'm':
        if (numCount != 1) {
            qDebug("%s: %c(%d): %d %d %d %d %d", seq.mid(1).constData(), cmd, numCount, num[0], num[1], num[2], num[3], num[4]);
            break;
        }

        switch (num[0]) {
        case 0:
            currentColor_ = 0x07;
            currentInverse_ = false;
            break;
        case 1:
            if ((currentColor_ & 0x0f) < 8)
                currentColor_ += 8;
            break;
        case 2:
        case 22:
            if ((currentColor_ & 0x0f) >= 8)
                currentColor_ -= 8;
            break;
        case 7:
            currentInverse_ = true;
            break;

        case 27:
            currentInverse_ = false;
            break;

        case 30:
        case 31:
        case 32:
        case 33:
        case 34:
        case 35:
        case 36:
        case 37:
            currentColor_ = (currentColor_ & ~0x07) | (num[0] - 30);
            break;

        case 40:
        case 41:
        case 42:
        case 43:
        case 44:
        case 45:
        case 46:
        case 47:
            currentColor_ = (currentColor_ & ~0x70) | ((num[0] - 30) << 4);
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

    return true;
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
        visibleRegion_.moveTop(qMax(0, visibleRegion_.top() - visibleRegion_.height() - 1));
        update();
        break;

    case Qt::Key_PageDown:
        visibleRegion_.moveBottom(qMin(bufferRegion_.bottom(), visibleRegion_.bottom() + visibleRegion_.height() - 1));
        update();
        break;

    default:
        // Pass the key on unless it is the detach key "CTRL-\"
        if (!(e->modifiers() == Qt::ControlModifier &&
              e->key() == Qt::Key_Backslash))
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

void ConsoleWidget::setFontSize(int pointSize)
{
    font_ = QFont("DejaVu Sans Mono", pointSize);
    QFontMetrics metrics(font_);
    cellSize_.setHeight(metrics.height());
    cellSize_.setWidth(metrics.averageCharWidth());
    baseline_ = metrics.ascent();
    handleTerminalResize();
}

int ConsoleWidget::fontSize() const
{
    return font_.pointSize();
}

void ConsoleWidget::clear()
{
    quint8 color = getCurrentColor();
    for (int i = 0; i < bufferRegion_.height() * bufferRegion_.width(); i++) {
        cells_[i].c = QLatin1Char(' ');
        cells_[i].color = color;
    }
    cursorLocation_.setX(0);
    cursorLocation_.setY(0);
    visibleRegion_.moveTopLeft(cursorLocation_);
}

void ConsoleWidget::setColor(quint8 color)
{
    currentColor_ = color;
}

void ConsoleWidget::moveVisibleToIncludeCursor()
{
    if (visibleRegion_.contains(cursorLocation_))
        return;

    if (cursorLocation_.y() < visibleRegion_.top())
        visibleRegion_.moveTop(cursorLocation_.y());
    else if (cursorLocation_.y() > visibleRegion_.bottom())
        visibleRegion_.moveBottom(cursorLocation_.y());

    if (cursorLocation_.x() < visibleRegion_.left())
        visibleRegion_.moveLeft(cursorLocation_.x());
    else if (cursorLocation_.x() > visibleRegion_.right())
        visibleRegion_.moveRight(cursorLocation_.x());
}

void ConsoleWidget::moveCursorUp(int count)
{
    cursorLocation_.setY(qMax(0, cursorLocation_.y() - count));
    moveVisibleToIncludeCursor();
    blink_ = false;
}

void ConsoleWidget::moveCursorDown(int count)
{
    while (count) {
        if (cursorLocation_.y() < bufferRegion_.height() - 1) {
            cursorLocation_.setY(cursorLocation_.y() + 1);
        } else {
            scrollUp();
        }
        count--;
    }
    moveVisibleToIncludeCursor();
    blink_ = false;
}

void ConsoleWidget::moveCursorLeft(int count)
{
    cursorLocation_.setX(qMax(0, cursorLocation_.x() - count));
    moveVisibleToIncludeCursor();
    blink_ = false;
}

void ConsoleWidget::moveCursorRight(int count)
{
    cursorLocation_.setX(qMin(cursorLocation_.x() + count, bufferRegion_.width()));
    moveVisibleToIncludeCursor();
    blink_ = false;
}

void ConsoleWidget::moveCursorStartOfLine()
{
    cursorLocation_.setX(0);
    moveVisibleToIncludeCursor();
}

quint8 ConsoleWidget::getCurrentColor() const
{
    if (!currentInverse_)
        return currentColor_;
    else
        return (currentColor_ >> 4) | (currentColor_ << 4);
}

void ConsoleWidget::scrollUp()
{
    int lastLineOffset = bufferRegion_.width() * (bufferRegion_.height() - 1);
    memmove(cells_, &cells_[bufferRegion_.width()], lastLineOffset * sizeof(Cell));
    quint8 color = getCurrentColor();
    for (int i = 0; i < bufferRegion_.width(); i++) {
        cells_[lastLineOffset + i].c = QLatin1Char(' ');
        cells_[lastLineOffset + i].color = color;
    }
    update();
}

void ConsoleWidget::print(const QString &text)
{
    for (int i = 0; i < text.length(); i++)
        putchar(text.at(i));
}

void ConsoleWidget::putchar(QChar c)
{
    int index = cursorLocation_.y() * bufferRegion_.width() + cursorLocation_.x();
    cells_[index].c = c;
    cells_[index].color = getCurrentColor();
    if (cursorLocation_.x() >= visibleRegion_.right()) {
        cursorLocation_.setX(0);
        moveCursorDown(1);
    } else {
        moveCursorRight(1);
    }
    update();
}

void ConsoleWidget::zoomIn()
{
    int newPointSize = fontSize() + 2;
    if (newPointSize < 64)
        setFontSize(newPointSize);
}

void ConsoleWidget::zoomOut()
{
    int newPointSize = fontSize() - 2;
    if (newPointSize > 6)
        setFontSize(newPointSize);
}

void ConsoleWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    int index = visibleRegion_.y() * bufferRegion_.width() + visibleRegion_.x();
    int y = 0;
    int x = 0;
    for (int i = 0; i < visibleRegion_.height(); i++) {
        x = 0;
        int bx = 0;
        quint8 background = cells_[index].color >> 4;
        for (int j = 0; j < visibleRegion_.width(); j++) {
            quint8 newBackground = cells_[index].color >> 4;
            if (background != newBackground) {
                p.fillRect(bx, y, x - bx, cellSize_.height(), backgroundColors_[background]);
                bx = x;
                background = newBackground;
            }
            index++;
            x += cellSize_.width();
        }
        p.fillRect(bx, y, x - bx, cellSize_.height(), backgroundColors_[background]);
        index += bufferRegion_.width() - visibleRegion_.width();
        y += cellSize_.height();
    }

    // Fill in the areas not covered by the console
    quint8 background = getCurrentColor() >> 4;
    QColor backgroundColor = backgroundColors_[background];
    int consoleWidthPixels = x;
    int consoleHeightPixels = y;

    p.fillRect(consoleWidthPixels, 0, width() - consoleWidthPixels, height(), backgroundColor);
    p.fillRect(0, consoleHeightPixels, consoleWidthPixels, height() - consoleHeightPixels, backgroundColor);

    // Draw all of the letters.
    p.setFont(font_);
    index = visibleRegion_.y() * bufferRegion_.width() + visibleRegion_.x();
    y = 0;
    x = 0;
    for (int i = 0; i < visibleRegion_.height(); i++) {
        x = 0;
        for (int j = 0; j < visibleRegion_.width(); j++) {
            QChar c = cells_[index].c;
            if (c != QLatin1Char(' ')) {
                p.setPen(foregroundColors_[cells_[index].color & 0x0f]);
                p.drawText(x,
                           y + baseline_,
                           QString(c));
            }
            index++;
            x += cellSize_.width();
        }
        index += bufferRegion_.width() - visibleRegion_.width();
        y += cellSize_.height();
    }


    // Draw the cursor
    if (!blink_ && visibleRegion_.contains(cursorLocation_)) {
        int cursorX = cursorLocation_.x() - visibleRegion_.x();
        int cursorY = cursorLocation_.y() - visibleRegion_.y();
        QRect r(cursorX * cellSize_.width(),
                cursorY * cellSize_.height() + baseline_,
                cellSize_.width(),
                cellSize_.height() - baseline_);
        p.fillRect(r, foregroundColors_[7]);
    }
}

void ConsoleWidget::timerEvent(QTimerEvent *)
{
    blink_ = !blink_;
    update();
}

void ConsoleWidget::handleTerminalResize()
{
    // Notify the dtach program of our new size
    int xpixel = size().width();
    int ypixel = size().height();
    int col = qMin(xpixel / cellSize_.width(), bufferRegion_.width());
    int row = qMin(ypixel / cellSize_.height(), bufferRegion_.height());

    client_->setWindowSize(col, row, xpixel, ypixel);

    // Grow/shrink the visible region
    visibleRegion_.setWidth(col);
    visibleRegion_.setHeight(row);

    // In case it shrank, move it to include the cursor
    moveVisibleToIncludeCursor();

    // In case the visible region moved out of the buffered area, move it back.
    if (visibleRegion_.bottom() > bufferRegion_.bottom())
        visibleRegion_.moveBottom(bufferRegion_.bottom());
    if (visibleRegion_.right() > bufferRegion_.right())
        visibleRegion_.moveRight(bufferRegion_.right());

    update();
}

void ConsoleWidget::resizeEvent(QResizeEvent *)
{
    handleTerminalResize();
}
