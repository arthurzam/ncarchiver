#include "global.h"
#include "global_ui.h"
#include "filetree.h"
#include "textbox.h"

#include <string.h>
#include <stdlib.h>

#include <ncurses.h>
#include <ctype.h>

static struct {
    const char **names;
    const struct mime_type_t **mimes;
    unsigned size;
} types = {NULL, NULL, 0};

struct newfiledialog_t {
    struct compression_options_t options;
    unsigned selected_row;
    unsigned selected_type;
    unsigned size;
    struct textbox_data_t tb_location;
    struct textbox_data_t tb_filename;
    struct textbox_data_t tb_password;
};

static int _qsort_strtype(const void *a, const void *b)
{
    return strcmp((*(const struct mime_type_t **)a)->extension, (*(const struct mime_type_t **)b)->extension);
}

static void _newfiledialog_setTypes()
{
    const struct mime_type_t *ptr;
    unsigned i;
    section_foreach_entry(format_array, const struct format_t *, iter)
    {
        if ((*iter)->mime_types_rw)
            for(ptr = (*iter)->mime_types_rw; ptr->name; ++ptr)
            {
                if (!ptr->extension)
                    continue;
                for (i = 0; i < types.size; ++i)
                    if (0 == strcmp(ptr->extension, types.mimes[i]->extension))
                        break;
                if (i == types.size)
                {
                    ++types.size;
                    types.mimes = realloc(types.mimes, sizeof(const struct mime_type_t *) * types.size);
                    types.mimes[i] = ptr;
                }
            }
    }
    qsort (types.mimes, types.size, sizeof(const struct mime_type_t *), _qsort_strtype);
    types.names = (const char **)malloc(sizeof(const char *) * (types.size + 1));
    for (i = 0; i < types.size; ++i)
        types.names[i] = types.mimes[i]->prettyText;
    types.names[i] = NULL;
}

static unsigned _newfiledialog_mimeSize(const struct mime_type_t *mime)
{
    unsigned size = 8;
    if (mime->compressionMethods || mime->compressionLevelMin != -1 || mime->compressionLevelMax != -1)
    {
        size += 2;
        if (mime->compressionMethods)
            size++;
        if (mime->compressionLevelMin != -1 || mime->compressionLevelMax != -1)
            size++;
    }
    if (mime->encryptionMethods)
        size += 5;
    return size;
}

enum SelRows {
    ROW_OK = 0,
    ROW_CANCEL,
    ROW_LOCATION,
    ROW_FILENAME,
    ROW_TYPE,

    ROW_COMPR_METHOD,
    ROW_COMPR_LEVEL,

    ROW_ENCR_METHOD,
    ROW_ENCR_PASS,
    ROW_ENCR_HDR
};

static int newfiledialog_key(int index, int key) {
    struct newfiledialog_t *data = (struct newfiledialog_t *)ui_data[index];
    struct textbox_data_t *selectedTB = (
                    data->selected_row == ROW_FILENAME  ? &data->tb_filename :
                    data->selected_row == ROW_ENCR_PASS ? &data->tb_password :
                    NULL
                );
    if (selectedTB && textbox_key(selectedTB, key))
        return 0;

    const struct mime_type_t *mime = types.mimes[data->selected_type];
    switch (key) {
        case KEY_TAB:
        case KEY_DOWN:
            ++data->selected_row;
            if (data->selected_row == ROW_COMPR_METHOD && mime->compressionMethods == NULL)
                data->selected_row = ROW_COMPR_LEVEL;
            if (data->selected_row == ROW_COMPR_LEVEL && mime->compressionLevelMin == -1 && mime->compressionLevelMax == -1)
                data->selected_row = ROW_ENCR_METHOD;
            if (data->selected_row == ROW_ENCR_METHOD && mime->encryptionMethods == NULL)
                data->selected_row = ROW_ENCR_PASS;
            if (data->selected_row == ROW_ENCR_HDR + 1)
                data->selected_row = ROW_OK;
            return 0;
        case KEY_UP:
            if (data->selected_row == 0)
                data->selected_row = ROW_ENCR_HDR + 1;
            --data->selected_row;
            if (data->selected_row == ROW_ENCR_METHOD && mime->encryptionMethods == NULL)
                data->selected_row = ROW_COMPR_LEVEL;
            if (data->selected_row == ROW_COMPR_LEVEL && mime->compressionLevelMin == -1 && mime->compressionLevelMax == -1)
                data->selected_row = ROW_COMPR_METHOD;
            if (data->selected_row == ROW_COMPR_METHOD && mime->compressionMethods == NULL)
                data->selected_row = ROW_TYPE;
            return 0;
        case KEY_HOME:
            data->selected_row = ROW_LOCATION;
            break;
        case KEY_END:
            data->selected_row = ROW_OK;
            return newfiledialog_key(index, KEY_UP);
        case KEY_RETURN:
        case KEY_ENTER:
            if (data->selected_row <= ROW_CANCEL)
                return 1;
            if (data->selected_row == ROW_TYPE)
            {
                unsigned prev = data->selected_type;
                data->selected_type = prompt_list_init("Archive Type", types.names, data->selected_type);
                if (prev != data->selected_type)
                {
                    data->options.mime = mime = types.mimes[data->selected_type];
                    data->options.compressionLevel = mime->compressionLevelDefault;
                    data->options.compressionMethod = mime->compressionMethodDefault;
                    data->options.encryptionMethod = mime->encryptionMethodDefault;
                    data->size = _newfiledialog_mimeSize(mime);
                }
            }
            else if (data->selected_row == ROW_COMPR_METHOD)
                data->options.compressionMethod = prompt_list_init("Compression Method", types.mimes[data->selected_type]->compressionMethods, data->options.compressionMethod);
            else if (data->selected_row == ROW_ENCR_METHOD)
                data->options.encryptionMethod = prompt_list_init("Encryption Method", types.mimes[data->selected_type]->encryptionMethods, data->options.encryptionMethod);
            else if (data->selected_row == ROW_LOCATION)
            {
                char *res = fselect_init(data->options.location, FSELECT_DIRS_ONLY);
                free(data->options.location);
                data->options.location = res;
            }
            else if (data->selected_row == ROW_ENCR_HDR)
                data->options.encryptHeaders = !data->options.encryptHeaders;
            return 0;
        case KEY_SPACE:
            if (data->selected_row == ROW_ENCR_HDR)
            {
                data->options.encryptHeaders = !data->options.encryptHeaders;
                return 0;
            }
            break;
        case '+':
            if (data->selected_row == ROW_COMPR_LEVEL)
            {
                if (data->options.compressionLevel < types.mimes[data->selected_type]->compressionLevelMax)
                    ++data->options.compressionLevel;
                return 0;
            }
            break;
        case '-':
            if (data->selected_row == ROW_COMPR_LEVEL)
            {
                if (data->options.compressionLevel > types.mimes[data->selected_type]->compressionLevelMin)
                    --data->options.compressionLevel;
                return 0;
            }
            break;
    }
    return 0;
}

