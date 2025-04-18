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

enum cmc_LogLevelEnum {
  cmc_LogLevelEnum_ERROR,
  cmc_LogLevelEnum_WARNING,
  cmc_LogLevelEnum_INFO,
  cmc_LogLevelEnum_DEBUG,
};

struct cmc_ConfigSettings {
  char **supported_paths;
  uint32_t paths_length;
  char *name;
  void (*log_func)(enum cmc_LogLevelEnum log_level, char *msg);
};

enum cmc_ConfigFieldTypeEnum {
  cmc_ConfigFieldTypeEnum_NONE,
  cmc_ConfigFieldTypeEnum_STRING,
  cmc_ConfigFieldTypeEnum_INT,
  cmc_ConfigFieldTypeEnum_MAX,
};

struct cmc_ConfigField {
  char *name;
  void *value;
  bool optional;
  void *default_value;
  enum cmc_ConfigFieldTypeEnum type;
  void *next_field;
};

struct cmc_Config {
  struct cmc_ConfigField *fields;
  struct cmc_ConfigSettings *settings;
};

typedef struct cme_Error *cmc_error_t;

cmc_error_t cmc_lib_init(void);
cmc_error_t cmc_config_create(const struct cmc_ConfigSettings *settings,
                              struct cmc_Config **config);
void cmc_config_destroy(struct cmc_Config **config);
cmc_error_t cmc_config_add_field(const struct cmc_ConfigField *field,
                                 struct cmc_Config *config);
cmc_error_t cmc_config_parse(struct cmc_Config *config);

// `str` is already allocated memory which should be destroyed by calling
//     cmc_config_destroy.
cmc_error_t cmc_config_get_str(const char *name,
                               const struct cmc_Config *config, char **output);

cmc_error_t cmc_config_get_int(const char *name,
                               const struct cmc_Config *config, int *output);

static inline void cmc_error_destroy(cmc_error_t *error) {
  cme_error_destroy((struct cme_Error *)*error);
}

#endif // C_MINILIB_CONFIG_H
