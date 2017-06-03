TEMPLATE = app
CONFIG += console link_pkgconfig
CONFIG -= app_bundle qt
PKGCONFIG += ncursesw libarchive

DEFINES += HAVE_MMAP

INCLUDEPATH += $$PWD/formats

HEADERS += \
    xdgmime/xdgmime.h \
    xdgmime/xdgmimealias.h \
    xdgmime/xdgmimecache.h \
    xdgmime/xdgmimeglob.h \
    xdgmime/xdgmimeicon.h \
    xdgmime/xdgmimeint.h \
    xdgmime/xdgmimemagic.h \
    xdgmime/xdgmimeparent.h \
    \
    ui/browser.h \
    ui/global_ui.h \
    ui/quit.h \
    ui/util.h \
    ui/inputtext.h \
    \
    formats/global.h \
    formats/filetree.h \
    formats/cli.h

SOURCES += \
    xdgmime/xdgmime.c \
    xdgmime/xdgmimealias.c \
    xdgmime/xdgmimecache.c \
    xdgmime/xdgmimeglob.c \
    xdgmime/xdgmimeicon.c \
    xdgmime/xdgmimeint.c \
    xdgmime/xdgmimemagic.c \
    xdgmime/xdgmimeparent.c \
    \
    ui/browser.c \
    ui/main_ui.c \
    ui/quit.c \
    ui/util.c \
    ui/inputtext.c \
    \
    formats/libarchive.c \
    formats/filetree.c \
    formats/cli.c \
    formats/global.c \
    formats/cli_7z.c