#define WINDOW_WIDTH 60

static void newfiledialog_draw(int index) {
    struct newfiledialog_t *data = (struct newfiledialog_t *)ui_data[index];
    const struct mime_type_t *mime = types.mimes[data->selected_type];
    unsigned row = 2;

    nccreate(data->size, WINDOW_WIDTH, "Create New Archive");

    draw_label  (row, 2, "Location:", data->selected_row == ROW_LOCATION);
    textbox_draw(&data->tb_location, row++, 12, data->selected_row == ROW_LOCATION ? TEXTBOX_SELECTED : 0);
    draw_label  (row, 2, "Filename:", data->selected_row == ROW_FILENAME);
    textbox_draw(&data->tb_filename, row++, 12, data->selected_row == ROW_FILENAME ? TEXTBOX_SELECTED : 0);
    draw_label  (row, 2, "Format:"  , data->selected_row == ROW_TYPE);
    attron(A_UNDERLINE);
    ncprint     (row++, 12, "%s (*.%s)", mime->prettyText, mime->extension);
    attroff(A_UNDERLINE);


    if (mime->compressionMethods || mime->compressionLevelMin != -1 || mime->compressionLevelMax != -1)
    {
        row++;
        ncaddstr(row++, 2, "Compression");
        if (mime->compressionMethods)
        {
            draw_label(row, 4, "Method:", data->selected_row == ROW_COMPR_METHOD);
            ncaddstr  (row++, 14, mime->compressionMethods[data->options.compressionMethod]);
        }
        if (mime->compressionLevelMin != -1 || mime->compressionLevelMax != -1)
        {
            draw_label(row, 4, "Level:", data->selected_row == ROW_COMPR_LEVEL);
            ncprint   (row++, 14, "%d [%-*s%c%-*s] %d",
                        mime->compressionLevelMin, data->options.compressionLevel - mime->compressionLevelMin, "",
                        '0' + data->options.compressionLevel,
                        mime->compressionLevelMax - data->options.compressionLevel, "", mime->compressionLevelMax);
        }
    }

    if (mime->encryptionMethods)
    {
        row++;
        ncaddstr(row++, 2, "Encryption");
        draw_label(row, 4, "Method:", data->selected_row == ROW_ENCR_METHOD);
        ncaddstr  (row++, 14, mime->encryptionMethods[data->options.encryptionMethod]);

        draw_label  (row, 4, "Password:", data->selected_row == ROW_ENCR_PASS);
        textbox_draw(&data->tb_password, row++, 14, (data->selected_row == ROW_ENCR_PASS ? TEXTBOX_SELECTED : 0) | TEXTBOX_HIDE);

        if (data->selected_row == ROW_ENCR_HDR)
            attron(A_REVERSE);
        ncprint (row++, 4, "[%c] Encrypt Headers", data->options.encryptHeaders ? 'X' : ' ');
        if (data->selected_row == ROW_ENCR_HDR)
            attroff(A_REVERSE);
    }


    row++;
    draw_label(row, WINDOW_WIDTH / 4 - 1,       "OK"    , data->selected_row == ROW_OK);
    draw_label(row, (3 * WINDOW_WIDTH / 4) - 3, "Cancel", data->selected_row == ROW_CANCEL);
}

struct compression_options_t *newfiledialog_init() {
    if (!types.names)
        _newfiledialog_setTypes();
    const struct mime_type_t *mime = types.mimes[0];
    struct newfiledialog_t *data = TYPE_MALLOC(struct newfiledialog_t);
    memset(data, 0, sizeof(struct newfiledialog_t));
    textbox_init(&data->tb_location, getHomeDir(), isprint, WINDOW_WIDTH - 14);
    textbox_init(&data->tb_filename, NULL, isprint, WINDOW_WIDTH - 14);
    textbox_init(&data->tb_password, NULL, isprint, WINDOW_WIDTH - 16);
    data->options.compressionLevel = mime->compressionLevelDefault;
    data->options.compressionMethod = mime->compressionMethodDefault;
    data->options.encryptionMethod = mime->encryptionMethodDefault;
    data->size = _newfiledialog_mimeSize(mime);
    data->options.mime = mime;

    ui_insert(newfiledialog_draw, newfiledialog_key, data);
    while (input_handle(0) != 1);
    ui_remove();

    if (data->selected_row == ROW_CANCEL)
    {
        free(data);
        return NULL;
    }

    return &data->options;
}
