#include "cmc_env_parser.h"
#include "c_minilib_config.h"
#include "cmc_parse_interface/cmc_parse_interface.h"
#include <string.h>

static cmc_error_t _cmc_env_parser_create(cmc_ConfigParserData *data) {
  return NULL;
};

static cmc_error_t _cmc_env_parser_is_format(const size_t n, const char path[n],
                                             bool *result) {
  return NULL;
}

static cmc_error_t _cmc_env_parser_parse(const size_t n, const char buffer[n],
                                         const cmc_ConfigParserData data,
                                         struct cmc_Config *config) {
  return NULL;
}

static void _cmc_env_parser_destroy(cmc_ConfigParserData *data){

};

cmc_error_t cmc_env_parser_init(struct cmc_ConfigParseInterface *parser) {
  struct cmc_ConfigParseInterface env_parser = {
      .id = "env",
      .create = _cmc_env_parser_create,
      .is_format = _cmc_env_parser_is_format,
      .parse = _cmc_env_parser_parse,
      .destroy = _cmc_env_parser_destroy,
  };

  memcpy(parser, &env_parser, sizeof(struct cmc_ConfigParseInterface));

  return NULL;
};
