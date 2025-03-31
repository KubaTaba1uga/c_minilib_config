#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_minilib_config.h"
#include "error/error.h"

static struct cmc_Error generic_error = {.code = -1,
                                         .msg = "Generic error message",
                                         .source_file = __FILE__,
                                         .source_func = 0,
                                         .source_line = 0};

struct cmc_Error *cmc_error_create_with_fmt(int code, char *source_file,
                                            char *source_func, int source_line,
                                            char *fmt, ...) {
  struct cmc_Error *err = malloc(sizeof(struct cmc_Error));
  if (!err) {
    generic_error.code = ENOMEM;
    generic_error.msg = "Unable to allocate memory for `struct cmc_Error`";
    generic_error.source_func = (char *)__func__;
    generic_error.source_line = __LINE__;
    return &generic_error;
  }

  err->code = code;
  err->source_line = source_line;
  err->source_file = source_file ? strdup(source_file) : NULL;
  err->source_func = source_func ? strdup(source_func) : NULL;
  err->msg = NULL;

  va_list args;
  va_start(args, fmt);
  int len = vsnprintf(NULL, 0, fmt, args);
  va_end(args);

  if (len < 0) {
    cmc_error_destroy(err);
    generic_error.code = EINVAL;
    generic_error.msg = "Invalid `struct cmc_Error` variadic args";
    generic_error.source_func = (char *)__func__;
    generic_error.source_line = __LINE__;
    return &generic_error;
  }

  err->msg = malloc(len + 1);
  if (!err->msg) {
    cmc_error_destroy(err);
    generic_error.code = ENOMEM;
    generic_error.msg =
        "Unable to allocate memory for `struct cmc_Error` message";
    generic_error.source_func = (char *)__func__;
    generic_error.source_line = __LINE__;
    return &generic_error;
  }

  va_start(args, fmt);
  vsnprintf(err->msg, len + 1, fmt, args);
  va_end(args);

  return err;
}

void cmc_error_destroy(struct cmc_Error *err) {
  if (!err || err == &generic_error)
    return;

  free(err->msg);
  free(err->source_file);
  free(err->source_func);
  free(err);
}
