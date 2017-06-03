#include "cli.h"
#include "filetree.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>

char *getCmdPath(const char *cmd)
{
    char *const env_PATH = strdup(getenv("PATH"));
    char *found_path = NULL;
    const int cmd_len = strlen(cmd);

    char *start = env_PATH;
    char *end = NULL;

    do {
        end = strchr(start, ':');
        if (end != NULL)
            end[0] = 0;

        const int path_len = strlen(start);
        found_path = (char *)malloc(path_len + cmd_len + 3);
        memcpy(found_path, start, path_len);
        found_path[path_len] = '/';
        memcpy(found_path + 1 + path_len, cmd, cmd_len + 1);

        if (access(found_path, X_OK | R_OK) == 0) // TODO: check if it works. If not, replace with F_OK
            break;

        free(found_path);
        found_path = NULL;

        start = end + 1;
    } while (end != NULL);

    free(env_PATH);
    return found_path;
}

bool start_subprocess(int *pid, int *infd, int *outfd, const char *cmd, char **argv)
{
    int p1[2], p2[2];

    if (!pid || !outfd)
        return false;

    if (infd == NULL || pipe(p1) != -1)
    {
        if (pipe(p2) != -1)
        {
            if ((*pid = fork()) != -1)
            {
                if (*pid) /* Parent process */
                {
                    if (infd != NULL)
                    {
                        *infd = p1[1];
                        close(p1[0]);
                    }
                    *outfd = p2[0];
                    close(p2[1]);
                    return true;
                }
                else /* Child process */
                {
                    if (infd != NULL)
                    {
                        dup2(p1[0], STDIN_FILENO);
                        close(p1[0]);
                        close(p1[1]);
                    }
                    else
                        close(STDIN_FILENO);
                    dup2(p2[1], 1);
                    close(p2[0]);
                    close(p2[1]);
                    execvp(cmd, argv);
                    /* Error occured. */
                    fprintf(stderr, "error running %s: %s", cmd, strerror(errno));
                    abort();
                }
            }
            close(p2[1]);
            close(p2[0]);
        }
        if (infd != NULL)
        {
            close(p1[1]);
            close(p1[0]);
        }
    }
    return false;
}

static size_t arrlen(const char *const *arr) __attribute__((pure));
static size_t arrlen(const char *const *arr)
{
    if (!arr) return 0;
    size_t size;
    for (size = 0; *arr; ++size, ++arr);
    return size;
}

static char **arrcpy(char **dst, char **src) __attribute__ ((__nonnull__ (1)));
static char **arrcpy(char **dst, char **src)
{
    if (!src) return dst;
    for (; *src; ++dst, ++src)
        *dst = *src;
    return dst;
}

static void arrfree(char **arr)
{
    char **ptr;
    for (ptr = arr; *ptr; ++ptr)
        free(*ptr);
    free(arr);
}

static char **cli_passwordArray(struct archive_t *_archive)
{
    struct cli_format_t *cli_format = (struct cli_format_t *)_archive->format;
    size_t i, size = _archive->password ? arrlen(cli_format->passwordSwitch) : 0;
    char **passArgv = (char **)malloc(sizeof(char *) * (1 + size));
    for (i = 0; i < size; i++)
    {
        if (-1 == asprintf(passArgv + i, cli_format->passwordSwitch[i], _archive->password))
        {
            fputs("Error with asprintf", stderr);
            abort();
        }
    }
    passArgv[size] = NULL;
    return passArgv;
}

struct dir_t *cli_listFiles(struct archive_t *_archive)
{
    struct cli_format_t *cli_format = (struct cli_format_t *)_archive->format;

    int size = 3 + arrlen(cli_format->listSwitch) + arrlen(cli_format->passwordSwitch), i;
    char **argv = (char **)malloc(sizeof(char *) * size), **ptr;
    char **passArgv = cli_passwordArray(_archive);
    argv[0] = getCmdPath(cli_format->cmds[0]);
    ptr = arrcpy(argv + 1, (char **)cli_format->listSwitch);
    ptr = arrcpy(ptr, passArgv);
    ptr[0] = _archive->path;
    ptr[1] = NULL;

    int pid, in, out;
    struct dir_t *root = NULL;
    if (start_subprocess(&pid, &in, &out, argv[0], argv))
    {
        FILE *outF = fdopen(out, "r");
        FILE *inF = fdopen(in, "w");
        if (cli_format->processList)
            root = cli_format->processList(_archive, inF, outF);
        fclose(inF);
        fclose(outF);
        waitpid(pid, &i, 0);
    }
    arrfree(passArgv);
    free(argv[0]);
    free(argv);
    return root;
}
