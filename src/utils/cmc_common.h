/*
 * Copyright (c) 2025 Jakub Buczynski <KubaTaba1uga>
 * SPDX-License-Identifier: MIT
 * See LICENSE file in the project root for full license information.
 */

#ifndef C_MINILIB_CONFIG_CMC_COMMON_H
#define C_MINILIB_CONFIG_CMC_COMMON_H

#define CMC_SETTINGS_ARR_PATHS_MAX 32
#define CMC_SETTINGS_PATHS_MAX 128
#define CMC_SETTINGS_NAME_MAX 32

#define CMC_FOREACH_PTR(item, array, size)                                     \
  for (size_t _i = 0; _i < (size) && ((item) = &(array)[_i], 1); ++_i)

#define CMC_LOG(settings, level, fmt, ...)                                     \
  do {                                                                         \
    if ((settings) && (settings)->log_func) {                                  \
      char _log_buf[1024];                                                     \
      int _written =                                                           \
          snprintf(_log_buf, sizeof(_log_buf), (fmt), ##__VA_ARGS__);          \
      if (_written < 0 || _written >= (int)sizeof(_log_buf)) {                 \
        strncpy(_log_buf, "[log message truncated]", sizeof(_log_buf) - 1);    \
        _log_buf[sizeof(_log_buf) - 1] = '\0';                                 \
      }                                                                        \
      (settings)->log_func(level, _log_buf);                                   \
    }                                                                          \
  } while (0)

#endif // C_MINILIB_CONFIG_CMC_COMMON_H
