#ifndef C_MINILIB_CONFIG_CMC_FIELD_H
#define C_MINILIB_CONFIG_CMC_FIELD_H

#include "c_minilib_config.h"
#include "utils/cmc_error.h"

cmc_error_t cmc_field_create(const char *name,
                             const enum cmc_ConfigFieldTypeEnum type,
                             const void *default_value, const bool optional,
                             struct cmc_ConfigField **field);

cmc_error_t cmc_field_add_value(const void *value,
                                struct cmc_ConfigField *field);

void cmc_field_destroy(struct cmc_ConfigField **field);

#endif // C_MINILIB_CONFIG_CMC_FIELD_H
