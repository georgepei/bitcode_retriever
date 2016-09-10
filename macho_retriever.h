#ifndef MACHO_RETRIEVER_H
#define MACHO_RETRIEVER_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include "WinList.h"

typedef struct _bitcode_data {
    LIST_ENTRY listEntry;
    uint64_t size;
    bool is_archive;
    char *buffer;
    const char *cpu;
    char *file_name;
}bitcode_data;

bitcode_data* make_bitcode(FILE* stream, const char* cpuname, const uint64_t offset, const uint64_t size, bool is_archive);


int max_number_of_archives();
int is_macho(FILE* stream);

void retrieve_bitcode(FILE *stream, LIST_ENTRY* bit_codes);

#ifdef __cplusplus
}
#endif
#endif
