#include "cli.h"
#include "filetree.h"
#include "prompt.h"
#include "functions.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <regex.h>

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

        if (access(found_path, X_OK | R_OK) == 0)
            break;

        free(found_path);
        found_path = NULL;

        start = end + 1;
    } while (end != NULL);

    free(env_PATH);
    return found_path;
}

char *getCorrectCommand(const struct cli_format_t *format, int use)
{
    char *res = NULL;
    int i;
    for (i = 0; format->cmds[i] != NULL; ++i)
        if ((format->cmds_uses[i] & use) && (res = getCmdPath(format->cmds[i])) != NULL)
            break;
    return res;
}

bool start_subprocess(pid_t *pid, int *infd, int *outfd, const char *cmd, char **argv, const char *cwd)
{
    int p1[2], p2[2];
    int flags;

    if (infd == NULL || pipe(p1) != -1) {
        if (outfd == NULL || pipe(p2) != -1) {
            if ((*pid = fork()) != -1) {
                if (*pid) { /* Parent process */
                    if (infd != NULL) {
                        *infd = p1[1];
//                        if (-1 == (flags = fcntl(*infd, F_GETFL, 0)))
//                            flags = 0;
//                        fcntl(*infd, F_SETFL, flags | O_NONBLOCK);
                        close(p1[0]);
                    }
                    if (outfd != NULL) {
                        *outfd = p2[0];
                        if (-1 == (flags = fcntl(*outfd, F_GETFL, 0)))
                            flags = 0;
                        fcntl(*outfd, F_SETFL, flags | O_NONBLOCK);
                        close(p2[1]);
                    }
                    return true;
                } else { /* Child process */
                    if (infd != NULL) {
                        dup2(p1[0], STDIN_FILENO);
                        close(p1[0]);
                        close(p1[1]);
                    } else
                        close(STDIN_FILENO);
                    if (outfd != NULL) {
                        dup2(p2[1], STDOUT_FILENO);
                        dup2(p2[1], STDERR_FILENO);
                        close(p2[0]);
                        close(p2[1]);
                    } else {
                        close(STDOUT_FILENO);
                        close(STDERR_FILENO);
                    }
                    if (cwd)
                        chdir(cwd);
                    execvp(cmd, argv);
                    /* Error occured. */
                    LOG_E("cli", "error running %s: %s", cmd, strerror(errno));
                    abort();
                }
            }
            if (outfd != NULL) {
                close(p2[1]);
                close(p2[0]);
            }
        }
        if (infd != NULL) {
            close(p1[1]);
            close(p1[0]);
        }
    }
    return false;
}

static char **cli_passwordArray(const char *const *arr, const char *str)
{
    size_t i, size = str ? arrlen(arr) : 0;
    char **passArgv = (char **)malloc(sizeof(char *) * (1 + size));
    for (i = 0; i < size; i++) {
        if (-1 == asprintf(passArgv + i, arr[i], str)) {
            LOG_e("cli", "Error with asprintf");
            abort();
        }
    }
    passArgv[size] = NULL;
    return passArgv;
}

struct dir_t *cli_listFiles(struct archive_t *archive)
{
    struct cli_format_t *cli_format = (struct cli_format_t *)archive->format;

    char *argv_cmd[] = { getCorrectCommand(cli_format, CLI_LIST), NULL };
    char *argv_path[] = { archive->path, NULL };
    char **passArgv = cli_passwordArray(cli_format->passwordSwitch, archive->password);
    char **argv = arrcatdup(argv_cmd, cli_format->listSwitch, passArgv, argv_path, NULL);
    int i;

    pid_t pid;
    int in, out;
    struct dir_t *root = NULL;
    if (start_subprocess(&pid, &in, &out, argv[0], argv, NULL)) {
        FILE *outF = fdopen(out, "r");
        FILE *inF = fdopen(in, "w");
        if (cli_format->processList)
            root = cli_format->processList(archive, inF, outF);
        fclose(inF);
        fclose(outF);
        kill(pid, SIGKILL);
        waitpid(pid, &i, 0);
    }
    arrfree(passArgv);
    free(argv_cmd[0]);
    free(argv);
    return root;
}

