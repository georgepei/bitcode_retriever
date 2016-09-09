#ifndef MACHO_RETRIEVER_H
#define MACHO_RETRIEVER_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>

struct bitcode_data {
  uint64_t size;
    bool is_archive;
  char* buffer;
  const char* cpu;
};

struct bitcode_data* make_bitcode(FILE* stream, const char* cpuname, const uint64_t offset, const uint64_t size, bool is_archive);


struct bitcode_data* retrieve_bitcode_from_nonfat(FILE* stream, const uint32_t offset);

int max_number_of_archives();
int is_macho(FILE* stream);

void retrieve_bitcode(FILE* stream, struct bitcode_data* bitcodes[], int* count);

#ifdef __cplusplus
}
#endif
#endif
