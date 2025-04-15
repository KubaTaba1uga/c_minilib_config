#ifndef C_MINILIB_CONFIG_CMC_ERROR_H
#define C_MINILIB_CONFIG_CMC_ERROR_H

#include <c_minilib_error.h>

#define cmc_errorf(...) (cmc_error_t) cme_errorf(__VA_ARGS__)

#endif // C_MINILIB_CONFIG_CMC_ERROR_H
