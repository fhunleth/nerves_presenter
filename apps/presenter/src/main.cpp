#include <QApplication>

#include "PresentationWidget.h"
#include "ConsoleWidget.h"
#include "SwitcherWidget.h"
#include "KeyHandler.h"
#include "DtachClient.h"

#include <QLabel>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString slidePath = argc > 1 ? argv[1] : "/mnt";

    ConsoleWidget *c = new ConsoleWidget();
    PresentationWidget *mainWidget = new PresentationWidget(slidePath);

    SwitcherWidget switcher;
    switcher.setFullscreenWidget(mainWidget);
    switcher.setOverlayWidget(c);
    switcher.setMinimumSize(800, 600);
    switcher.show();

    KeyHandler helper(&switcher, 0);
    a.installEventFilter(&helper);

    // On EGL, it seems like we have to force focus
    switcher.forceFocus();

    switcher.toggle();

    return a.exec();
}
