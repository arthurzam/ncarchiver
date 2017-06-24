#include "global.h"
#include "global_ui.h"
#include "filetree.h"

#include <string.h>
#include <stdlib.h>

#include <ncurses.h>
#include <ctype.h>

struct compressdialog_t {
    struct compression_options_t options;
    int selected_row;
};

static int compressdialog_key(int index, int key) {
    struct compressdialog_t *data = (struct compressdialog_t *)ui_data[index];
    switch (key) {
        case '\t':
        case KEY_DOWN:
            if ((++data->selected_row) == 7)
                data->selected_row = 0;
            return 0;
        case KEY_UP:
            if ((data->selected_row--) == 0)
                data->selected_row = 6;
            return 0;
        case 10: // Enter
        case KEY_ENTER:
            return (data->selected_row >= 5 ? 2 : 0);
        case ' ':
            if (data->selected_row == 4)
            {
                data->options.encryptHeaders = !data->options.encryptHeaders;
                return 0;
            }
            break;
    }
    char **selectedTB = (data->selected_row == 0 ? &(data->options.location) :
                         data->selected_row == 1 ? &(data->options.filename) :
                         data->selected_row == 3 ? &(data->options.password) :
                         NULL);
    if (!selectedTB)
        return 0;
    size_t i;
    if (key == KEY_BACKSPACE || key == 0x7f)
    {
        i = *selectedTB ? strlen(*selectedTB) : 0;
        if (i > 0)
        {
            *selectedTB = realloc(*selectedTB, i - 1);
            if (*selectedTB)
                (*selectedTB)[i - 1] = '\0';
        }
    }
    else if (isprint(key))
    {
        i = *selectedTB ? strlen(*selectedTB) : 0;
        *selectedTB = realloc(*selectedTB, i + 2);
        (*selectedTB)[i] = key;
        (*selectedTB)[i + 1] = '\0';
    }
    return 0;
}

#define WINDOW_WIDTH 60

static void _compressdialog_draw_textbox(int row, int col, const char *text, bool hide)
{
    const unsigned maxLen = WINDOW_WIDTH - 2 - col;
    unsigned i;
    i = text ? strlen(text) : 0;

    if (hide)
    {
        i = i > maxLen ? maxLen : i;
        mvhline(subwinr + row, subwinc + col, '*', i);
        mvhline(subwinr + row, subwinc + col + i, ' ', maxLen - i);
        return;
    }
    i = i > maxLen ? i - maxLen : 0;
    ncprint(row, col, "%-*s", maxLen, (text ?: "") + i);
}

static void _compressdialog_draw_label(int row, int col, const char *text, bool flag)
{
    if (flag)
        attron(A_REVERSE);
    ncaddstr(row, col, text);
    if (flag)
        attroff(A_REVERSE);
}

static void compressdialog_draw(int index) {
    struct compressdialog_t *data = (struct compressdialog_t *)ui_data[index];

    nccreate(10, WINDOW_WIDTH, "Create New Archive");

    _compressdialog_draw_label(2, 2, "Location:", data->selected_row == 0);
    _compressdialog_draw_label(3, 2, "Filename:", data->selected_row == 1);
    _compressdialog_draw_label(4, 2, "Format:"  , data->selected_row == 2);
    _compressdialog_draw_label(5, 2, "Password:", data->selected_row == 3);

    attron(A_UNDERLINE);
    _compressdialog_draw_textbox(2, 12, data->options.location, false);
    _compressdialog_draw_textbox(3, 12, data->options.filename, false);
    // _compressdialog_draw_textbox(2, 10, data->options.location, false);
    _compressdialog_draw_textbox(5, 12, data->options.password, true);
    attroff(A_UNDERLINE);

    if (data->selected_row == 4)
        attron(A_REVERSE);
    ncprint (6, 2, "[%c] Encrypt Headers", data->options.encryptHeaders ? 'X' : ' ');
    if (data->selected_row == 4)
        attroff(A_REVERSE);

    _compressdialog_draw_label(8, WINDOW_WIDTH / 4 - 1,       "OK"    , data->selected_row == 5);
    _compressdialog_draw_label(8, (3 * WINDOW_WIDTH / 4) - 3, "Cancel", data->selected_row == 6);
}

void compressdialog_init() {
    struct compressdialog_t *data = TYPE_MALLOC(struct compressdialog_t);
    memset(data, 0, sizeof(struct compressdialog_t));
    data->options.password = strdup("123");
    ui_insert(compressdialog_draw, compressdialog_key, data);
}
