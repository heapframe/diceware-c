#include <ctype.h>       // for isdigit
#include <stdint.h>      // for uint8_t
#include <stdio.h>       // for printf, perror, NULL, fclose, fopen, fscanf
#include <stdlib.h>      // for exit, free, malloc, strtol
#include <string.h>      // for strncpy
#include <sys/random.h>  // for getrandom
#include "uthash.h"      // for HASH_ADD_INT, HASH_DEL, HASH_FIND_INT, HASH_...

#define WORDLIST_PATH "/opt/eff_large_wordlist.txt"
#define MAX_WORDS 1000

// uthash code isnt mine
typedef struct {
    int key;
    char value[16];
    UT_hash_handle hh; // makes this structure hashable
} MapEntry;

MapEntry *map = NULL;

void insert_entry(int key, const char *value) {
    MapEntry *entry = malloc(sizeof(*entry));
    if (!entry) { perror("malloc"); exit(1); }
    entry->key = key;
    strncpy(entry->value, value, sizeof(entry->value) - 1);
    entry->value[sizeof(entry->value) - 1] = '\0';
    HASH_ADD_INT(map, key, entry);
}

const char *lookup_entry(int key) {
    MapEntry *entry;
    HASH_FIND_INT(map, &key, entry);
    return entry ? entry->value : NULL;
}

void free_map(void) {
    MapEntry *cur, *tmp;
    HASH_ITER(hh, map, cur, tmp) {
        HASH_DEL(map, cur);
        free(cur);
    }
}

uint8_t buf[64];
size_t idx = sizeof(buf);
uint8_t next_byte() {
    if (idx >= sizeof(buf)) {
        if (getrandom(buf, sizeof(buf), 0) != sizeof(buf)) {
            perror("getrandom");
            exit(1);
        }
        idx = 0;
    }
    return buf[idx++];
}

int main(int argc, char *argv[]) {
    FILE *fp = fopen(WORDLIST_PATH, "r"); //from https://www.eff.org/files/2016/07/18/eff_large_wordlist.txt

    if (!fp) {
        printf("Cant open %s\n", WORDLIST_PATH);
        perror("fopen");
        return 1;
    }

    int key;
    char value[64];
    while (fscanf(fp, "%d %63s", &key, value) == 2) {
        insert_entry(key, value);
    }
    fclose(fp);

    int wordCount = 5;
    if (argc >= 2) {
        char *c = argv[1];
        int is_numeric = 1;
        for (int i = 0; c[i] != '\0'; i++) {
            if (!isdigit((unsigned char)c[i])) {
                is_numeric = 0;
                break;
            }
        }
        if (is_numeric) {
            char *end;
            long wc = strtol(c, &end, 10);
            if (*end == '\0' && wc > 0 && wc < MAX_WORDS)
                wordCount = wc;
        }
    }

    for (int i = 0; i < wordCount; i++) {
        int word = 0;
        for (int j = 0; j < 5; j++) {
            int roll = (next_byte() % 6) + 1;
            word = (word * 10) + roll;
        }
        const char *result = lookup_entry(word);
        if (result)
            printf("%s ", result);
        else
            printf("KEY_%d_NOT_FOUND ", word);
    }
    printf("\n");
    free_map();
    return 0;
}