static bool _check_errors(const char* line, const char *const *patterns)
{
    if (!patterns)
        return false;
    int ret;
    regex_t re;
    const char *const *iter;

    for (iter = patterns; *iter != NULL; ++iter) {
        ret = regcomp(&re, *iter, REG_EXTENDED | REG_NOSUB);
        if (ret != REG_NOERROR) {
            LOG_E("cli regex", "Bad regex - %s", *iter);
        }
        ret = regexec(&re, line, 0, NULL, 0);
        regfree(&re);
        if (ret == REG_NOERROR)
            return true;
    }
    return false;
}

int cli_processLineErrors(struct archive_t *archive, const char *line, FILE *inF, int *flags)
{
    static char *repeatingPath = NULL;

    struct cli_format_t *cli_format = (struct cli_format_t *)archive->format;

    regmatch_t matches[10];
    int i;
    regex_t re;

    const char *const *iter;

    if (_check_errors(line, cli_format->errorWrongPassword)) {
        LOG_I("cli parse", "bad password %s for %s", archive->password, archive->path);
        return ARCHIVE_ERROR_BAD_PASSWORD;
    } else if (_check_errors(line, cli_format->errorCorruptedArchive)) {
        LOG_I("cli parse", "corrupted archive %s", archive->path);
        return ARCHIVE_ERROR_CORRUPTED;
    } else if (_check_errors(line, cli_format->errorFullDisk)) {
        LOG_I("cli parse", "disk full for %s", archive->path);
        return ARCHIVE_ERROR_FULL_DISK;
    } else if (_check_errors(line, cli_format->fileExistsPatterns)) {
        int answer = prompt_overwrite(repeatingPath, flags);
        repeatingPath[0] = '\0';
        if (cli_format->fileExistsInput[answer])
            fprintf(inF, "%s\n", cli_format->fileExistsInput[answer]);
        if (answer == 4) {
            LOG_i("cli parse", "parsing was cancelled after file collision");
            return ARCHIVE_ERROR_CANCELED;
        }
        fflush(inF);
    }

    for (iter = cli_format->fileExistsFileName; *iter != NULL; ++iter) {
        regcomp(&re, *iter, REG_EXTENDED);
        i = regexec(&re, line, sizeof(matches) / sizeof(matches[0]), (regmatch_t *)&matches, 0);
        regfree(&re);
        if (i == REG_NOERROR) {
            i = matches[1].rm_eo - matches[1].rm_so;
            repeatingPath = (char *)realloc(repeatingPath, i + 1);
            memcpy(repeatingPath, line + matches[1].rm_so, i);
            repeatingPath[i] = '\0';
            LOG_I("cli parse", "found repeating file %s", repeatingPath);
            return ARCHIVE_ERROR_NO;
        }
    }

    return ARCHIVE_ERROR_NO;
}

static int cli_normal_runProcess(struct archive_t *archive, char **argv, const char *cwd)
{
    struct cli_format_t *cli_format = (struct cli_format_t *)archive->format;

    pid_t pid;
    int in, out;
    FILE *outF, *inF;
    int res = ARCHIVE_ERROR_NO;
    int flags = 0;
    if (start_subprocess(&pid, &in, &out, argv[0], argv, cwd)) {
        inF = fdopen(in, "w");
        outF = fdopen(out, "r");
        char line[2048 + 1];
        line[sizeof(line) - 1] = '\0';
        while (!feof(outF) && res == ARCHIVE_ERROR_NO) {
            if (NULL == fgets(line, sizeof(line) - 1, outF)) {
                if (errno == EAGAIN)
                    continue;
            }
            line[strlen(line) - 1] = '\0'; // remove new line

            if (*line == '\0')
                continue;

            res = (cli_format->processLine ?: cli_processLineErrors)(archive, line, inF, &flags);
        }
        fclose(outF);
        fclose(inF);
        kill(pid, SIGKILL);
        waitpid(pid, NULL, 0);
    }
    return res;
}

