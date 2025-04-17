#ifndef C_MINILIB_CONFIG_CMC_STRING_H
#define C_MINILIB_CONFIG_CMC_STRING_H

#include <c_minilib_config.h>
#include <stdio.h>

#define CMC_JOIN_PATH_STACK(varname, dir_path, config_name)                    \
  /* 1 for `/` and ` for null byte  */                                         \
  char varname[strlen(dir_path) + strlen(config_name) + 2];                    \
  sprintf(varname, "%s/%s", dir_path, config_name)

#define CMC_JOIN_STR_STACK(varname, str_a, str_b)                              \
  char varname[strlen(str_a) + strlen(str_b) + 1];                             \
  sprintf(varname, "%s%s", str_a, str_b)

#endif // C_MINILIB_CONFIG_CMC_STRING_H
