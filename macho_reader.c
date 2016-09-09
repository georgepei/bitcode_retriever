#include "macho_reader.h"
#include "macho_retriever.h"

#include <stdlib.h>
#include <string.h>

#include <mach-o/loader.h>
#include <mach-o/fat.h>
#include <mach-o/swap.h>
#include <mach/machine.h>

const static int uint32_size = sizeof(uint32_t);

const static int fat_header_size = sizeof(struct fat_header);
const static int fat_arch_size = sizeof(struct fat_arch);

struct _cpu_type_names {
  cpu_type_t cputype;
  const char *cpu_name;
};

static struct _cpu_type_names cpu_type_names[] = {
  { CPU_TYPE_I386, "i386" },
  { CPU_TYPE_X86_64, "x86_64" },
  { CPU_TYPE_ARM, "arm" },
  { CPU_TYPE_ARM64, "arm64" }
};

int get_cpu_type_count() {
  return (int)(sizeof(cpu_type_names) / sizeof(cpu_type_names[0]));
}

static const char *cpu_type_name(cpu_type_t cpu_type) {
  static int cpu_type_names_size = sizeof(cpu_type_names) / sizeof(struct _cpu_type_names);
  for (int i = 0; i < cpu_type_names_size; i++ ) {
    if (cpu_type == cpu_type_names[i].cputype) {
      return cpu_type_names[i].cpu_name;
    }
  }

  return "unknown";
}

uint32_t get_magic(FILE *stream, int offset) {
  uint32_t magic = 0;
  fseek(stream, offset, SEEK_SET);
  fread(&magic, uint32_size, 1, stream);
  rewind(stream);
  return magic;
}

int is_magic_macho(const uint32_t magic) {
  return magic == MH_MAGIC_64
      || magic == MH_CIGAM_64
      || magic == MH_MAGIC
      || magic == MH_CIGAM
      || magic == FAT_MAGIC
      || magic == FAT_CIGAM;
}

bool is_magic_64(const uint32_t magic) {
  return magic == MH_MAGIC_64 || magic == MH_CIGAM_64;
}

int is_should_swap_bytes(const uint32_t magic) {
  return magic == MH_CIGAM || magic == MH_CIGAM_64 || magic == FAT_CIGAM;
}

int is_fat(const uint32_t magic) {
  return magic == FAT_MAGIC || magic == FAT_CIGAM;
}

struct fat_header *load_fat_header(FILE *stream, const int swap_bytes) {
  struct fat_header *header = malloc(fat_header_size);
  fread(header, fat_header_size, 1, stream);
  rewind(stream);

  if (swap_bytes) {
    swap_fat_header(header, 0);
  }

  return header;
}

struct fat_arch *load_fat_arch(FILE *stream, const int offset, const int swap_bytes) {
  struct fat_arch *arch = malloc(fat_arch_size);
  fseek(stream, offset, SEEK_SET);
  fread(arch, fat_arch_size, 1, stream);
  rewind(stream);

  if (swap_bytes) {
    swap_fat_arch(arch, 1, 0);
  }

  return arch;
}

uint32_t offset_for_arch(FILE *stream, const int index, const int swap_bytes) {
  int offset = fat_header_size + fat_arch_size * index;
  struct fat_arch *arch = load_fat_arch(stream, offset, swap_bytes);
  uint32_t arch_offset = arch->offset;
  free(arch);
  return arch_offset;
}

struct mach_header *load_mach_header(FILE *stream, const int offset, const int swap_bytes, bool is64bit) {
    uint32_t machohead_size = 0;
    if (is64bit) {
        machohead_size = sizeof(struct mach_header_64);
    } else {
        machohead_size = sizeof(struct mach_header);
    }

    struct mach_header *header = malloc(machohead_size);
    fseek(stream, offset, SEEK_SET);
    fread(header, machohead_size, 1, stream);
    rewind(stream);

    if (swap_bytes) {
        if (is64bit) {
            swap_mach_header_64(header, 0);
        } else {
            swap_mach_header(header, 0);
        }
    }
    return header;
}

struct load_command *load_load_command(FILE *stream, const int offset, const int swap_bytes) {
  struct load_command *command = malloc(sizeof(struct load_command));
  fseek(stream, offset, SEEK_SET);
  fread(command, sizeof(struct load_command), 1, stream);
  rewind(stream);

  if (swap_bytes) {
    swap_load_command(command, 0);
  }

  return command;
}

