QT       += core gui widgets network

TARGET = console
TEMPLATE = app

SOURCES += main.cpp\
    Erlcmd.cpp \
    ConsoleWidget.cpp \
    SwitcherWidget.cpp \
    KeyHandler.cpp \
    DtachClient.cpp \
    ErlWebView.cpp \
    PresentationWidget.cpp \
    SlideLoader.cpp

HEADERS  += \
    Erlcmd.h \
    ConsoleWidget.h \
    SwitcherWidget.h \
    KeyHandler.h \
    DtachClient.h \
    ErlWebView.h \
    PresentationWidget.h \
    SlideLoader.h

FORMS    +=

#LIBS += -lerl_interface -lei

RESOURCES += \
    assets.qrc
