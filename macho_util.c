#include "macho_reader.h"
#include "macho_retriever.h"
#include "macho_util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xar/xar.h>

#include <libxml/parser.h>

char *construct_file_name(const char *arch, const char* file_name, const char *ext) {
  const char *delimiter = ".";
  int length = strlen(arch) + strlen(delimiter) + strlen(ext) + 1;
    if(file_name != NULL){
        length += strlen(file_name);
        length += strlen(delimiter);
    }
  char *new_file_name = calloc(sizeof(char), length);
  strcpy(new_file_name, arch);
  strcat(new_file_name, delimiter);
  if(file_name != NULL){
    strcat(new_file_name, file_name);
    strcat(new_file_name, delimiter);
  }
  strcat(new_file_name, ext);
  return new_file_name;
}

char *write_to_file(bitcode_data *bitcode) {
    char* suffix = NULL;
    if(bitcode->is_archive){
        suffix = "xar";
    } else{
        suffix = "bc";
    }
  char *filename = construct_file_name(bitcode->cpu, bitcode->file_name, suffix);
  FILE *output = fopen(filename, "wb");

  if (!output) {
    fprintf(stderr, "Cannot open '%s' for writing\n", filename);
    free(filename);
    return NULL;
  }

  fwrite(bitcode->buffer, sizeof(char), bitcode->size, output);
  fclose(output);

  return filename;
}
//
//int extract_xar(const char *path, const char *cpu, char *files[], int *count) {
//  xar_t x;
//  xar_iter_t xi;
//  xar_file_t xf;
//  xar_stream xs;
//  char buffer[8192];
//
//  x = xar_open(path, READ);
//  if (!x) {
//    fprintf(stderr, "Error opening archive\n");
//    return 1;
//  }
//
//  xi = xar_iter_new();
//  if (!xi) {
//    fprintf(stderr, "Error creating xar iterator\n");
//    return 2;
//  }
//
//  *count = 0;
//  for (xf = xar_file_first(x, xi); xf; xf = xar_file_next(xi)) {
//    char *path = xar_get_path(xf);
//    const char *type;
//    xar_prop_get(xf, "type", &type);
//    if (!type) {
//      fprintf(stderr, "File has no type %s\n", path);
//      free(path);
//      continue;
//    }
//    if (strcmp(type, "file") != 0) {
//      fprintf(stderr, "Skipping %s\n", path);
//      free(path);
//      continue;
//    }
//
//    if (xar_extract_tostream_init(x, xf, &xs) != XAR_STREAM_OK) {
//      fprintf(stderr, "Error initializing stream %s\n", path);
//      free(path);
//      continue;
//    }
//
//    char *prefix = construct_file_name(cpu, path);
//    char *output_path = construct_file_name(prefix, "bc");
//    free(path);
//    free(prefix);
//
//    FILE *output = fopen(output_path, "wb");
//    if (!output) {
//      fprintf(stderr, "Error opening output file %s\n", output_path);
//      free(output_path);
//      continue;
//    }
//
//    xs.avail_out = sizeof(buffer);
//    xs.next_out = buffer;
//
//    int32_t ret;
//    while ((ret = xar_extract_tostream(&xs)) != XAR_STREAM_END) {
//      if (ret == XAR_STREAM_ERR) {
//        fprintf(stderr, "Error extracting stream %s\n", output_path);
//        free(output_path);
//        return 3;
//      }
//
//      fwrite(buffer, sizeof(char), sizeof(buffer) - xs.avail_out, output);
//
//      xs.avail_out = sizeof(buffer);
//      xs.next_out = buffer;
//    }
//
//    if (xar_extract_tostream_end(&xs) != XAR_STREAM_OK) {
//      fprintf(stderr, "Error ending stream %s\n", output_path);
//    }
//
//    fclose(output);
//    files[(*count)++] = output_path;
//  }
//  return 0;
//}

//int write_to_bitcode(bitcode_data *bitcode, char *files[], int *count) {
//    char* file_name = write_to_file(bitcode);
//    if(!bitcode->is_archive){
//        printf("bit code is in byte code format not in archive, so need not to extract it");
//        return 0;
//    }
//
//  int extracted = extract_xar(file_name, bitcode->cpu, files, count);
//  if (extracted != 0) {
//    fprintf(stderr, "Error extracting xar file %s\n", file_name);
//    free(file_name);
//    return 1;
//  }
//
//  int removed = remove(file_name);
//  if (removed != 0) {
//    fprintf(stderr, "Error removing xar file %s\n", file_name);
//    free(file_name);
//    return 2;
//  }
//
//  free(file_name);
//  return 0;
//}

int get_options(xmlNode *option_parent, char *options[], int *size) {
  xmlNode *cur_node = NULL;
  *size = 0;
  for (cur_node = option_parent; cur_node; cur_node = cur_node->next) {
    if (cur_node->type == XML_ELEMENT_NODE && strcmp((const char *)cur_node->name, "option") == 0) {
      char *content = (char *)xmlNodeGetContent(cur_node);
      options[(*size)++] = content;
    }
  }
  return 0;
}

int get_linker_options(xmlNode *a_node, char *options[], int *size) {
  xmlNode *cur_node = NULL;
  for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
    if (cur_node->type == XML_ELEMENT_NODE) {
      if (strcmp((const char *)cur_node->name, "link-options") == 0) {
        return get_options(cur_node->children, options, size);
      } else {
        get_linker_options(cur_node->children, options, size);
      }
    }
  }
  return 1;
}

int retrieve_toc(const char *xar_path, const char *toc_path) {
  xar_t x;
  x = xar_open(xar_path, READ);
  if (!x) {
    fprintf(stderr, "Error opening xar archive %s\n", xar_path);
    return 1;
  }
  xar_serialize(x, toc_path);
  return 0;
}

int retrieve_linker_options(const char *xar_path, char *options[], int *size) {
  const char *toc_file = "toc.temp";
  retrieve_toc(xar_path, toc_file);

  xmlDoc *doc = NULL;
  doc = xmlReadFile(toc_file, NULL, 0);
  if (doc == NULL) {
    fprintf(stderr, "Cannot parse TOC %s\n", toc_file);
    remove(toc_file);
    return 1;
  }

  get_linker_options(xmlDocGetRootElement(doc), options, size);

  xmlFreeDoc(doc);
  xmlCleanupParser();

  remove(toc_file);

  return 0;
}
