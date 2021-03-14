/**
 * File:   html_view.c
 * Author: AWTK Develop Team
 * Brief:  Simple HTML View
 *
 * Copyright (c) 2021 - 2021 Guangzhou ZHIYUAN Electronics Co.,Ltd.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * License file for more details.
 *
 */

/**
 * History:
 * ================================================================
 * 2021-03-09 Li XianJing <xianjimli@hotmail.com> created
 *
 */


#include "tkc/mem.h"
#include "tkc/utils.h"
#include "html_view_register.h"
#include "base/widget_factory.h"

#include "html_view/html.h"
#include "html_view/html_view.h"

ret_t html_view_register(void) {
  widget_factory_register(widget_factory(), WIDGET_TYPE_HTML, html_create);
  return widget_factory_register(widget_factory(), WIDGET_TYPE_HTML_VIEW, html_view_create);
}

const char* html_view_supported_render_mode(void) {
  return "OpenGL|AGGE-BGR565|AGGE-BGRA8888|AGGE-MONO";
}
