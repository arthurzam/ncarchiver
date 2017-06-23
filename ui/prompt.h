#ifndef INPUTTEXT_H
#define INPUTTEXT_H

char *prompt_text(const char *title, const char *prompt) __attribute__ ((__nonnull__));
int prompt_number(const char *title, const char *prompt) __attribute__ ((__nonnull__));

/*
 * for passing buttons without some input type we add prefix char
 *  \x1 - like default for buttons
 *  \x2 - no shortcut
 *  other - little letter for the shortcut
 */

int prompt_msgbox(const char *title, const char *msg, const char *const *buttons, int defaultBtn, int width) __attribute__ ((__nonnull__));
bool prompy_yesno(const char *title, const char *msg, int width) __attribute__ ((__nonnull__));
void prompy_ok(const char *title, const char *msg, int width) __attribute__ ((__nonnull__));


enum OverwriteFlags {
    OVERWRITE_AUTOSKIP = 0x1,
    OVERWRITE_OVERWRITE = 0x2
};

int prompt_overwrite(const char *filename, int *flags) __attribute__ ((__nonnull__));

#endif // INPUTTEXT_H