struct segment_command *load_segment_command(FILE *stream, const int offset, struct mach_header *header) {
    int swap_bytes = is_should_swap_bytes(header->magic);
    int is64bit = is_magic_64(header->magic);

    uint32_t segment_command_size = 0;
    if(is64bit){
        segment_command_size = sizeof(struct segment_command_64);
    } else{
        segment_command_size = sizeof(struct segment_command);
    }

  struct segment_command *command = malloc(segment_command_size);
  fseek(stream, offset, SEEK_SET);
  fread(command, segment_command_size, 1, stream);
  rewind(stream);

  if (swap_bytes) {
      if(is64bit) {
          swap_segment_command_64(command, 0);
      } else{
          swap_segment_command(command, 0);
      }
  }

  return command;
}

struct bitcode_data* find_bitcode_section(FILE *stream, struct mach_header *header, const int offset, const int swap_bytes, struct segment_command_64* cmd) {
    bool is64bit = is_magic_64(header->magic);
    uint32_t section_size = 0;
    uint32_t machohead_size = 0;
    uint32_t segment_command_size = 0;
    if(is64bit){
        section_size = sizeof(struct section_64);
        machohead_size = sizeof(struct mach_header_64);
        segment_command_size = sizeof(struct segment_command_64);
    } else{
        section_size = sizeof(struct section);
        machohead_size = sizeof(struct mach_header);
        segment_command_size = sizeof(struct segment_command);
    }

    for(int i=0; i< cmd->nsects; i++){
        struct section* section = calloc(section_size, 1);

        fseek(stream, offset + machohead_size + segment_command_size + i*section_size, SEEK_SET);
        fread(section, section_size, 1, stream);
        if(swap_bytes) {
            if(is64bit) {
                swap_section_64((struct section_64*)section, 1, 0);
            } else{
                swap_section((struct section*)section, 1, 0);
            }
        }

        printf("sgement name: %s, section name: %s\n", section->segname, section->sectname);
        if(strcmp(section->sectname, "__bitcode") == 0 && strcmp(section->segname, "__LLVM") == 0){
            //found bitcode
            const char *cpu_name = cpu_type_name(header->cputype);
            int32_t section_offset = 0;
            int32_t section_size = 0;
            if(is64bit){
                section_offset = ((struct section_64*)section)->offset;
                section_size = ((struct section_64*)section)->size;
            } else{
                section_offset = ((struct section*)section)->offset;
                section_size = ((struct section*)section)->size;
            }

            struct bitcode_data* bitcode = make_bitcode(stream, cpu_name, offset + section_offset, section_size, false);

            return bitcode;
        }
    }
    return NULL;
}

struct bitcode_data *load_llvm_segment_command(FILE *stream, const int offset, struct mach_header *header) {
    int swap_bytes = is_should_swap_bytes(header->magic);
    int is64bit = is_magic_64(header->magic);

    int cmd_offset = 0;
    uint32_t load_command_size = 0;
    uint32_t filetype = 0;
    uint32_t cpu_type = 0;
    if(is64bit){
        cmd_offset = offset + sizeof(struct mach_header_64);
        load_command_size = ((struct mach_header_64 *)header)->ncmds;
        filetype = ((struct mach_header_64 *)header)->filetype;
        cpu_type = ((struct mach_header_64 *)header)->cputype;
    } else{
        cmd_offset = offset + sizeof(struct mach_header);
        load_command_size = header->ncmds;
        filetype = header->filetype;
        cpu_type = header->cputype;
    }

    struct bitcode_data *bitcode = NULL;

    for (int i = 0; i < load_command_size; i++) {
        struct load_command *cmd = load_load_command(stream, cmd_offset, swap_bytes);
        if (cmd->cmd == LC_SEGMENT_64 || cmd == LC_SEGMENT) {
            struct segment_command *segment = load_segment_command(stream, cmd_offset, header);
            if (filetype == MH_OBJECT) {
                bitcode = find_bitcode_section(stream, header, offset, swap_bytes, segment);

            } else {
                if (!strncmp("__LLVM", segment->segname, 7)) {
                    const char *cpu_name = cpu_type_name(cpu_type);
                    bitcode = make_bitcode(stream, cpu_name, offset + segment->fileoff, segment->filesize, true);
                }
            }
            free(segment);
        }
        cmd_offset += cmd->cmdsize;
        free(cmd);

        if(bitcode != NULL){
            break;
        }
    }

    return bitcode;
}
