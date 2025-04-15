#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "c_minilib_config.h"
/* #include "error/error.h" */

cmc_error_t cmc_lib_init(void) { return NULL; };

/* struct cmc_Error *config_create(const struct cmc_ConfigSettings *settings, */
/*                                 struct cmc_Config *config) { */
/*   if (!config || !settings) { */
/*     return CMC_ERRORF(EINVAL, "`config` and `settings` can't be NULL"); */
/*   } */

/*   config->settings = malloc(sizeof(struct cmc_ConfigSettings)); */
/*   if (!config->settings) { */
/*     return CMC_ERRORF(ENOMEM, */
/*                       "Can't allocate memory for `struct
 * cmc_ConfigSettings`"); */
/*   } */

/*   /\* memcpy(void *, const void *, unsigned long) *\/ */

/*   return NULL; */
/* }; */
