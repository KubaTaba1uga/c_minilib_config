#ifndef C_MINILIB_CONFIG_CMC_COMMON_H
#define C_MINILIB_CONFIG_CMC_COMMON_H

#define CMC_SETTINGS_ARR_PATHS_MAX 32
#define CMC_SETTINGS_PATHS_MAX 128
#define CMC_SETTINGS_NAME_MAX 32

#define CMC_FOREACH(item, array, size)                                         \
  for (size_t _i = 0; _i < (size) && ((item) = (array)[_i], 1); ++_i)

#define CMC_FOREACH_PTR(item, array, size)                                     \
  for (size_t _i = 0; _i < (size) && ((item) = &(array)[_i], 1); ++_i)

#define CMC_LOG(settings, level, fmt, ...)                                     \
  do {                                                                         \
    if ((settings) && (settings)->log_func) {                                  \
      /* 1) compute needed length */                                           \
      int _len = snprintf(NULL, 0, (fmt), ##__VA_ARGS__);                      \
      /* 2) allocate on stack and format */                                    \
      char _buf[_len + 1];                                                     \
      snprintf(_buf, _len + 1, (fmt), ##__VA_ARGS__);                          \
      /* 3) call user logger */                                                \
      (settings)->log_func((level), _buf);                                     \
    }                                                                          \
  } while (0)

#endif // C_MINILIB_CONFIG_CMC_COMMON_H
