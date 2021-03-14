#include "awtk.h"
#include "html_view_register.h"

ret_t application_init(void) {
  html_view_register();

  window_open("main");

  return RET_OK;
}

ret_t application_exit(void) {
  log_debug("application_exit\n");
  return RET_OK;
}
