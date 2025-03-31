#ifndef C_MINILIB_CONFIG_ERROR_H
#define C_MINILIB_CONFIG_ERROR_H

#include "c_minilib_config.h"

struct cmc_Error *cmc_error_create_with_fmt(int code, char *source_file,
                                            char *source_func, int source_line,
                                            char *fmt, ...);

void cmc_error_destroy(struct cmc_Error *);

#define CMC_ERRORF(code, fmt, ...)                                             \
  cmc_error_create_with_fmt((code), (char *)__FILE__, (char *)__func__,        \
                            __LINE__, (char *)(fmt), ##__VA_ARGS__)

#endif // C_MINILIB_CONFIG_ERROR_H
