#include "filetree.h"

#include <string.h>
#include <stdio.h>

struct dir_t *filetree_addNode(struct dir_t *root, char *path)
{
    char *start = path, *end = NULL;
    struct dir_t *temp;

    for (; *start == '/'; ++start);

    while ((end = strchr(start, '/')) != NULL)
    {
        *end = '\0';

        // search for this dir
        for (temp = root->subs; temp; temp = temp->next)
        {
            if (strcmp(temp->name, start) == 0)
            {
                root = (struct dir_t *)temp;
                break;
            }
        }

        // insert new empty dir node
        if (root != (struct dir_t *)temp)
        {
            temp = TYPE_MALLOC(struct dir_t);
            *temp = (struct dir_t) {
                .parent = root, .prev = NULL, .next = root->subs, .subs = NULL, .name = strdup(start),
                .moreInfo = NULL, .flags = NODE_ISDIR, .realSize = 0, .compressSize = 0, .items = 0
            };

            if (root->subs)
                root->subs->prev = temp;
            root->items++;
            root->subs = temp;

            root = temp;
        }

        *end = '/';
        start = end + 1;
        for (; *start == '/'; ++start);
    }

    if (*start != '\0')
    {
        temp = TYPE_MALLOC(struct dir_t);
        *temp = (struct dir_t) {
            .parent = root, .prev = NULL, .next = root->subs, .subs = NULL, .name = strdup(start),
            .moreInfo = NULL, .flags = 0, .realSize = 0, .compressSize = 0, .items = 0
        };

        if (root->subs)
            root->subs->prev = temp;
        root->items++;
        root->subs = temp;
    }
    return temp;
}

void filetree_free(struct dir_t *root)
{
    struct dir_t *node, *next;
    struct dir_more_info_t *moreInfo;
    if (!root)
        return;

    for (node = root->subs; node; node = next)
    {
        next = node->next;
        free(node->name);
        if (node->moreInfo)
        {
            for (moreInfo = node->moreInfo; moreInfo->key; ++moreInfo)
                free(moreInfo->value);
            free(node->moreInfo);
        }
        node->name = NULL;
        filetree_free(node);
    }

    free(root);
}

uint8_t sort_flags = SORT_DIRS_FIRST | SORT_COL_NAME;

static int filetree_cmp(struct dir_t *x, struct dir_t *y)
{
    if (sort_flags & SORT_DIRS_FIRST)
    {
        if ((x->flags & NODE_ISDIR) != (y->flags & NODE_ISDIR))
        {
            return ((x->flags & NODE_ISDIR) ? -1 : 1);
        }
    }
    return strcmp(x->name, y->name);
}

struct dir_t *filetree_sort(struct dir_t *list) {
  struct dir_t *p, *q, *e, *tail;
  int insize, nmerges, psize, qsize, i;

  insize = 1;
  while(1) {
    p = list;
    list = NULL;
    tail = NULL;
    nmerges = 0;
    while(p) {
      nmerges++;
      q = p;
      psize = 0;
      for(i=0; i<insize; i++) {
        psize++;
        q = q->next;
        if(!q) break;
      }
      qsize = insize;
      while(psize > 0 || (qsize > 0 && q)) {
        if(psize == 0) {
          e = q; q = q->next; qsize--;
        } else if(qsize == 0 || !q) {
          e = p; p = p->next; psize--;
        } else if(filetree_cmp(p,q) <= 0) {
          e = p; p = p->next; psize--;
        } else {
          e = q; q = q->next; qsize--;
        }
        if(tail) tail->next = e;
        else     list = e;
        e->prev = tail;
        tail = e;
      }
      p = q;
    }
    tail->next = NULL;
    if(nmerges <= 1) {
      if(list->parent)
        list->parent->subs = list;
      return list;
    }
    insize *= 2;
  }
}

char *filetree_getpath(const struct dir_t *node)
{
    static char *path = NULL;
    static int pathLen = 0;

    const struct dir_t *d, **list;
    int c, i;

    if (node->parent == NULL)
        return "/";

    c = i = 1;
    for (d = node; d; d = d->parent)
    {
        i += strlen(d->name)+1;
        c++;
    }

    if (pathLen == 0)
    {
        path = malloc(i);
        pathLen = i;
    }
    else if (pathLen < i)
    {
        path = realloc(path, i);
        pathLen = i;
    }

    list = malloc(c * sizeof(struct dir_t *));
    for (d = node, c = 0; d; d = d->parent, ++c)
        list[c] = d;

    path[0] = '\0';
    while (c--)
    {
        strcat(path, list[c]->name);
        if (list[c]->parent && (list[c]->flags & NODE_ISDIR))
            strcat(path, "/");
    }
    free(list);
    return path;
}

struct dir_t *filetree_createRoot()
{
    struct dir_t *root = TYPE_MALLOC(struct dir_t);
    *root = (struct dir_t){
        .parent = NULL, .prev = NULL, .next = NULL, .subs = NULL, .name = (char *)"/",
        .flags = NODE_ISDIR, .realSize = 0, .compressSize = 0, .items = 0
    };
    return root;
}

char **filetree_getArr(struct dir_t **nodes, unsigned nodes_size)
{
    unsigned i;
    char **files = (char**)malloc(sizeof(char *) * (nodes_size + 1));
    for (i = 0; i < nodes_size; ++i)
        files[i] = strdup(filetree_getpath(nodes[i]) + 1);
    files[nodes_size] = NULL;
    return files;
}
