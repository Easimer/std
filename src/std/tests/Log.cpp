#include <std/Check.h>
#include <std/CompilerInfo.h>
#include <std/log.h>
#include <std/log/log_internal.h>
#include <std/Testing.hpp>

SN_TEST(Log, registerHandler) {
  auto fun_on_event = [](const struct log_handler *handler,
                         const struct log_event *ev, va_list ap) {
    bool *p_called = (bool *)handler->data;
    *p_called = true;
  };

  struct log_handler_api api = {
      .on_event = fun_on_event,
  };

  bool called = false;
  struct log_handler handler = {
      .api = &api,
      .data = &called,
  };
  int rc = log_register_handler(&handler);
  CHECK(rc == LOG_ERR_OK);

  log_log(LOG_INFO, "dummy.c", 123, "None");
  CHECK(called == true);

  rc = log_unregister_handler(&handler);
  CHECK(rc == LOG_ERR_OK);

  called = false;
  log_log(LOG_INFO, "dummy.c", 123, "None");
  CHECK(called == false);
}

SN_TEST(Log, eventParams) {
  struct log_handler_test {
    enum log_level level;
    const char *fmt;
    const char *file;
    int32_t line;
  };

  auto fun_on_event = [](const struct log_handler *handler,
                         const struct log_event *ev, va_list ap) {
    auto *out = (struct log_handler_test *)handler->data;
    out->level = ev->level;
    out->fmt = ev->fmt;
    out->file = ev->file;
    out->line = ev->line;
  };

  struct log_handler_api api = {
      .on_event = fun_on_event,
  };

  struct log_handler_test data = {
      .fmt = NULL,
      .file = NULL,
      .line = -1,
  };

  struct log_handler handler = {
      .api = &api,
      .data = &data,
  };
  log_register_handler(&handler);

  const char *fmt_in = "format format";
  const char *file_in = "dummy.c";
  int32_t line_in = 123;
  for (int level = LOG_TRACE; level <= LOG_FATAL; level++) {
#if SN_COMPILER == SN_COMPILER_CLANG || SN_COMPILER == SN_COMPILER_GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-security"
#endif
    log_log((enum log_level)level, file_in, line_in, fmt_in);
#if SN_COMPILER == SN_COMPILER_CLANG || SN_COMPILER == SN_COMPILER_GCC
#pragma GCC diagnostic pop
#endif
    CHECK(data.level == level);
    CHECK(data.fmt == fmt_in);
    CHECK(data.file == file_in);
    CHECK(data.line == line_in);
  }

  log_unregister_handler(&handler);
}
