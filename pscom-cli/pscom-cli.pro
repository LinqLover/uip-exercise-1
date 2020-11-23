TEMPLATE = app

QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    source/main.cpp \
    source/verbosity.cpp


DESTDIR = $$PWD/bin
CONFIG(debug, debug|release) {
    TARGET = $$join(TARGET,,,d)
}


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../pscom/lib/ -lpscom
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../pscom/lib/ -lpscomd
else:unix:CONFIG(release, debug|release): LIBS += -L$$PWD/../pscom/lib/ -lpscom
else:unix:CONFIG(debug, debug|release): LIBS += -L$$PWD/../pscom/lib/ -lpscomd

INCLUDEPATH += $$PWD/../pscom/include
DEPENDPATH += $$PWD/../pscom/include

HEADERS += \
    source/verbosity.h
