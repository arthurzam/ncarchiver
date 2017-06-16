#ifndef INPUTTEXT_H
#define INPUTTEXT_H

char *prompt_text(const char *title, const char *prompt);
int prompt_number(const char *title, const char *prompt);

/*
 * for passing buttons without some input type we add prefix char
 *  \x1 - like default for buttons
 *  \x2 - no shortcut
 *  other - little letter for the shortcut
 */

int prompt_msgbox(const char *title, const char *msg, const char *const *buttons, int defaultBtn, int width);
bool prompy_yesno(const char *title, const char *msg, int width);

#endif // INPUTTEXT_H
