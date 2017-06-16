#include "global.h"
#include "global_ui.h"
#include "filetree.h"

#include <string.h>
#include <stdlib.h>

#include <ncurses.h>
#include <ctype.h>

struct prompt_t {
    const char *title;
    const char *prompt;
    char *res;
    const char *const *buttons;
    int (*check_func)(int);
    unsigned width;
    unsigned selected_button;
    unsigned cursor;
};

static int prompt_key(int index, int key) {
    struct prompt_t *data = (struct prompt_t *)ui_data[index];
    size_t i;
    switch(key) {
        case 9: // TAB
            if (data->buttons[++data->selected_button] == NULL)
                data->selected_button = 0;
            return 0;
        case 10: // Enter
        case KEY_ENTER:
            return 1;
        case KEY_BACKSPACE:
        case 0x7f: // DEL
            i = data->res ? strlen(data->res) : 0;
            if (i > 0)
            {
                data->res = realloc(data->res, i - 1);
                if (data->res)
                    data->res[i - 1] = '\0';
            }
            return 0;
    }
    if (!data->check_func)
    {
        if (isalpha(key))
            for (i = 0; data->buttons[i] != NULL; ++i)
                if (tolower(key) == data->buttons[i][0])
                {
                    data->selected_button = i;
                    return 1;
                }
        for (i = 0; data->buttons[i] != NULL; ++i)
            if (data->buttons[i][0] == '\x1')
            {
                data->selected_button = i;
                return 1;
            }
    }
    else if (data->check_func(key))
    {
        i = data->res ? strlen(data->res) : 0;
        data->res = realloc(data->res, i + 2);
        data->res[i] = key;
        data->res[i + 1] = '\0';
    }
    return 0;
}

static void prompt_draw(int index) {
    struct prompt_t *data = (struct prompt_t *)ui_data[index];
    unsigned col = 2, i;

    nccreate(data->check_func ? 8 : 7, data->width, data->title);

    ncaddstr(2,2, data->prompt);
    if (data->check_func)
    {
        attron(A_REVERSE);
        i = strlen(data->res ?: "");
        i = i > data->width - 4 ? i - data->width + 4 : 0;
        ncprint(3,2,"%-*s", data->width - 4, (data->res ?: "") + i);
        attroff(A_REVERSE);
    }

    for (i = 0; data->buttons[i] != NULL; ++i)
    {
        if (i == data->selected_button)
            attron(A_REVERSE);
        if (data->check_func)
            ncaddstr(5, col, data->buttons[i]);
        else
            ncaddstr(4, col, data->buttons[i] + 1);
        if (i == data->selected_button)
            attroff(A_REVERSE);
        col += 3 + strlen(data->buttons[i]);
    }
}

static void prompt_init(struct prompt_t *data) {

    ui_insert(prompt_draw, prompt_key, data);
    while (input_handle(0) != 1);
    ui_remove();
}

static const char *const prompt_text_btns[] = {
    "OK",
    "Cancel",
    NULL
};

char *prompt_text(const char *title, const char *prompt)
{
    struct prompt_t data = {
        .title = title, .prompt = prompt, .res = NULL, .buttons = prompt_text_btns,
        .check_func = isprint, .width = 30, .selected_button = 0, .cursor = 0
    };
    prompt_init(&data);
    if (data.selected_button == 1) // Cancel
        free(data.res);
    return data.selected_button == 0 ? data.res : NULL;
}

int prompt_number(const char *title, const char *prompt)
{
    struct prompt_t data = {
        .title = title, .prompt = prompt, .res = NULL, .buttons = prompt_text_btns,
        .check_func = isdigit, .width = 30, .selected_button = 0, .cursor = 0
    };
    prompt_init(&data);
    if (!data.res) return -1;
    int res = data.selected_button == 0 ? atoi(data.res) : -1;
    free(data.res);
    return res;
}

int prompt_msgbox(const char *title, const char *msg, const char *const *buttons, int defaultBtn, int width)
{
    struct prompt_t data = {
        .title = title, .prompt = msg, .res = NULL, .buttons = buttons,
        .check_func = NULL, .width = width, .selected_button = defaultBtn, .cursor = 0
    };
    prompt_init(&data);
    free(data.res);
    return data.selected_button;
}

static const char *const prompy_yesno_btns[] = {
    "y""(Y)es",
    "\x1""(N)o",
    NULL
};

bool prompy_yesno(const char *title, const char *msg, int width)
{
    struct prompt_t data = {
        .title = title, .prompt = msg, .res = NULL, .buttons = prompy_yesno_btns,
        .check_func = NULL, .width = width, .selected_button = 0, .cursor = 0
    };
    prompt_init(&data);
    free(data.res);
    return data.selected_button == 0;
}
