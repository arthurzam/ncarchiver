QT       -= core gui
TARGET = formats
TEMPLATE = lib
CONFIG += staticlib

PKGCONFIG += libarchive

HEADERS += \
    global.h \
    filetree.h \
    cli.h

SOURCES += \
    libarchive.c \
    filetree.c \
    cli.c \
    global.c \
    cli_7z.c
