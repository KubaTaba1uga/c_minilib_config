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
   C MiniLib Config - Lightweight Configuration Parser for Embedded / CLI Apps.

   This library provides a small, flexible, and tree-based API for
   configuration parsing and validation. Designed for small to medium projects,
   it prioritizes clarity, low overhead, and simple integration.

   Supported field types:
     - Integer
     - String
     - Array (homogeneous)
     - Dictionary (key-value map)

   Supported input formats:
     - Environment-style `.env` files (key=value syntax)

   Arrays are flattened by encoding their indices into keys. For example:
       ARR=          → absent
       ARR_0=abc     → ARR = [abc]
       ARR_1=def     → ARR = [abc, def]
       ARR_2=ghi     → ARR = [abc, def, ghi]

   Nested arrays and dicts are supported via compound key naming:
       NESTED_0_KEY=value         → NESTED = [{KEY: value}]
       NESTED_1_KEY=value2        → NESTED = [{KEY: value}, {KEY: value2}]

   Coding conventions:
     - Input arguments are always marked `const` and never modified.
     - Output arguments are never `const`, and are only written on success.
     - All functions return a `cmc_error_t`. On error, outputs are undefined.
     - Functions prefixed with `create` allocate memory.
       These require `destroy`.
     - Functions prefixed with `init` initialize a struct but never allocate it.
       (They may allocate memory inside, depending on optional fields.)

 ******************************************************************************/
//////////

/******************************************************************************
 *                             General                                        *
 ******************************************************************************/

typedef struct cme_Error *cmc_error_t;

/**
 * Initialize the config library. Must be called before any parsing.
 */
cmc_error_t cmc_lib_init(void);

/**
 * Clean up global parser state. Call after all parsing is complete.
 */
cmc_error_t cmc_lib_destroy(void);

/**
 * Destroy a dynamically allocated error object.
 */
static inline void cmc_error_destroy(cmc_error_t *error) {
  cme_error_destroy((struct cme_Error *)*error);
}

/**
 * Internal representation of a node in the config tree.
 * Used for hierarchical organization of fields (esp. dicts and arrays).
 */
struct cmc_TreeNode {
  struct cmc_TreeNode **subnodes;
  uint32_t subnodes_len;
};

/**
 * Simple foreach macro for arrays.
 */
#define CMC_FOREACH(item, array, size)                                         \
  for (size_t _i = 0; _i < (size) && ((item) = (array)[_i], 1); ++_i)

/**
 * Iterate over the subnodes of a TreeNode.
 */
#define CMC_TREE_SUBNODES_FOREACH(var, node)                                   \
  struct cmc_TreeNode *var;                                                    \
  CMC_FOREACH(var, (node).subnodes, (node).subnodes_len)

/******************************************************************************
 *                             Field                                          *
 ******************************************************************************/

/**
 * Supported field types for configuration.
 */
enum cmc_ConfigFieldTypeEnum {
  cmc_ConfigFieldTypeEnum_NONE,
  cmc_ConfigFieldTypeEnum_STRING,
  cmc_ConfigFieldTypeEnum_INT,
  cmc_ConfigFieldTypeEnum_ARRAY,
  cmc_ConfigFieldTypeEnum_DICT,
  cmc_ConfigFieldTypeEnum_MAX,
};

/**
 * Represents a single configuration field.
 */
struct cmc_ConfigField {
  char *name;
  void *value;
  bool optional;
  enum cmc_ConfigFieldTypeEnum type;
  struct cmc_TreeNode _self;
};

/**
 * Create a new configuration field.
 * Allocates memory and stores optional default value.
 */
cmc_error_t cmc_field_create(const char *name,
                             const enum cmc_ConfigFieldTypeEnum type,
                             const void *default_value, const bool optional,
                             struct cmc_ConfigField **field);
/**
 * Add a subfield (child) to an array or dictionary field.
 * For arrays, the child's name is usually empty.
 * For dictionaries, the child must have a valid key name.
 */
cmc_error_t cmc_field_add_subfield(struct cmc_ConfigField *field,
                                   struct cmc_ConfigField *child_field);
/**
 * Get the parsed string value of a field.
 */
cmc_error_t cmc_field_get_str(const struct cmc_ConfigField *field,
                              char **output);
/**
 * Get the parsed integer value of a field.
 */
cmc_error_t cmc_field_get_int(const struct cmc_ConfigField *field, int *output);

/**
 * Convert a tree node pointer back to its containing field.
 */
struct cmc_ConfigField *cmc_field_of_node(struct cmc_TreeNode *node_ptr);

/**
 * Iterate over all subfields of a parent field (array/dict).
 */
#define CMC_FOREACH_FIELD(var, field, func)                                    \
  CMC_TREE_SUBNODES_FOREACH(__##var##subnode, (field)->_self) {                \
    struct cmc_ConfigField *(var) = cmc_field_of_node(__##var##subnode);       \
    func                                                                       \
  }

/**
 * Specialized iterator for array of scalar types.
 */
#define CMC_FOREACH_FIELD_ARRAY(var, type, field, func)                        \
  CMC_TREE_SUBNODES_FOREACH(__##var##subnode, (*field)->_self) {               \
    struct cmc_ConfigField *__##var##_subfield =                               \
        cmc_field_of_node(__##var##subnode);                                   \
    type var = __##var##_subfield->value;                                      \
    func                                                                       \
  }

/**
 * Specialized iterator for dictionary of scalar types.
 */
#define CMC_FOREACH_FIELD_DICT(var, type, field, func)                         \
  CMC_TREE_SUBNODES_FOREACH(__##var##subnode, (*field)->_self) {               \
    struct cmc_ConfigField *__##var##_subfield =                               \
        cmc_field_of_node(__##var##subnode);                                   \
    char *var##_name = __##var##_subfield->name;                               \
    type var = __##var##_subfield->value;                                      \
    func                                                                       \
  }

/******************************************************************************
 *                             Config
 ******************************************************************************/

/**
 * Logging verbosity levels.
 */
enum cmc_LogLevelEnum {
  cmc_LogLevelEnum_ERROR,
  cmc_LogLevelEnum_WARNING,
  cmc_LogLevelEnum_INFO,
  cmc_LogLevelEnum_DEBUG,
};

/**
 * Configuration system settings (parsing context).
 */
struct cmc_ConfigSettings {
  char **supported_paths;
  uint32_t paths_length;
  char *name;
  void (*log_func)(enum cmc_LogLevelEnum log_level, char *msg);
};

/**
 * Represents a complete configuration object.
 * Holds the defined schema and parsed values.
 */
struct cmc_Config {
  struct cmc_ConfigSettings *settings;
  struct cmc_TreeNode _fields;
};

/**
 * Create a new configuration context.
 * Allocates memory and copies the provided settings.
 */
cmc_error_t cmc_config_create(const struct cmc_ConfigSettings *settings,
                              struct cmc_Config **config);
/**
 * Add a top-level field to the configuration schema.
 */
cmc_error_t cmc_config_add_field(struct cmc_ConfigField *field,
                                 struct cmc_Config *config);
/**
 * Parse configuration from disk into fields values.
 * Automatically selects the best parser (e.g. `.env`).
 */
cmc_error_t cmc_config_parse(struct cmc_Config *config);

/**
 * Free all memory associated with the configuration object.
 */
void cmc_config_destroy(struct cmc_Config **config);

#endif // C_MINILIB_CONFIG_H
