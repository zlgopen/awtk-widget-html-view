/**
 * File:   html.c
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

#include "html.h"
#include "html_impl.h"

const char* s_html_properties[] = {HTML_PROP_URL, NULL};

TK_DECL_VTABLE(html) = {.size = sizeof(html_t),
                        .type = WIDGET_TYPE_HTML,
                        .clone_properties = s_html_properties,
                        .persistent_properties = s_html_properties,
                        .parent = TK_PARENT_VTABLE(widget),
                        .create = html_create,
                        .get_prop = html_get_prop,
                        .set_prop = html_set_prop,
                        .on_paint_self = html_on_paint_self,
                        .on_event = html_on_event,
                        .on_destroy = html_on_destroy};

widget_t* html_create(widget_t* parent, xy_t x, xy_t y, wh_t w, wh_t h) {
  widget_t* widget = widget_create(parent, TK_REF_VTABLE(html), x, y, w, h);
  html_t* html = HTML(widget);
  return_value_if_fail(html != NULL, NULL);

  html->yslidable = TRUE;
  html->impl = html_impl_create(widget);

  return widget;
}

ret_t html_set_url(widget_t* widget, const char* url) {
  html_t* html = HTML(widget);
  return_value_if_fail(html != NULL, RET_BAD_PARAMS);

  html->url = tk_str_copy(html->url, url);

  return RET_OK;
}

ret_t html_set_yslidable(widget_t* widget, bool_t yslidable) {
  html_t* html = HTML(widget);
  return_value_if_fail(html != NULL, RET_FAIL);

  html->yslidable = yslidable;

  return RET_OK;
}

widget_t* html_cast(widget_t* widget) {
  return_value_if_fail(WIDGET_IS_INSTANCE_OF(widget, html), NULL);

  return widget;
}
