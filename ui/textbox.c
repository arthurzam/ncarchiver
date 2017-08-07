#include "global.h"
#include "global_ui.h"
#include "filetree.h"
#include "actions.h"
#include "functions.h"
#include "textbox.h"

#include <string.h>
#include <stdlib.h>

#include <ncurses.h>

static void _textbox_delChar(struct textbox_data_t *textbox, uint16_t index) {
    if (!textbox->str) return;
    if (textbox->len == 1) {
        textbox->str = (free(textbox->str), NULL);
        textbox->cursor = textbox->start = 0;
        return;
    }
    if (textbox->len == index + 1) {
        textbox->str = (char *)realloc(textbox->str, textbox->len);
        textbox->str[--textbox->len] = '\0';
        // TODO: change cursor
        return;
    }
    for (uint16_t i = index; i < textbox->len; ++i)
        textbox->str[i] = textbox->str[i + 1];
    NC_ASSERT_VAL(textbox->str[textbox->len], '\0');
    --textbox->len;
    // TODO: change cursor
}

int textbox_key(struct textbox_data_t *textbox, int key) {
    switch (key) {
        case KEY_LEFT:
            if (textbox->cursor != 0)
                --textbox->cursor;
            else if (textbox->start != 0)
                --textbox->start;
            return 1;
        case KEY_RIGHT:
            if (textbox->cursor + textbox->start != textbox->len - 1) {
                if (textbox->cursor != textbox->width - 1)
                    ++textbox->cursor;
                else
                    ++textbox->start;
            }
            return 1;
        case KEY_BACKSPACE:
            _textbox_delChar(textbox, textbox->start + textbox->cursor - 1);
            return 1;
        case KEY_DELETE:
            _textbox_delChar(textbox, textbox->start + textbox->cursor);
            return 1;
        case KEY_HOME:
            textbox->cursor = 0;
            textbox->start = 0;
            return 1;
        case KEY_END:
            if (textbox->len > textbox->width) {
                textbox->cursor = textbox->width - 1;
                textbox->start = textbox->len - textbox->width;
            } else {
                textbox->start = 0;
                textbox->cursor = textbox->len - 1;
            }
            return 1;
        case KEY_CTRL('u'):
            textbox->cursor = textbox->start = textbox->len = 0;
            textbox->str = (free(textbox->str), NULL);
            return 1;
    }
    if (textbox->check_func(key)) {
        textbox->str = realloc(textbox->str, textbox->len + 2);
        textbox->str[textbox->len] = key;
        textbox->str[textbox->len + 1] = '\0';
        return 1;
    }
    return 0;
}

void textbox_draw(const struct textbox_data_t *textbox, uint16_t r, uint16_t c, int flags) {
    NC_ASSERT(textbox->cursor < textbox->width, "check textbox's width changed");
    NC_ASSERT(textbox->start + textbox->cursor < textbox->len, "error: start + cursor >= len");

    const char *str = textbox->str + textbox->start;
    move(subwinr + r, subwinc + c);

    attron(A_UNDERLINE);
    if (flags & TEXTBOX_HIDE) {
        const uint16_t len = min(textbox->width, textbox->len - textbox->start);
        for (unsigned i = 0; i < len; ++i) {
            if ((flags & TEXTBOX_SELECTED) && (i == textbox->cursor))
                attron(A_REVERSE);
            addch('*');
            if ((flags & TEXTBOX_SELECTED) && (i == textbox->cursor))
                attron(A_REVERSE);
        }
    } else if (flags & TEXTBOX_SELECTED) {
        addnstr(str, textbox->cursor);
        str += textbox->cursor;

        attron(A_REVERSE);
        addch(*str);
        attroff(A_REVERSE);
        ++str;

        addnstr(str, min(textbox->len - textbox->cursor - textbox->start, textbox->width - textbox->cursor) - 1);
    } else {
        addnstr(str, min(textbox->width, textbox->len - textbox->start));
    }

    attroff(A_UNDERLINE);
}

void textbox_init(struct textbox_data_t *textbox, const char *def, int (*check_func)(int), uint16_t width) {
    NC_ASSERT(textbox->str != def, "strings shouldn't be set already");
    if (textbox->str)
        free(textbox->str);
    *textbox = (struct textbox_data_t) {
        .str = strdup(def), .check_func = check_func, .len = strlen(def),
        .start = 0, .cursor = 0, .width = width
    };
}
