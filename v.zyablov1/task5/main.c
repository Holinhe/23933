#include <stdio.h>
#include <stdlib.h>
#include <errno.h>


typedef struct row {
    unsigned int pos;
    unsigned int len;
} row;


int main(int argc, char *argv[]) {
    long val;
    if (argc == 1) {
        perror("run without arguments");
        return 0;
    }

    FILE *file;
    file = fopen(argv[1], "r");

    if (file == NULL) {
        perror("can't open file");
        return 0;
    }

    // get count of strings in file
    char ch;
    unsigned int str_count = 1;

    while ((ch = getc(file)) != EOF) {
        if (ch == '\n') {
            ++str_count;
        }
    }

    row *table = malloc(sizeof(row) * str_count);

    // fill the table of offsets and lengths
    unsigned int cur_pos = 0;
    unsigned int cur = 0;
    unsigned int cur_str_len = 0;

    fseek(file, 0, 0);
    while ((ch = getc(file)) != EOF) {
        if (ch == '\n') {
            table[cur++].len = cur_str_len;
            table[cur].pos = cur_pos + 1;

            cur_str_len = 0;
        } else {
            ++cur_str_len;
        }

        ++cur_pos;
    }
    table[cur].len = cur_pos - table[cur].pos;

    printf("Enter string number: ");

    char buff[64];
    while (scanf("%63s", buff) != 0) {
        char *endptr;
        errno = 0;
        val = strtol(buff, &endptr, 10);

        if (((errno == ERANGE || (*endptr != '\0'))) || val < 0 || val > str_count) {
            perror("string number out of range\n");
            printf("\nEnter string number: ");
            continue;
        }
        if (val == 0) { break; }

        fseek(file, table[val - 1].pos, 0);

        for (int i = 0; i < table[val - 1].len; ++i) {
            putc(getc(file), stdout);
        }
        printf("\nEnter string number: ");
    }
    putc('\n', stdout);

    fclose(file);
    free(table);
    return 0;
}
