#ifndef PRESENTATIONWIDGET_H
#define PRESENTATIONWIDGET_H

#include <QWidget>

class SlideLoader;

class PresentationWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PresentationWidget(const QString &slidePath, QWidget *parent = 0);
    ~PresentationWidget();

    void setSlidePath(const QString &path);

public slots:
    void nextSlide();
    void previousSlide();

protected:
    void keyPressEvent(QKeyEvent *);
    void paintEvent(QPaintEvent *);
    void mouseMoveEvent(QMouseEvent *);

private slots:
    void slidesFound(int count);
    void slide(int index, QImage image);
    void noSlide(int index);
    void hideCursorTimeout();

private:
    void scaleImage(QPainter *painter, const QImage &img) const;
    QImage instructionSlide();
    QImage outOfSlidesSlide();
    void reinitialize();

private:
    QString slidePath_;

    int currentSlideIndex_;
    int startSlideIndex_;
    int slideCount_;
    QImage currentSlide_;

    SlideLoader *loader_;
    QThread *loaderThread_;
    QTimer *hideCursorTimer_;

    enum State {
        Slide,
        OutOfSlides
    };
    State state_;
    int slideCounter_;

    QImage instructionsSlide_;
    QImage outOfSlidesSlide_;
};

#endif // PRESENTATIONWIDGET_H
