#ifndef MACHO_READER_H
#define MACHO_READER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

struct mach_header;
struct mach_header_64;
struct fat_header;

int get_cpu_type_count();

uint32_t get_magic(FILE *stream, const int offset);

int is_magic_macho(const uint32_t magic);
bool is_magic_64(const uint32_t magic);
int is_fat(const uint32_t magic);
int is_should_swap_bytes(const uint32_t magic);

struct fat_header *load_fat_header(FILE *stream, const int swap_bytes);
uint32_t offset_for_arch(FILE *stream, const int index, const int swap_bytes);

struct mach_header *load_mach_header(FILE *stream, const int offset, const int swap_bytes, bool is64bit);

struct bitcode_data *load_llvm_segment_command(FILE *stream, const int offset,
                                               struct mach_header *header);

#ifdef __cplusplus
}
#endif
#endif
