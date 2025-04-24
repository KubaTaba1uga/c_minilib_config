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

 C minilib config is library designed for small to midsize configs.
 Supported config fields are:
  - integer
  - str
  - array
  - dict

 Supported config files formats are:
  - Environment variables

 Environment variables have to be represented in `key=value` syntax.
 For array we are using flat structure where array indexing is pushed into
 naming scheme. So for array like `a = [1,2,3]` we are doing:
 ```
 A_0=1
 A_1=2
 A_2=3
 ```
 Indexes are put inside keys so we are not requireing any custom list format and
  we support list and dicts nestings.

 There are copule convetions we are following in code:
   - input args are marked with const, we never modify input args.
   - output args are not marked with const, we always modify output args on func
 success.
   - on func error we are always returning cmc_error_t. On error output args
 should not be modified.
   - functions with create in name always allocates memory on success. It is
 regarding obj itself as well as obj attributes. Something created always needs
 to be destroyed.
   - functions with init in name always initializes data but do necessarly
 allocate anything. Init func will never allocate memory for obj itself, however
 it may allocate mamory intenrally or using obj attribute. Init func sometimes
 is paired with destroy func if any allocations indieed occur.

 ******************************************************************************/
//////////

/******************************************************************************
 *                             General                                        *
 ******************************************************************************/
typedef struct cme_Error *cmc_error_t;
cmc_error_t cmc_lib_init(void);
cmc_error_t cmc_lib_destroy(void);

static inline void cmc_error_destroy(cmc_error_t *error) {
  cme_error_destroy((struct cme_Error *)*error);
}

// We need tree to represent configuration. Each key value pair is represented
// internally as tree's node. Str or Int cannot have any leafs, while dict and
// list can have as many leafs as they want. Usually this tree is invisible to
// end user. If you need to traverse it just use one of `foreach` getter
// provided for dict and list.
struct cmc_TreeNode {
  struct cmc_TreeNode **subnodes;
  uint32_t subnodes_len;
};

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

struct cmc_ConfigField {
  char *name;
  void *value;
  bool optional;
  enum cmc_ConfigFieldTypeEnum type;
  struct cmc_TreeNode _self;
};

// This function allocates memory, there is no need for exposing field destroy
// to user
//   because all fields (and their values) are freed on config destroy.
cmc_error_t cmc_field_create(const char *name,
                             const enum cmc_ConfigFieldTypeEnum type,
                             const void *default_value, const bool optional,
                             struct cmc_ConfigField **field);
cmc_error_t cmc_field_add_subfield(struct cmc_ConfigField *field,
                                   struct cmc_ConfigField *child_field);
cmc_error_t cmc_field_get_str(const struct cmc_ConfigField *field,
                              char **output);
cmc_error_t cmc_field_get_int(const struct cmc_ConfigField *field, int *output);

/******************************************************************************
 *                             Config
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
  struct cmc_ConfigSettings *settings;
  struct cmc_TreeNode _fields;
};

// This function allocates memory, which needs to be destroyed.
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
