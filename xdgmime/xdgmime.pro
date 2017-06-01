#-------------------------------------------------
#
# Project created by QtCreator 2017-05-30T19:21:36
#
#-------------------------------------------------

QT       -= core gui
TARGET = xdgmime
TEMPLATE = lib
CONFIG += staticlib

DEFINES += HAVE_MMAP

QMAKE_CFLAGS += -fvisibility=hidden

SOURCES += \
    xdgmime.c \
    xdgmimealias.c \
    xdgmimecache.c \
    xdgmimeglob.c \
    xdgmimeicon.c \
    xdgmimeint.c \
    xdgmimemagic.c \
    xdgmimeparent.c

HEADERS += \
    xdgmime.h \
    xdgmimealias.h \
    xdgmimecache.h \
    xdgmimeglob.h \
    xdgmimeicon.h \
    xdgmimeint.h \
    xdgmimemagic.h \
    xdgmimeparent.h
