#include "macho_retriever.h"
#include "macho_util.h"

#include <string.h>
#include <unistd.h>
#include "WinList.h"
#include <stdint.h>

int main(int argc, char *argv[]) {
  int extract = 0;
  int linker_options = 0;

  int c;
  while ((c = getopt(argc, argv, "el")) != -1) {
    switch (c) {
      case 'e':
        extract = 1;
        break;
      case 'l':
        linker_options = 1;
        break;
      default:
        fprintf(stderr, "Unknown option `-%c'.\n", optopt);
        exit(1);
    }
  }

  if(optind >= argc) {
    fprintf(stderr, "No file provided.\n");
  }

  char *filename = argv[optind];

  LIST_ENTRY bit_codes;
  InitializeListHead(&bit_codes);

  FILE *stream = fopen(filename, "rb");
  retrieve_bitcode(stream, &bit_codes);

  for(LIST_ENTRY* tmp = bit_codes.ForwardLink; tmp != &bit_codes; tmp = tmp->ForwardLink) {

      bitcode_data *item = CONTAINING_RECORD(tmp, bitcode_data, listEntry);
      char *file_name = write_to_file(item);
  }



//  for (int i = 0; i < archive_count; i++) {
//    if (archives[i]) {
//
//      if (extract) {
//        char *bitcode_files[1024];
//        int bitcode_count;
//        write_to_bitcode(archives[i], bitcode_files, &bitcode_count);
//        for (int j = 0; j < bitcode_count; j++) {
//          printf("%s\n", bitcode_files[j]);
//          free(bitcode_files[j]);
//        }
//      }
//
//      if(!extract || linker_options) {
//        char *file_name = write_to_file(archives[i]);
//        printf("%s\n", file_name);
//
//        if (linker_options) {
//          char *options[128];
//          int size = 0;
//          retrieve_linker_options(file_name, options, &size);
//          printf("Linker options: ");
//          for (int i = 0; i < size; i++) {
//            printf("%s ", options[i]);
//            free(options[i]);
//          }
//          printf("\n");
//        }
//
//        free(file_name);
//      }
//      free(archives[i]->buffer);
//      free(archives[i]);
//    }
//  }

  fclose(stream);
  return 0;
}
