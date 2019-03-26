#include "global.h"
#include "global_ui.h"
#include "filetree.h"
#include "textbox.h"

#include <string.h>
#include <stdlib.h>

#include <ncurses.h>
#include <ctype.h>

struct prompt_t {
    struct textbox_data_t textbox;
    const char *const title;
    const char *const prompt;
    const char *const *const buttons;
    unsigned selected_button;
};

static int prompt_key(int index, int key) {
    struct prompt_t *data = (struct prompt_t *)ui_data[index];
    if (data->textbox.check_func && textbox_key(&data->textbox, key))
        return 0;

    switch(key) {
        case KEY_TAB:
            if (data->buttons[++data->selected_button] == NULL)
                data->selected_button = 0;
            return 0;
        case KEY_RETURN:
        case KEY_ENTER:
            return 1;
    }
    size_t i;
    if (!data->textbox.check_func) {
        if (isalpha(key))
            for (i = 0; data->buttons[i] != NULL; ++i)
                if (tolower(key) == data->buttons[i][0]) {
                    data->selected_button = i;
                    return 1;
                }
        for (i = 0; data->buttons[i] != NULL; ++i)
            if (data->buttons[i][0] == '\x1') {
                data->selected_button = i;
                return 1;
            }
    }
    return 0;
}

static void prompt_draw(int index) {
    struct prompt_t *data = (struct prompt_t *)ui_data[index];
    unsigned col = 2, i;

    nccreate(data->textbox.check_func ? 7 : 6, data->textbox.width + 4, data->title);

    ncaddstr(2,2, data->prompt);
    if (data->textbox.check_func)
        textbox_draw(&data->textbox, 3, 2, TEXTBOX_SELECTED);

    for (i = 0; data->buttons[i] != NULL; ++i) {
        if (i == data->selected_button)
            attron(A_REVERSE);
        if (data->textbox.check_func)
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

char *prompt_text(const char *title, const char *prompt) {
    struct prompt_t data = {
        .textbox = TEXTBOX_INIT_EMPTY(isprint, 26), .title = title,
        .prompt = prompt, .buttons = prompt_text_btns, .selected_button = 0
    };
    prompt_init(&data);
    if (data.selected_button == 1) // Cancel
        free(data.textbox.str);
    return data.selected_button == 0 ? data.textbox.str : NULL;
}

int prompt_number(const char *title, const char *prompt) {
    struct prompt_t data = {
        .textbox = TEXTBOX_INIT_EMPTY(isdigit, 26), .title = title,
        .prompt = prompt, .buttons = prompt_text_btns, .selected_button = 0
    };
    prompt_init(&data);
    if (!data.textbox.str) return -1;
    int res = data.selected_button == 0 ? atoi(data.textbox.str) : -1;
    free(data.textbox.str);
    return res;
}

int prompt_msgbox(const char *title, const char *msg, const char *const *buttons, int defaultBtn, int width) {
    struct prompt_t data = {
        .textbox = TEXTBOX_INIT_EMPTY(NULL, width - 4), .title = title,
        .prompt = msg, .buttons = buttons, .selected_button = defaultBtn
    };
    prompt_init(&data);
    NC_ASSERT_VAL(data.textbox.str, NULL);
    return data.selected_button;
}

static const char *const prompt_yesno_btns[] = {
    "y"  "(Y)es",
    "\x1""(N)o",
    NULL
};

bool prompt_yesno(const char *title, const char *msg, int width) {
    struct prompt_t data = {
        .textbox = TEXTBOX_INIT_EMPTY(NULL, width - 4), .title = title,
        .prompt = msg, .buttons = prompt_yesno_btns, .selected_button = 0
    };
    prompt_init(&data);
    NC_ASSERT_VAL(data.textbox.str, NULL);
    return data.selected_button == 0;
}

static const char *const prompt_ok_btns[] = {
    "\x1""OK",
    NULL
};

void prompt_ok(const char *title, const char *msg, int width) {
    struct prompt_t data = {
        .textbox = TEXTBOX_INIT_EMPTY(NULL, width - 4), .title = title,
        .prompt = msg, .buttons = prompt_ok_btns, .selected_button = 0
    };
    prompt_init(&data);
    NC_ASSERT_VAL(data.textbox.str, NULL);
}

static const char *prompt_overwrite_btns[] = {
    "o"  "(O)verwrite",
    "s"  "(S)kip",
    "\x2""Overwrite All",
    "\x2""Autoskip",
    "c"  "(C)ancel",
    NULL
};

int prompt_overwrite(const char *filename, int *flags) {
    if (*flags & OVERWRITE_AUTOSKIP)
        return 3;
    if (*flags & OVERWRITE_OVERWRITE)
        return 2;
    int answer = prompt_msgbox("File Exists", filename, prompt_overwrite_btns, 1, 66);
    if (answer == 2)
        *flags |= OVERWRITE_OVERWRITE;
    else if (answer == 3)
        *flags |= OVERWRITE_AUTOSKIP;
    return answer;
}
