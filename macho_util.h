#ifndef MACHO_UTIL_H
#define MACHO_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <libxml/tree.h>
#include "macho_retriever.h"

char *write_to_file(bitcode_data *bitcode);



int get_options(xmlNode *option_parent, char *options[], int *size);
int get_linker_options(xmlNode *a_node, char *options[], int *size);
int retrieve_toc(const char *xar_path, const char *toc_path);
int retrieve_linker_options(const char *xar_path, char *options[], int *size);

#ifdef __cplusplus
}
#endif
#endif
