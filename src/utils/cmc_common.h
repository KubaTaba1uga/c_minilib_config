#ifndef C_MINILIB_CONFIG_CMC_COMMON_H
#define C_MINILIB_CONFIG_CMC_COMMON_H

#define CMC_SETTINGS_ARR_PATHS_MAX 32
#define CMC_SETTINGS_PATHS_MAX 128
#define CMC_SETTINGS_NAME_MAX 32

#define CMC_FOREACH(item, array, size)                                         \
  for (size_t _i = 0; _i < (size) && ((item) = (array)[_i], 1); ++_i)

#endif // C_MINILIB_CONFIG_CMC_COMMON_H
