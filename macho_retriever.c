#include "macho_reader.h"
#include "macho_retriever.h"

#include <mach-o/fat.h>
#include <mach-o/loader.h>
#include <time.h>

#include <string.h>

int max_number_of_archives() { return get_cpu_type_count(); }



bitcode_data *make_bitcode(FILE *stream, const char *cpuname, const uint64_t offset, const uint64_t size, bool is_archive) {
  bitcode_data *bitcode = malloc(sizeof(bitcode_data));
    bitcode->is_archive = is_archive;
  bitcode->size = size;

  bitcode->buffer = malloc(sizeof(char) * size);
  fseek(stream, offset, SEEK_SET);
  fread(bitcode->buffer, sizeof(char), size, stream);

  bitcode->cpu = cpuname;
  return bitcode;
}

bitcode_data *retrieve_bitcode_from_macho_file(FILE *stream, const uint32_t offset, const uint32_t size, LIST_ENTRY* bit_codes)
{
    uint32_t magic = read_uint32(stream, offset);
    bool is64 = is_magic_64(magic);
    int swap_bytes = is_should_swap_bytes(magic);
    struct mach_header *header = load_mach_header(stream, offset, swap_bytes, is64);

    bitcode_data * bitcode = load_llvm_segment_command(stream, offset, header);

    if(bitcode != NULL) {
        InsertHeadList(bit_codes, &bitcode->listEntry);
    }

    free(header);
    return bitcode;
}

void retrieve_bitcode_from_nonfat(FILE *stream, const uint32_t offset, const uint32_t size, LIST_ENTRY* bit_codes) {
    uint32_t magic = read_uint32(stream, offset);
    if (is_magic_macho(magic)) {
        retrieve_bitcode_from_macho_file(stream, offset, size, bit_codes);
    } else{
        uint64_t magic_static_library = 0;
        read_data(stream, offset, (uint8_t *)&magic_static_library, sizeof(magic_static_library));
        if(magic_static_library == *(uint64_t*)"!<arch>\n"){
            archive_header_line* header = get_archive_header_line(stream, offset + sizeof(magic_static_library));

            uint32_t header_fixsize = (uint8_t *)&header->long_name - (uint8_t *)header;

            for(uint32_t tmp_offset = offset+sizeof(magic_static_library) + header_fixsize + atoi((const char *)header->file_size); tmp_offset < size+offset;){
                archive_header_line* header_object = get_archive_header_line(stream, tmp_offset);
                printf("    object: %s\n", header_object->long_name);

                bitcode_data* bitcode = retrieve_bitcode_from_macho_file(
                        stream, tmp_offset+header_object->header_size,
                        atoi((const char *)header_object->file_size)-(header_object->header_size-header_fixsize), bit_codes);

                if(bitcode != NULL) {
                    if (header_object->long_name != NULL) {
                        bitcode->file_name = strdup((const char *) header_object->long_name);
                    } else {
                        bitcode->file_name = strdup((const char *) header_object->name);
                    }
                } else{
                    printf("object file %s have no bitcode data\n", header_object->long_name==NULL?header_object->name:header_object->long_name);
                }

                tmp_offset += atoi((const char *)header_object->file_size) + header_fixsize;

                free_archive_header_line(header_object);
            }
            free_archive_header_line(header);
        } else{
            printf("error: unexpected file!!\n");
        }
    }
    return;
}

void retrieve_bitcode(FILE *stream, LIST_ENTRY* bit_codes) {
  uint32_t magic = read_uint32(stream, 0);
  if (is_fat(magic)) {
    int swap_bytes = is_should_swap_bytes(magic);
    struct fat_header *header = load_fat_header(stream, swap_bytes);

    for (int i = 0; i < header->nfat_arch; i++) {
        int offset = sizeof(struct fat_header) + sizeof(struct fat_arch) * i;
        struct fat_arch *arch = load_fat_arch(stream, offset, swap_bytes);
        retrieve_bitcode_from_nonfat(stream, arch->offset, arch->size, bit_codes);
        free(arch);
    }
    free(header);
  } else {
    retrieve_bitcode_from_nonfat(stream, 0, 0, bit_codes);
  }
}
