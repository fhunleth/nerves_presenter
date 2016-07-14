#include "KeyHandler.h"
#include "SwitcherWidget.h"

#include <QKeyEvent>
#include <QLabel>

KeyHandler::KeyHandler(SwitcherWidget *switcher,
                       QLabel *webView,
                       QObject *parent) :
    QObject(parent),
    switcher_(switcher),
    webView_(webView)
{
}

bool KeyHandler::eventFilter(QObject *, QEvent *e)
{
    if (e->type() == QEvent::KeyPress || e->type() == QEvent::KeyRelease) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(e);
        switch (keyEvent->key()) {
        case Qt::Key_F1:
            // Toggle active widget on key press; just consume key releases
            if (e->type() == QEvent::KeyPress)
                switcher_->toggle();

            return true;
        default:
            break;
        }
    }
    return false;
}
