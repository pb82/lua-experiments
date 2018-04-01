TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt
SOURCES += main.cpp \
    config.cpp \
    lobj.cpp \
    nulldb.cpp

LIBS += -llua5.3-c++

# unix:!macx: LIBS += -L$$PWD/../../../../usr/local/lib/ -lluajit-5.1
# INCLUDEPATH += $$PWD/../../../../usr/local/include/luajit-2.0
# DEPENDPATH += $$PWD/../../../../usr/local/include/luajit-2.0

HEADERS += \
    logger.h \
    config.h \
    lobj.h \
    persistence.h \
    nulldb.h

OTHER_FILES += config.lua

QMAKE_POST_LINK += $$quote(cp $${PWD}/*.lua $${OUT_PWD})
