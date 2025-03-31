#ifndef C_MINILIB_CONFIG_H
#define C_MINILIB_CONFIG_H

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

//
////
//////
/******************************************************************************

 There are copule convetions we are following:
   - all non atomic types are pointers.
   - input args are marked with const, we never modify input args.
   - output args are not marked with const, we always modify output args.

 ******************************************************************************/
//////////

/*
 Usage example:
    char *paths[] = { "abc", "def", NULL };
    struct cmc_ConfigSettings settings = {
        .supported_paths = paths,
        .name_override = NULL,
        .log_func = NULL,
    };
*/

struct cmc_ConfigSettings {
  char **supported_paths;
  char *name_override;
  void (*log_func)(int log_level, char *msg);
};

enum cmc_ConfigFieldTypeEnum { STRING, INT };

struct cmc_ConfigField {
  char *name;
  enum cmc_ConfigFieldTypeEnum type;
  void *value;
  void *default_value;
  bool optional;
};

struct cmc_Config {
  struct cmc_ConfigSettings *settings;
  struct cmc_ConfigField *fields;
};

struct cmc_Error {
  int code;
  char *msg;
  char *source_file;
  char *source_func;
  int source_line;
};

struct cmc_ConfigOps {
  struct cmc_Error *(*lib_init)(void);
  struct cmc_Error *(*config_create)(const struct cmc_ConfigSettings *settings,
                                     struct cmc_Config *config);
  struct cmc_Error *(*config_add_field)(const struct cmc_ConfigField *field,
                                        struct cmc_Config *config);
  struct cmc_Error *(*config_parse)(struct cmc_Config *config);
  struct cmc_Error *(*config_destroy)(struct cmc_Config *config);
  struct cmc_Error *(*config_get_str)(const char *name,
                                      const struct cmc_Config *config, size_t n,
                                      char buffer[n]);
  struct cmc_Error *(*config_get_int)(const char *name,
                                      const struct cmc_Config *config,
                                      int *output);
  void (*error_destroy)(struct cmc_Error *error);
};

#endif // C_MINILIB_CONFIG_H
