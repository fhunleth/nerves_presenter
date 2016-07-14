#include "SwitcherWidget.h"
#include <QResizeEvent>
#include <QKeyEvent>

SwitcherWidget::SwitcherWidget(QWidget *parent) :
    QWidget(parent),
    fullscreen_(0),
    overlay_(0),
    overlayActive_(false)
{
}

void SwitcherWidget::setFullscreenWidget(QWidget *w)
{
    fullscreen_ = w;

    w->setParent(this);
    w->setGeometry(0, 0, size().width(), size().height());
    w->setVisible(true);
    w->setFocus();
}

void SwitcherWidget::setOverlayWidget(QWidget *overlay)
{
    overlay_ = overlay;
    overlay->setParent(this);

    int w = size().width();
    int h = size().height();

    overlay->setGeometry(0, h * 3 / 10, w, h * 7 / 10);
    overlay->setVisible(false);
}

void SwitcherWidget::toggle()
{
    overlayActive_ = !overlayActive_;
    forceFocus();
}

void SwitcherWidget::forceFocus()
{
    if (!overlay_ || !fullscreen_)
        return;

    if (overlayActive_) {
        overlay_->setVisible(true);
        overlay_->raise();
        overlay_->setFocus();
    } else {
        overlay_->setVisible(false);
        fullscreen_->setFocus();
    }
}

void SwitcherWidget::resizeEvent(QResizeEvent *e)
{
    int w = e->size().width();
    int h = e->size().height();

    if (fullscreen_)
        fullscreen_->setGeometry(0, 0, w, h);

    if (overlay_)
        overlay_->setGeometry(0, h * 3 / 10, w, h * 7 / 10);
}
