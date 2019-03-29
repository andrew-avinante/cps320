#ifndef utils
#define utils

typedef char* keys_t;

typedef char* value_t;

typedef struct Node {
    keys_t key;
    value_t value;
} node_t;

typedef node_t dict_t[];

void blog(const char *fmt, ...);

void getDictLen(dict_t dict);

#endif