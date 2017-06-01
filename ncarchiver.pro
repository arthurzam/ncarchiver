TEMPLATE = subdirs

SUBDIRS += \
    xdgmime \
    formats \
    ui

ui.depends = xdgmime formats
