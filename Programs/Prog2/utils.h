#ifndef utils
#define utils
#define DICT_SIZE 8

// typedef for dict key
typedef char* keys_t;

// typedef for dict value
typedef char* value_t;

// struct typedef for dict node
typedef struct Node {
    keys_t key;
    value_t value;
} node_t;

// typedef for dict
typedef node_t dict_t[];

// Generic log-to-stdout logging routine
// Message format: "timestamp:pid:user-defined-message"
void blog(const char *fmt, ...);

#endif