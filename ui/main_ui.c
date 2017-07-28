#include "global_ui.h"
#include "filetree.h"
#include "cli.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <sys/time.h>

#include <xdgmime/xdgmime.h>



// int read_only = 0;
long update_delay = 100;
// int cachedir_tags = 0;

static int min_rows = 17, min_cols = 60;
static int ncurses_init = 0;
static int ncurses_tty = 0; /* Explicitely open /dev/tty instead of using stdio */
static long lastupdate = 999;


static void screen_draw() {
    int i;
    for (i = 0; ui_draw_funcs[i] != NULL; ++i)
        ui_draw_funcs[i](i);
}


/* wait:
 *  -1: non-blocking, always draw screen
 *   0: blocking wait for input and always draw screen
 *   1: non-blocking, draw screen only if a configured delay has passed or after keypress
 */
int input_handle(int wait)
{
    int ch;
    int i;
    struct timeval tv;

    if (wait != 1)
        screen_draw();
    else {
        gettimeofday(&tv, (void *)NULL);
        tv.tv_usec = (1000 * (tv.tv_sec % 1000) + (tv.tv_usec / 1000)) / update_delay;
        if (lastupdate != tv.tv_usec) {
            screen_draw();
            lastupdate = tv.tv_usec;
        }
    }

    /* No actual input handling is done if ncurses hasn't been initialized yet. */
    if (!ncurses_init)
        return !wait;

    nodelay(stdscr, !!wait);
    while ((ch = getch()) != ERR) {
        if (ch == KEY_RESIZE) {
            if(ncresize(min_rows, min_cols))
                min_rows = min_cols = 0;
            /* ncresize() may change nodelay state, make sure to revert it. */
            nodelay(stdscr, wait?1:0);
            screen_draw();
            continue;
        }
        for (i = 0; ui_key_funcs[i] != NULL; ++i);
        i = ui_key_funcs[i - 1](i - 1, ch);
        if (i == 2)
            free(ui_remove());
        return i;
    }
    return 0;
}

/* parse command line */
static void init_nc()
{
    int ok = 0;
    FILE *tty;
    SCREEN *term;

    if(ncurses_init)
        return;
    ncurses_init = 1;

    if(ncurses_tty) {
        tty = fopen("/dev/tty", "r+");
        if(!tty) {
            fprintf(stderr, "Error opening /dev/tty: %s\n", strerror(errno));
            exit(1);
        }
        term = newterm(NULL, tty, tty);
        if(term)
            set_term(term);
        ok = !!term;
    } else {
        /* Make sure the user doesn't accidentally pipe in data to ncdu's standard
         * input without using "-f -". An annoying input sequence could result in
         * the deletion of your files, which we want to prevent at all costs. */
        if(!isatty(0)) {
            fprintf(stderr, "Standard input is not a TTY. Did you mean to import a file using '-f -'?\n");
            exit(1);
        }
        ok = !!initscr();
    }

    if(!ok) {
        fprintf(stderr, "Error while initializing ncurses.\n");
        exit(1);
    }

    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    if(ncresize(min_rows, min_cols))
        min_rows = min_cols = 0;
}

struct archive_t *arc;

static const struct format_t *findFormat(const char *mime)
{
    section_foreach_entry(format_array, const struct format_t *, iter) {
        const struct mime_type_t *ptr;
        if ((*iter)->mime_types_rw)
            for(ptr = (*iter)->mime_types_rw; ptr->name; ++ptr)
                if (0 == strcmp(ptr->name, mime)) {
                    arc = malloc((*iter)->objectSize);
                    arc->flags = 0;
                    return *iter;
                }
    }
    section_foreach_entry(format_array, const struct format_t *, iter) {
        const char *const *ptr;
        if ((*iter)->mime_types_ro)
            for(ptr = (*iter)->mime_types_ro; *ptr; ++ptr)
                if (0 == strcmp(*ptr, mime)) {
                    arc = malloc((*iter)->objectSize);
                    arc->flags |= ARCHIVE_READ_ONLY;
                    return *iter;
                }
    }
    return NULL;
}

FILE* loggerFile;

//#define DEFAULT_PATH "/home/arthur/Downloads/cm/addonsu-arm-signed.zip"
//#define DEFAULT_PATH "/home/arthur/dev/build-ncarchiver-Desktop-Debug/bb.7z"
#define DEFAULT_PATH "/home/arthur/dev/build-ncarchiver-Desktop-Debug/archive.7z"
//#define DEFAULT_PATH "/home/arthur/dev/build-ncarchiver-Desktop-Debug/firefox-QMPlay2.tar.gz"

#include <magic.h>

/* main program */
int main(int argc, char **argv)
{
#ifdef TEST_MAGIC
    magic_t magic_cookie = magic_open(MAGIC_MIME_TYPE);
    magic_load(magic_cookie, NULL);
    for (int i = 1; i < argc; i++)
    {
        printf("%s -> %s\n", argv[i], magic_file(magic_cookie, argv[i]));
    }
    magic_close(magic_cookie);
    return 0;
#endif


    loggerFile = fopen("/tmp/ncarchiver.log", "w");
    read_locale();
//  argv_parse(argc, argv);

//  if(dir_ui == 2)
    init_nc();


    char *path = argc > 1 ? argv[1] : DEFAULT_PATH;
    const char *mime = xdg_mime_get_mime_type_for_file(path, NULL);
    printf("mime is %s\n", mime);
    const struct format_t *format = findFormat(mime);
    arc->mime = mime;
    arc->format = format;
    format->openArchive(arc, path);
    arc->dir = format->listFiles(arc);
    if(!arc->dir) {
        switch (arc->error) {
            case ARCHIVE_ERROR_BAD_PASSWORD:
                arc->password = prompt_text("Password", "Enter password:");
                if (!(arc->dir = format->listFiles(arc))) {
                    fprintf(stderr, "Bad Password\n");
                    goto _exit;
                }
                break;
        }
    }
    browse_init(arc->dir);

    while (input_handle(0) != 1);

_exit:
    if(ncurses_init) {
        erase();
        refresh();
        endwin();
    }
    //  exclude_clear();
    format->closeArchive(arc);
    fclose(loggerFile);
    return 0;
}
