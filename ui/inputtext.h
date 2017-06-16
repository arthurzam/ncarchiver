#ifndef INPUTTEXT_H
#define INPUTTEXT_H

char *prompt_text(const char *title, const char *prompt);
int prompt_number(const char *title, const char *prompt);
int prompt_msgbox(const char *title, const char *msg, const char *const *buttons, int defaultBtn, int width);

#endif // INPUTTEXT_H
