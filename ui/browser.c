#include "global.h"
#include "global_ui.h"
#include "filetree.h"
#include "actions.h"
#include "functions.h"

#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <ncurses.h>


// static int graph = 1, show_as = 0, info_show = 0, info_page = 0, info_start = 0, show_items = 0;
static struct dir_t *curr, *selected;

static struct dir_t **selected_array = NULL;
static unsigned selected_size = 0;

static void browse_draw_item(struct dir_t *n, int row) {
    char type, *size, dt;
    int col = 0;

    if (n == selected)
        attron(A_REVERSE);
    if (n == curr->parent) {
        mvaddstr(row, 17, "/..");
        if (n == selected)
            attroff(A_REVERSE);
        return;
    }

    type = ((n->flags & NODE_ISDIR) ? (n->items > 0 ? 'D' : 'e') : ' ');
    size = ((n->flags & NODE_ISDIR) ? "" : formatsize(n->realSize));
    dt = ((n->flags & NODE_ISDIR) ? '/' : ' ');

    mvprintw(row, col, "%3s %c %9s ", ((n->flags & NODE_SELECTED) ? "(*)" : ""), type, size);
    col += 16;


    mvprintw(row, col, " %c%-*s", dt, wincols - 2 - col, cropstr(n->name, wincols - 2 - col));
    if (n == selected)
        attroff(A_REVERSE);
}


static void browse_draw(int index) {
    (void)index;
    int i = 2;
    unsigned col;

    erase();

    /* top line - basic info */
    attron(A_REVERSE);
    mvhline(0, 0, ' ', wincols);
    mvhline(winrows - 1, 0, ' ', wincols);
    mvprintw(0, 0, "%s %s ~ Use the arrow keys to navigate, press ? for help", PACKAGE_NAME, PACKAGE_VERSION);
    col = wincols + 1;
    if (arc->flags & ARCHIVE_READ_ONLY)
        mvaddstr(0, col -= 12, "[read only]");
    if (geteuid() == 0)
        mvaddstr(0, col -= 7,  "[root]");
    attroff(A_REVERSE);

    /* second line - the path */
    mvhline(1, 0, '-', wincols);
    mvaddch(1, 3, ' ');
    char *tmp = filetree_getpath(curr);
    mvaddstr(1, 4, cropstr(tmp, wincols-8));
    mvaddch (1, 4 + min((int)strlen(tmp), wincols - 8), ' ');

    /* bottom line - shortcuts */
    attron(A_REVERSE);
    mvaddstr(winrows - 1, 1, "(N)ew   (O)pen   (Q)uit   (D)elete   (T)est   (I)nfo   (A)dd   (E)xtract");
    attroff(A_REVERSE);

    /* get start position */
    struct dir_t *t = curr->subs;

    if (curr->parent)
        browse_draw_item(curr->parent, i++);

    /* print the list to the screen */
    for(; t && i < winrows - 3; t = t->next, i++) {
        browse_draw_item(t, i);
    }

    /* move cursor to selected row for accessibility */
}

static int browse_key(int index, int ch) {
    (void)index;
    switch (ch) {
        case KEY_UP:
            if (selected == curr->parent) break;
            if (selected->prev)
                selected = selected->prev;
            else if (curr->parent)
                selected = curr->parent;
            break;
        case KEY_DOWN:
            if (selected == curr->parent)
                selected = curr->subs;
            else if (selected->next)
                selected = selected->next;
            break;
        case '\n':
        case KEY_ENTER:
        case KEY_RIGHT:
            if (selected->flags & NODE_ISDIR) {
                curr = selected;
                selected = filetree_sort(curr->subs);
            }
            break;
        case KEY_SPACE:
            if (selected != curr->parent) {
                selected->flags ^= NODE_SELECTED;
                if (selected->flags & NODE_SELECTED) {
                    selected_array = realloc(selected_array, sizeof(struct dir_t *) * (++selected_size));
                    selected_array[selected_size - 1] = selected;
                } else {
                    unsigned i = 0;
                    for (; selected_array[i] != selected; ++i);
                    for (++i; i < selected_size; ++i)
                        selected_array[i - 1] = selected_array[i];
                    selected_array = realloc(selected_array, sizeof(struct dir_t *) * (--selected_size));
                }
            }
            break;
        case 'q':
        case 'Q':
            if (prompt_yesno("Confirm Quit", "Really quit? (y/N)", 30))
                return 1;
            break;
        case 'o':
        case 'O':
            if (selected_size > 0) {
                char **files = filetree_getArr(selected_array, selected_size);
                actions_openFiles((const char *const *)files);
                arrfree(files);
            }
            break;
        case 'd':
        case 'D':
            if (!(arc->flags & ARCHIVE_READ_ONLY) && selected_size > 0) {
                if (prompt_yesno("Delete files", "Are you sure you want to delete files?", 42)) {
                    char **files = filetree_getArr(selected_array, selected_size);
                    arc->format->deleteFiles(arc, files);
                    arrfree(files);
                }
            }
            break;
        case 't':
        case 'T':
        {
            if (arc->format->testFiles)
                arc->format->testFiles(arc);
        }
            break;
        case 'n':
        {
            struct compression_options_t *options = newfiledialog_init();
            if (!options)
                break;
            const char *files[1] = {NULL};
            struct archive_t *newArc = NULL;
            section_foreach_entry(format_array, const struct format_t *, iter) {
                const struct mime_type_t *ptr;
                if ((*iter)->mime_types_rw)
                    for(ptr = (*iter)->mime_types_rw; ptr->name; ++ptr)
                        if (ptr == options->mime) {
                            newArc = malloc((*iter)->objectSize);
                            newArc->format = *iter;
                            newArc->mime = options->mime->name;
                            break;
                        }
                if (newArc)
                    break;
            }
            NC_ASSERT_NONNULL(options->location);
            NC_ASSERT_NONNULL(options->filename);
            char *path = strconcat(options->location, options->filename, '/');
            newArc->format->openArchive(newArc, path);
            free(path);
            newArc->format->addFiles(newArc, files, options);
            free(options);

            if (prompt_yesno("Replace Archive", "Open new Archive ?", 24)) {
                arc->format->closeArchive(arc);
                arc = newArc;
                newArc->dir = newArc->format->listFiles(newArc);
                ui_remove();
                browse_init(newArc->dir);
            } else {
                newArc->format->closeArchive(newArc);
            }
        }
            break;
        case 'i':
            nodeinfo_init(selected);
            break;
        case 'E':
        case 'e':
            extractdialog_init(selected_array, selected_size);
            break;
        case 'A':
        case 'a':
            addfilesdialog_init(curr, NULL);
            break;
//        default:
//        {
//            char str[500];
//            sprintf(str, "%d", ch);
//            prompt_yesno("KEY", str, 24);
//            sprintf(str, "%d %c", ch - KEY_MAX, ch - KEY_MAX);
//            prompt_yesno("KEY", str, 24);
//        }
//            break;
    }
    return 0;
}


void browse_init(struct dir_t *base) {
    ui_insert(browse_draw, browse_key, NULL);
    curr = base;
    if (curr->subs)
        selected = filetree_sort(curr->subs);
}

