TEMPLATE = app
CONFIG += console link_pkgconfig
CONFIG -= app_bundle qt
PKGCONFIG += ncursesw libarchive

HEADERS += \
    browser.h \
    global_ui.h \
    quit.h \
    util.h

SOURCES += \
    browser.c \
    main_ui.c \
    quit.c \
    util.c

unix: LIBS += -L$$OUT_PWD/../xdgmime/ -lxdgmime -L$$OUT_PWD/../formats/ -lformats

INCLUDEPATH += $$PWD/../xdgmime $$PWD/../formats
DEPENDPATH += $$PWD/../xdgmime $$PWD/../formats

unix: PRE_TARGETDEPS += $$OUT_PWD/../xdgmime/libxdgmime.a $$OUT_PWD/../formats/libformats.a