bool cli_extractFiles(struct archive_t *archive, const char *const *files, const char *destinationFolder)
{
    struct cli_format_t *cli_format = (struct cli_format_t *)archive->format;
    char *argv_cmd[] = { getCorrectCommand(cli_format, CLI_EXTRACT), NULL };
    char *argv_path[] = { archive->path, NULL };
    char **passArgv = cli_passwordArray(cli_format->passwordSwitch, archive->password);
    char **argv = arrcatdup(argv_cmd, cli_format->extractSwitch, passArgv, argv_path, files, NULL);

    int res = cli_normal_runProcess(archive, argv, destinationFolder);

    arrfree(passArgv);
    free(argv[0]);
    free(argv);

    return res == ARCHIVE_ERROR_NO;
}

bool cli_deleteFiles(struct archive_t *archive, const char *const *files)
{
    struct cli_format_t *cli_format = (struct cli_format_t *)archive->format;
    char *argv_cmd[] = { getCorrectCommand(cli_format, CLI_DELETE), NULL };
    char *argv_path[] = { archive->path, NULL };
    char **passArgv = cli_passwordArray(cli_format->passwordSwitch, archive->password);
    char **argv = arrcatdup(argv_cmd, cli_format->delSwitch, passArgv, argv_path, files, NULL);

    int res = cli_normal_runProcess(archive, argv, NULL);

    arrfree(passArgv);
    free(argv[0]);
    free(argv);

    return res;
}

bool cli_testFiles(struct archive_t *archive)
{
    struct cli_format_t *cli_format = (struct cli_format_t *)archive->format;
    char *argv_cmd[] = { getCorrectCommand(cli_format, CLI_TEST), NULL };
    char *argv_path[] = { archive->path, NULL };
    char **argv_pass = cli_passwordArray(cli_format->passwordSwitch, archive->password);
    char **argv = arrcatdup(argv_cmd, cli_format->testSwitch, argv_pass, argv_path, NULL);

    int res = cli_normal_runProcess(archive, argv, NULL);

    arrfree(argv_pass);
    free(argv[0]);
    free(argv);

    return res;
}

static const char *_archive_mime_finder(const char *mime, const struct pMimeStr* args)
{
    for (; args->mime; args++)
        if (0 == strcmp(args->mime, mime))
            return args->str;
    return NULL;
}

bool cli_addFiles(struct archive_t *archive, const char *const *files, const struct compression_options_t *options)
{
    struct cli_format_t *cli_format = (struct cli_format_t *)archive->format;
    char *argv_cmd[] = { getCorrectCommand(cli_format, CLI_ADD), NULL };
    char *argv_path[] = { archive->path, NULL };
    char **argv_pass = cli_passwordArray(options->encryptHeaders ? cli_format->passwordHeadersSwitch : cli_format->passwordSwitch, archive->password ?: options->password);
    char *argv_add[5];

    int i = 0;
    if (options->compressionLevel > -1) {
        char lev[] = "0";
        lev[0] += options->compressionLevel;
        asprintf(argv_add + (i++), _archive_mime_finder(archive->mime, cli_format->compressionLevelSwitch), lev);
    }
    if (options->compressionMethod > -1)
        asprintf(argv_add + (i++), _archive_mime_finder(archive->mime, cli_format->compressionMethodSwitch), options->mime->compressionMethods[options->compressionMethod]);
    if (options->encryptionMethod > -1)
        asprintf(argv_add + (i++), _archive_mime_finder(archive->mime, cli_format->encryptionMethodSwitch), options->mime->encryptionMethods[options->encryptionMethod]);
    argv_add[i] = NULL;

    char **argv = arrcatdup(argv_cmd, cli_format->addSwitch, argv_pass, argv_add, argv_path, files, NULL);

    int res = cli_normal_runProcess(archive, argv, NULL);

    for (i = 0; argv_add[i]; i++)
        free(argv_add[i]);
    free(argv[0]);
    free(argv);
    return res;
}
