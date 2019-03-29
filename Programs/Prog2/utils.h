#ifndef utils
#define utils

typedef char* keys_t;

typedef char* value_t;

typedef struct Node {
    keys_t key;
    value_t value;
} node_t;

typedef struct dict {
    node_t node[DICT_SIZE];
} dict_t;

size_t getDictLen(dict_t dict);

void blog(const char *fmt, ...);

#endif