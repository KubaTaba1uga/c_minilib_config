#ifndef C_MINILIB_CONFIG_CMC_PARSE_INTERFACE_H
#define C_MINILIB_CONFIG_CMC_PARSE_INTERFACE_H

#include <c_minilib_config.h>
#include <stdbool.h>
#include <stddef.h>

typedef void *cmc_ConfigParserData;

enum cmc_ConfigParseFormat {
  cmc_ConfigParseFormat_NONE,
  cmc_ConfigParseFormat_ENV,
  cmc_ConfigParseFormat_JSON,
  // Add new formats here
  cmc_ConfigParseFormat_MAX,
};

struct cmc_ConfigParseInterface {
  const char id;
  cmc_ConfigParserData data;
  // Initialize parser instance
  cmc_error_t (*init)(cmc_ConfigParserData *);
  // Decide if parser can parse a file
  cmc_error_t (*is_format)(const size_t n, const char path[n], bool *result);
  // Parse a file loaded into buffer
  cmc_error_t (*parse)(const size_t n, const char buffer[n],
                       struct cmc_Config *config);
  // Destroy parser instance
  cmc_error_t (*destroy)(cmc_ConfigParserData *);
};

#endif // C_MINILIB_CONFIG_PARSE_INTERFACE_H
