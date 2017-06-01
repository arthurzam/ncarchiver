#if 0
#include <stdio.h>

#include "global.h"
#include "filetree.h"

#include <xdgmime.h>

#define section_foreach_entry(section_name, type_t, elem)     \
     for (type_t *elem =                                      \
            ({                                                \
                extern type_t __start_##section_name;         \
                &__start_##section_name;                      \
            });                                               \
            elem !=                                           \
            ({                                                \
                extern type_t __stop_##section_name;          \
                &__stop_##section_name;                       \
            });                                               \
            ++elem)

void outputAll(struct dir_t *dir)
{
    if (!dir) return;
    puts(filetree_getpath(dir));
    for (struct dir_t *node = dir->subs; node; node = node->next)
        outputAll(node);
}

int main(/*int argc, char *argv[]*/)
{
    printf("%s\n", xdg_mime_get_mime_type_for_file("/home/arthur/dev/firefox-QMPlay2.tar", NULL));
    section_foreach_entry(format_array, struct format_t *, iter)
    {
        printf("module %s\n", (*iter)->name);
        struct archive_t *arc = (*iter)->openArchive((*iter), "/home/arthur/dev/firefox-QMPlay2.tar");
        arc->dir = (*iter)->listFiles(arc);
        outputAll(arc->dir);
        puts("******************");
        filetree_sort(arc->dir->subs->subs);
        outputAll(arc->dir);
        puts("******************");
        (*iter)->closeArchive(arc);
    }
    return 0;
}

#else


#include "global_ui.h"
#include "filetree.h"
#include "cli.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <sys/time.h>


int pstate = ST_BROWSE;
int read_only = 0;
long update_delay = 100;
int cachedir_tags = 0;

static int min_rows = 17, min_cols = 60;
static int ncurses_init = 0;
static int ncurses_tty = 0; /* Explicitely open /dev/tty instead of using stdio */
static long lastupdate = 999;
#ifndef TEST_UI


static void screen_draw() {
    switch(pstate) {
        //case ST_CALC:   dir_draw();    break;
        case ST_BROWSE: browse_draw(); break;
        //case ST_HELP:   help_draw();   break;
        //case ST_SHELL:  shell_draw();  break;
        //case ST_DEL:    delete_draw(); break;
        case ST_QUIT:   quit_draw();   break;
    }
}


/* wait:
 *  -1: non-blocking, always draw screen
 *   0: blocking wait for input and always draw screen
 *   1: non-blocking, draw screen only if a configured delay has passed or after keypress
 */
int input_handle(int wait)
{
    int ch;
    struct timeval tv;

    if (wait != 1)
        screen_draw();
    else
    {
        gettimeofday(&tv, (void *)NULL);
        tv.tv_usec = (1000 * (tv.tv_sec % 1000) + (tv.tv_usec / 1000)) / update_delay;
        if (lastupdate != tv.tv_usec)
        {
            screen_draw();
            lastupdate = tv.tv_usec;
        }
    }

    /* No actual input handling is done if ncurses hasn't been initialized yet. */
    if (!ncurses_init)
        return !wait;

    nodelay(stdscr, !!wait);
    while ((ch = getch()) != ERR)
    {
        if (ch == KEY_RESIZE)
        {
            if(ncresize(min_rows, min_cols))
                min_rows = min_cols = 0;
            /* ncresize() may change nodelay state, make sure to revert it. */
            nodelay(stdscr, wait?1:0);
            screen_draw();
            continue;
        }
        switch(pstate)
        {
            //case ST_CALC:   return dir_key(ch);
            case ST_BROWSE: return browse_key(ch);
            //case ST_HELP:   return help_key(ch);
            //case ST_DEL:    return delete_key(ch);
            case ST_QUIT:   return quit_key(ch);
        }
        screen_draw();
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

    if(ncurses_tty)
    {
        tty = fopen("/dev/tty", "r+");
        if(!tty)
        {
            fprintf(stderr, "Error opening /dev/tty: %s\n", strerror(errno));
            exit(1);
        }
        term = newterm(NULL, tty, tty);
        if(term)
            set_term(term);
        ok = !!term;
    }
    else
    {
        /* Make sure the user doesn't accidentally pipe in data to ncdu's standard
         * input without using "-f -". An annoying input sequence could result in
         * the deletion of your files, which we want to prevent at all costs. */
        if(!isatty(0))
        {
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


extern struct cli_format_t cli_7z_proc;
struct dir_t *curr, *selected;
/* main program */
int main(int argc, char **argv) {

    struct archive_t *arc = cli_7z_proc.parent.openArchive(&cli_7z_proc.parent, argc > 1 ? argv[1] : "/home/arthur/dev/build-ncarchiver-Desktop-Debug/ui/archive.7z");
    //arc->password = "1234";
    curr = arc->dir = cli_7z_proc.parent.listFiles(arc);
    selected = filetree_sort(curr->subs);
  read_locale();
//  argv_parse(argc, argv);

//  if(dir_ui == 2)
    init_nc();

  while(1) {
    if(input_handle(0))
      break;
  }

  if(ncurses_init) {
    erase();
    refresh();
    endwin();
  }
//  exclude_clear();

  cli_7z_proc.parent.closeArchive(arc);
  return 0;
}


#endif
#endif
