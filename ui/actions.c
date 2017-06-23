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
    int pid, status, n;
    char fullPath[FILENAME_MAX];
    char *argv[] = {"sh", "-c", "xdg-open", fullPath, NULL};

    char dTemplate[] = "/tmp/.-ncark-XXXXXX"; // len = 19
    mkdtemp(dTemplate);

    res = res & arc->format->extractFiles(arc, files, dTemplate);
    memcpy(fullPath, dTemplate, 19);
    fullPath[19] = '/';
    fullPath[20] = '\0';

    for (ptr = files; res && *ptr != NULL; ++ptr)
    {
        strcpy(fullPath + 20, *ptr);
        switch (pid = fork())
        {
            case -1:
                return false;
            case 0:
                setsid();
                close(0);
                close(1);
                close(2);
                n = open("/dev/null", O_RDWR);
                dup2(n, STDIN_FILENO);
                dup2(n, STDOUT_FILENO);
                dup2(n, STDERR_FILENO);
                close(n);
                execvp("/bin/sh", argv);
                exit(0);
                break;
            default:
                waitpid(pid, &status, 0);
                res = res && (0 == status);
        }
    }
    return res;
}
