#ifndef C_MINILIB_CONFIG_CMC_STRING_H
#define C_MINILIB_CONFIG_CMC_STRING_H

#include <c_minilib_config.h>
#include <string.h>

#define CMC_JOIN_PATH_STACK(varname, dir_path, config_name)                    \
  /* 1 for `/` and ` for null byte  */                                         \
  char varname[strlen(dir_path) + strlen(config_name) + 2];                    \
  sprintf(varname, "%s/%s", dir_path, config_name)

#endif // C_MINILIB_CONFIG_CMC_STRING_H
