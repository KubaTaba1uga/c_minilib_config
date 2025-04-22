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

/******************************************************************************
 *                             General                                        *
 ******************************************************************************/
typedef struct cme_Error *cmc_error_t;
cmc_error_t cmc_lib_init(void);
cmc_error_t cmc_lib_destroy(void);

static inline void cmc_error_destroy(cmc_error_t *error) {
  cme_error_destroy((struct cme_Error *)*error);
}

/******************************************************************************
 *                             Field                                          *
 ******************************************************************************/
enum cmc_ConfigFieldTypeEnum {
  cmc_ConfigFieldTypeEnum_NONE,
  cmc_ConfigFieldTypeEnum_STRING,
  cmc_ConfigFieldTypeEnum_INT,
  cmc_ConfigFieldTypeEnum_ARRAY,
  cmc_ConfigFieldTypeEnum_DICT,
  cmc_ConfigFieldTypeEnum_MAX,
};

struct cmc_tree_node;
struct cmc_ConfigField {
  /* struct cmc_tree_node self; */
  char *name;
  void *value;
  bool optional;
  struct cmc_ConfigField *next_field;
  enum cmc_ConfigFieldTypeEnum type;
};

// This function allocates memory, there is no destruct because all
//  fields are freed on config destruct.
cmc_error_t cmc_field_create(const char *name,
                             const enum cmc_ConfigFieldTypeEnum type,
                             const void *default_value, const bool optional,
                             struct cmc_ConfigField **field);
cmc_error_t cmc_field_add_nested_field(struct cmc_ConfigField *field,
                                       struct cmc_ConfigField *child_field);
cmc_error_t cmc_field_add_next_field(struct cmc_ConfigField *field,
                                     struct cmc_ConfigField *next_field);
cmc_error_t cmc_field_get_str(const struct cmc_ConfigField *field,
                              char **output);
cmc_error_t cmc_field_get_int(const struct cmc_ConfigField *field, int *output);

/******************************************************************************
 *                             Config *
 ******************************************************************************/
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

struct cmc_Config {
  struct cmc_ConfigField *fields;
  struct cmc_ConfigSettings *settings;
};

// Config is basically collection for fields
cmc_error_t cmc_config_create(const struct cmc_ConfigSettings *settings,
                              struct cmc_Config **config);
// Add fields to config
cmc_error_t cmc_config_add_field(struct cmc_ConfigField *field,
                                 struct cmc_Config *config);
// Create values for fields
cmc_error_t cmc_config_parse(struct cmc_Config *config);
// Destroy all fields and their values
void cmc_config_destroy(struct cmc_Config **config);

#endif // C_MINILIB_CONFIG_H
