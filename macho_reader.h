#ifndef MACHO_READER_H
#define MACHO_READER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "macho_retriever.h"

struct mach_header;
struct mach_header_64;
struct fat_header;

typedef struct _archive_header_line{
    uint8_t name[16];
    uint8_t modified_time[12];
    uint8_t userid[6];
    uint8_t groupid[6];
    uint8_t mode[8];
    uint8_t file_size[8];
    uint8_t end_mark[4];
    char* long_name;
    uint32_t header_size;//head line fix size + Long Name

}__attribute__((aligned(1),packed))  archive_header_line;

archive_header_line* get_archive_header_line(FILE *stream, uint32_t offset);
void free_archive_header_line(archive_header_line*  header);

int get_cpu_type_count();

uint32_t read_uint32(FILE *stream, const int offset);
bool read_data(FILE* stream, int offset, uint8_t* buffer, uint32_t buffersize);

bool is_magic_macho(const uint32_t magic);
bool is_magic_64(const uint32_t magic);
bool is_fat(const uint32_t magic);
bool is_should_swap_bytes(const uint32_t magic);

struct fat_arch *load_fat_arch(FILE *stream, const int offset, const int swap_bytes);

struct fat_header *load_fat_header(FILE *stream, const int swap_bytes);

struct mach_header *load_mach_header(FILE *stream, const int offset, const int swap_bytes, bool is64bit);

bitcode_data *load_llvm_segment_command(FILE *stream, const int offset,
                                               struct mach_header *header);

#ifdef __cplusplus
}
#endif
#endif
