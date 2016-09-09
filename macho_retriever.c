#include "macho_reader.h"
#include "macho_retriever.h"

#include <mach-o/fat.h>
#include <mach-o/loader.h>

#include <string.h>

int max_number_of_archives() { return get_cpu_type_count(); }

struct bitcode_data *make_bitcode(FILE *stream, const char *cpuname, const uint64_t offset, const uint64_t size, bool is_archive) {
  struct bitcode_data *bitcode = malloc(sizeof(struct bitcode_data));
    bitcode->is_archive = is_archive;
  bitcode->size = size;

  bitcode->buffer = malloc(sizeof(char) * size);
  fseek(stream, offset, SEEK_SET);
  fread(bitcode->buffer, sizeof(char), size, stream);

  bitcode->cpu = cpuname;
  return bitcode;
}


struct bitcode_data *retrieve_bitcode_from_nonfat(FILE *stream, const uint32_t offset) {
    uint32_t magic = get_magic(stream, offset);
    bool is64 = is_magic_64(magic);
    int swap_bytes = is_should_swap_bytes(magic);
    struct mach_header *header = load_mach_header(stream, offset, swap_bytes, is64);


    struct bitcode_data *bitcode = load_llvm_segment_command(stream, offset, header);
    free(header);
    return bitcode;
}

int is_macho(FILE *stream) {
  uint32_t magic = get_magic(stream, 0);
  return is_magic_macho(magic);
}

void retrieve_bitcode(FILE *stream, struct bitcode_data *bitcodes[], int *count) {
  uint32_t magic = get_magic(stream, 0);
  if (is_fat(magic)) {
    int swap_bytes = is_should_swap_bytes(magic);
    struct fat_header *header = load_fat_header(stream, swap_bytes);
    *count = header->nfat_arch;
    for (int i = 0; i < *count; i++) {
      uint32_t offset = offset_for_arch(stream, i, swap_bytes);
      bitcodes[i] = retrieve_bitcode_from_nonfat(stream, offset);
    }
    free(header);
  } else {
    bitcodes[0] = retrieve_bitcode_from_nonfat(stream, 0);
    *count = 1;
  }
}
