#ifndef C_MINILIB_CONFIG_H
#define C_MINILIB_CONFIG_H

#include <c_minilib_error.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
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
  uint32_t paths_length;
  char *name;
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

typedef struct cmx_Error *cmc_error_t;

cmc_error_t cmc_lib_init(void);
cmc_error_t cmc_config_create(const struct cmc_ConfigSettings *settings,
                              struct cmc_Config **config);
void cmc_config_destroy(struct cmc_Config **config);
cmc_error_t cmc_config_add_field(const struct cmc_ConfigField *field,
                                 struct cmc_Config *config);
cmc_error_t cmc_config_parse(struct cmc_Config *config);
cmc_error_t cmc_config_get_str(const char *name,
                               const struct cmc_Config *config, size_t n,
                               char buffer[n]);
cmc_error_t cmc_config_get_int(const char *name,
                               const struct cmc_Config *config, int *output);

inline void cmc_error_destroy(cmc_error_t *error) {
  cme_error_destroy((struct cme_Error *)*error);
}

#endif // C_MINILIB_CONFIG_H
