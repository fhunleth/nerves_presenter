#ifndef SWITCHERWIDGET_H
#define SWITCHERWIDGET_H

#include <QWidget>

class SwitcherWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SwitcherWidget(QWidget *parent = 0);

    void setFullscreenWidget(QWidget *w);
    void setOverlayWidget(QWidget *overlay);

    void toggle();
    void forceFocus();

protected:
    void resizeEvent(QResizeEvent *);

private:
    QWidget *fullscreen_;
    QWidget *overlay_;

    bool overlayActive_;
};

#endif // SWITCHERWIDGET_H
