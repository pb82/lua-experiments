TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt
SOURCES += main.cpp \
    config.cpp \
    nulldb.cpp \
    sandbox.cpp \
    asyncqueue.cpp \    
    pluginregistry.cpp \
    luatools.cpp


LIBS += -llua -luv

# unix:!macx: LIBS += -L$$PWD/../../../../usr/local/lib/ -lluajit-5.1
# INCLUDEPATH += $$PWD/../../../../usr/local/include/luajit-2.0
# DEPENDPATH += $$PWD/../../../../usr/local/include/luajit-2.0

HEADERS += \
    logger.h \
    config.h \
    persistence.h \
    nulldb.h \
    sandbox.h \
    asyncqueue.h \
    definitions.h \
    pluginregistry.h \
    json/Value.hpp \
    json/PrettyPrinter.hpp \
    json/Printer.hpp \
    plugin.h \
    luatools.h

OTHER_FILES += config.lua \
    libs/skeleton/skeleton.cpp

QMAKE_POST_LINK += $$quote(cp $${PWD}/*.lua $${OUT_PWD})
