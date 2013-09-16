#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sparse.h"

struct record {
    int r, c;
};

#define CHUNK_SIZE 1024

int
main(int argc, char *argv[]) {
    bsc_hist_t *H;
    int c, r;
    struct record *data= NULL;
    int *col;
    size_t nalloc, nread;

    H= bsc_hist_new();

    printf("Building histogram and caching data...");
    fflush(stdout);

    nalloc= 0;
    nread= 0;
    while(scanf("%d %d\n", &r, &c) == 2) {
        bsc_hist_count(H, c, r, 1);

        if(nalloc <= nread) {
            nalloc+= CHUNK_SIZE;
            data= realloc(data, nalloc * sizeof(struct record));
            if(!data) {
                perror("realloc");
                exit(EXIT_FAILURE);
            }
        }

        data[nread].r= r;
        data[nread].c= c;

        nread++;
    }

    printf(" done.\n");

    bsc_stats(H);

    col= malloc(H->end_row * sizeof(int));
    if(!col) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    printf("Recounting all entries...\n");
    fflush(stdout);

    for(c= 0; c < H->end_col; c++) {
        size_t i;
        if(c != 0 && c % 10 == 0)
            printf("%d/%d\n", c, H->end_col);
        bzero(col, H->end_row * sizeof(int));
        if(H->end_rows[c] <= H->start_rows[c]) continue;
        for(i= 0; i < nread; i++) {
            if(data[i].c == c) col[data[i].r]++;
        }
        for(r= H->start_rows[c]; r < H->end_rows[c]; r++) {
            int ri= r - H->start_rows[c];

            if(col[r] != H->entries[c][ri]) {
                printf("Mismatch at c=%d r=%d, %d vs %d\n",
                        c, r, col[r], H->entries[c][ri]);
                abort();
            }
        }
    }

    printf(" done.\n");

    bsc_hist_destroy(H);
    free(data);

    return 0;
}