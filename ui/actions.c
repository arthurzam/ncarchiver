#include "actions.h"
#include "global_ui.h"
#include "cli.h"

#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

bool actions_openFiles(const char *const *files)
{
    const char *const *ptr;
    bool res = true;
    int pid, status;
    char fullPath[FILENAME_MAX];
    char *argv[] = {getCmdPath("sh"), getCmdPath("xdg-open"), fullPath, NULL};

    char dTemplate[] = "/tmp/.-ncark-XXXXXX"; // len = 19
    mkdtemp(dTemplate);

    res = res & arc->format->extractFiles(arc, files, dTemplate);
    memcpy(fullPath, dTemplate, 19);
    fullPath[19] = '/';
    fullPath[20] = '\0';

    def_prog_mode();
    endwin();

    for (ptr = files; res && *ptr != NULL; ++ptr) {
        strcpy(fullPath + 20, *ptr);
        switch (pid = fork()) {
            case -1:
                res = false;
                break;
            case 0:
                execv(argv[0], argv);
                exit(0);
                break;
            default:
                waitpid(pid, &status, 0);
                res = res && (0 == status);
        }
    }
    free(argv[0]);
    free(argv[1]);
    refresh();
    return res;
}
