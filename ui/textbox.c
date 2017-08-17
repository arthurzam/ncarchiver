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
    } else if (textbox->len == index + 1) {
        textbox->str = (char *)realloc(textbox->str, textbox->len);
        textbox->str[--textbox->len] = '\0';
    } else {
        for (uint16_t i = index; i < textbox->len; ++i)
            textbox->str[i] = textbox->str[i + 1];
        NC_ASSERT_VAL(textbox->str[textbox->len], '\0');
        --textbox->len;
    }
}

int textbox_key(struct textbox_data_t *textbox, int key) {
    NC_ASSERT_NONNULL(textbox);
    switch (key) {
        case KEY_BACKSPACE:
            if (textbox->start + textbox->cursor == 0)
                return 1;
            _textbox_delChar(textbox, textbox->start + textbox->cursor - 1);
            // fallthrough
        case KEY_LEFT:
            if (textbox->cursor != 0)
                --textbox->cursor;
            else if (textbox->start != 0)
                --textbox->start;
            return 1;
        case KEY_DC:
        case KEY_DELETE:
            if (textbox->cursor + textbox->start != textbox->len)
                _textbox_delChar(textbox, textbox->start + textbox->cursor);
            return 1;
        case KEY_RIGHT:
            if (textbox->cursor + textbox->start != textbox->len) {
                if (textbox->cursor != textbox->width - 1)
                    ++textbox->cursor;
                else
                    ++textbox->start;
            }
            return 1;
        case KEY_CTRL('a'):
        case KEY_HOME:
            textbox->cursor = 0;
            textbox->start = 0;
            return 1;
        case KEY_CTRL('e'):
        case KEY_END:
            if (textbox->len + 1 > textbox->width) {
                textbox->cursor = textbox->width - 1;
                textbox->start = textbox->len - textbox->width + 1;
            } else {
                textbox->start = 0;
                textbox->cursor = textbox->len;
            }
            return 1;
        case KEY_CTRL('u'):
            textbox->cursor = textbox->start = textbox->len = 0;
            textbox->str = (free(textbox->str), NULL);
            return 1;
    }
    if (textbox->check_func(key)) {
        textbox->str = realloc(textbox->str, textbox->len + 2);
        if (textbox->cursor + textbox->start == textbox->len) {
            textbox->str[textbox->len] = key;
            textbox->str[++textbox->len] = '\0';

        } else {
            for (uint16_t i = textbox->len; i > textbox->cursor + textbox->start; --i)
                textbox->str[i] = textbox->str[i - 1];
            textbox->str[textbox->cursor + textbox->start] = key;
            textbox->str[++textbox->len] = '\0';
        }
        if (textbox->cursor != textbox->width - 1)
            ++textbox->cursor;
        else
            ++textbox->start;
        return 1;
    }
    return 0;
}

void textbox_draw(const struct textbox_data_t *textbox, uint16_t r, uint16_t c, int flags) {
    NC_ASSERT_NONNULL(textbox);
    NC_ASSERT(textbox->cursor < textbox->width, "check textbox's width changed");
    NC_ASSERT(textbox->len == 0 || textbox->start + textbox->cursor <= textbox->len, "error: start + cursor > len");

    const char *str = textbox->str + textbox->start;
    move(subwinr + r, subwinc + c);
    uint16_t len = min(textbox->width, textbox->len - textbox->start);
    const bool isEndInsert = (textbox->cursor + textbox->start == textbox->len);

    attron(A_UNDERLINE);
    if (!str);
    else if (flags & TEXTBOX_HIDE) {
        if (flags & TEXTBOX_SELECTED) {
            unsigned i;
            for (i = 0; i < textbox->cursor; ++i)
                addch('*');

            if (i != len) {
                attron(A_REVERSE);
                addch('*');
                attroff(A_REVERSE);

                for (++i; i < len; ++i)
                    addch('*');
            }
        } else {
            hline('*', len);
            move(subwinr + r, subwinc + c + len);
        }
    } else if ((flags & TEXTBOX_SELECTED) && !isEndInsert) {
        addnstr(str, textbox->cursor);
        str += textbox->cursor;

        attron(A_REVERSE);
        addch(*str);
        ++str;
        attroff(A_REVERSE);

        addnstr(str, len - textbox->cursor - 1);
    } else {
        addnstr(str, len);
    }

    if (isEndInsert && (flags & TEXTBOX_SELECTED)) {
        attron(A_REVERSE);
        addch(' ');
        attroff(A_REVERSE);
        ++len;
    }
    hline(' ', textbox->width - len);

    attroff(A_UNDERLINE);
}

void textbox_init(struct textbox_data_t *textbox, const char *def, int (*check_func)(int), uint16_t width) {
    NC_ASSERT_NONNULL(textbox);
    NC_ASSERT_NONNULL(check_func);
    NC_ASSERT(!def || textbox->str != def, "strings shouldn't be set already");
    if (textbox->str)
        free(textbox->str);
    *textbox = (struct textbox_data_t) {
        .str = def ? strdup(def) : NULL, .check_func = check_func, .len = def ? strlen(def) : 0,
        .start = 0, .cursor = 0, .width = width
    };
}
