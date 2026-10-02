#include <ctype.h>       // for isdigit
#include <fcntl.h>       // for open, O_RDONLY
#include <stdint.h>      // for uint8_t
#include <stdio.h>       // for perror, printf, size_t, snprintf, NULL
#include <stdlib.h>      // for exit, strtol
#include <string.h>      // for memchr, memcpy, strncpy
#include <sys/mman.h>    // for mmap, munmap, MAP_FAILED, MAP_PRIVATE, PROT_...
#include <sys/random.h>  // for getrandom
#include <sys/stat.h>    // for fstat, stat
#include <unistd.h>      // for close

#define WORDLIST_PATH "/opt/eff_large_wordlist.txt"
#define MAX_WORDS 1000
#define WORD_BUFFER 32 // max word length + safety

// small RNG buffer for efficiency
uint8_t buf[64];
size_t idx = sizeof(buf);
uint8_t next_byte() {
    if (idx >= sizeof(buf)) {
        if (getrandom(buf, sizeof(buf), 0) != sizeof(buf)) {
            for (int i = 0; i < 5; i++) {
                if (getrandom(buf, sizeof(buf), 0) == sizeof(buf)) 
                    break;
            }
            perror("getrandom");
            exit(1);
        }
        idx = 0;
    }
    return buf[idx++];
}

// convert dice number string (e.g., "12345") to int
int parse_dice_number(const char *str) {
    int n = 0;
    for (int i = 0; i < 5; i++) {
        if (!isdigit((unsigned char)str[i])) return -1;
        n = n * 10 + (str[i] - '0');
    }
    return n;
}

// generate a random dice number 11111-66666
int random_dice() {
    int num = 0;
    for (int i = 0; i < 5; i++) {
        uint8_t b;
        do {
            b = next_byte(); //remove modulo bias
        } while (b >= 252); // 252 is largest multiple of 6 < 256
        int roll = (b % 6) + 1;
        num = num * 10 + roll;
    }
    return num;
}

// lookup dice number in mmaped file
void lookup_word(char *mmap_start, size_t mmap_size, int key, char *out_word) {
    char *ptr = mmap_start;
    char *end = mmap_start + mmap_size;
    char line[WORD_BUFFER];
    while (ptr < end) {
        char *line_end = memchr(ptr, '\n', end - ptr);
        if (!line_end) line_end = end;
        size_t len = line_end - ptr;
        if (len >= sizeof(line)) len = sizeof(line) - 1;
        memcpy(line, ptr, len);
        line[len] = '\0';

        if (len >= 6) { // at least 5 digits + tab
            if (line[5] == '\t') {
                int line_key = parse_dice_number(line);
                if (line_key == key) {
                    strncpy(out_word, line + 6, WORD_BUFFER - 1);
                    out_word[WORD_BUFFER - 1] = '\0';
                    return;
                }
            }
        }
        ptr = line_end + 1;
    }
    snprintf(out_word, WORD_BUFFER, "KEY_%d_NOT_FOUND", key);
}

int main(int argc, char *argv[]) {
    int wordCount = 5;
    char *separator = " ";

    if (argc >= 2) {
        char *c = argv[1];
        int is_numeric = 1;
        for (int i = 0; c[i] != '\0'; i++)
            if (!isdigit((unsigned char)c[i])) is_numeric = 0;
        if (is_numeric) {
            char *end;
            long wc = strtol(c, &end, 10);
            if (*end == '\0' && wc > 0 && wc <= MAX_WORDS)
                wordCount = wc;
        }
    }

    if (argc >= 3) {
        char *c = argv[2];
        separator = c;
    }

    int fd = open(WORDLIST_PATH, O_RDONLY);
    if (fd < 0) { perror("open"); return 1; }
    struct stat st;
    if (fstat(fd, &st) < 0) { perror("fstat"); close(fd); return 1; }
    size_t filesize = st.st_size;

    char *map = mmap(NULL, filesize, PROT_READ, MAP_PRIVATE, fd, 0);
    if (map == MAP_FAILED) { perror("mmap"); close(fd); return 1; }
    close(fd);

    char word[WORD_BUFFER];
    for (int i = 0; i < wordCount; i++) {
        if ((wordCount > 1 && i == wordCount - 1) || wordCount == 1) {separator = "";}

        int dice = random_dice();
        lookup_word(map, filesize, dice, word);
        printf("%s%s", word, separator);
    }
    printf("\n");

    munmap(map, filesize);
    return 0;
}
