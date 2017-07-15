#ifndef LISTVIEW_H
#define LISTVIEW_H

struct listview_data_t {
    char **items;
    unsigned items_count;
    int first_row;
    int selected_row;
    unsigned height, width;
};

void listview_key(struct listview_data_t *data, int key);
void listview_draw(const struct listview_data_t *data);
void listview_init(struct listview_data_t *data, char **items, unsigned height, unsigned width, unsigned selected);

#endif // LISTVIEW_H
