#ifndef C_MINILIB_CONFIG_CMC_STRING_H
#define C_MINILIB_CONFIG_CMC_STRING_H

#include <c_minilib_config.h>
#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CMC_JOIN_PATH_STACK(varname, dir_path, config_name)                    \
  /* 1 for `/` and ` for null byte  */                                         \
  char varname[strlen(dir_path) + strlen(config_name) + 2];                    \
  sprintf(varname, "%s/%s", dir_path, config_name)

#define CMC_JOIN_STR_STACK(varname, str_a, str_b)                              \
  char varname[strlen(str_a) + strlen(str_b) + 1];                             \
  sprintf(varname, "%s%s", str_a, str_b)

static inline cmc_error_t cmc_convert_str_to_int(char *str, uint32_t n,
                                                 int *output) {
  cmc_error_t err;
  for (uint32_t i = 0; i < n; i++) {
    if (str[i] == 0) {
      break;
    }

    if (!isdigit(str[i])) {
      err = cmc_errorf(EINVAL, "Unable to convert to integer `str=%s`\n", str);
      return err;
    }
  }

  char number[n + 1];
  memset(number, 0, n + 1);
  strncpy(number, str, n);

  *output = atoi(number);

  return NULL;
}

#endif // C_MINILIB_CONFIG_CMC_STRING_H
