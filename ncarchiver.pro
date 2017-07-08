TEMPLATE = app
CONFIG += console link_pkgconfig
CONFIG -= app_bundle qt
PKGCONFIG += ncursesw libarchive

DEFINES += HAVE_MMAP HAVE_LOCALE_H

INCLUDEPATH += $$PWD/formats $$PWD/ui

QMAKE_CFLAGS += -Wextra

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
    ui/global_ui.h \
    ui/util.h \
    ui/prompt.h \
    ui/actions.h \
    \
    formats/global.h \
    formats/filetree.h \
    formats/cli.h \
    formats/logging.h \
    formats/functions.h

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
    ui/util.c \
    ui/prompt.c \
    ui/nodeinfo.c \
    ui/actions.c \
    ui/compressdialog.c \
    ui/fselect.c \
    ui/prompt_list.c \
    \
    formats/libarchive.c \
    formats/filetree.c \
    formats/cli.c \
    formats/global.c \
    formats/cli_7z.c \
    formats/functions.c
