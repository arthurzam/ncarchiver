#ifndef TEXTBOX_H
#define TEXTBOX_H

#include <stdint.h>

struct textbox_data_t {
    char *str;
    int (*check_func)(int);

    uint16_t len, start, cursor, width;
};

void textbox_init(struct textbox_data_t *textbox, const char *def, int (*check_func)(int), uint16_t width);
int  textbox_key (struct textbox_data_t *textbox, int key);
void textbox_draw(const struct textbox_data_t *textbox, uint16_t r, uint16_t c, int flags);

enum TextboxFlags {
    TEXTBOX_SELECTED = 0x1,
    TEXTBOX_HIDE = 0x2
};

#endif // TEXTBOX_H
